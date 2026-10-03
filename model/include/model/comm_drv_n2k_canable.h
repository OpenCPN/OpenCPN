/***************************************************************************
 *   Copyright (C) 2026 by twocanplugin@hotmail.com                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * comm_drv_n2k_canable.h NMEA2000 driver for CANable "Contact"
 * (and other SLCAN/LAWICEL-firmware) USB-CAN adapters connected via a
 * normal (virtual) serial port. Windows only
 *
:*/

#ifndef _COMM_DRV_N2K_CANABLE_H
#define _COMM_DRV_N2K_CANABLE_H

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <wx/wx.h>
#include <wx/thread.h>

#include <windows.h>

#include "model/comm_driver.h"
#include "model/comm_drv_n2k.h"
#include "model/comm_navmsg_bus.h"
#include "model/conn_params.h"
#include "model/comm_drv_stats.h"
#include "model/comm_can_util.h"
#include "N2kMsg.h"

class CommDriverN2KCanable;

// Note to self.
// Worker thread perform the actual serial reads, SLCAN line parsing,
// fast-packet reassembly, address-claim/product-info requests and delivery to
// DriverListener::Notify() similar to Worker::HandleInput() in
// comm_drv_n2k_socketcan.cpp
// Outgoing SLCAN commands are queued from any thread, either this one,
// for address-claim and product information frames, or the GUI thread
// or ordinary SendMessage() traffic.

class CanableSerialThread : public wxThread {
public:
  CanableSerialThread(CommDriverN2KCanable* parent, const wxString& port_name);
  ~CanableSerialThread() override;

  void* Entry() override;
  void OnExit() override;

  // Signals the worker thread to stop, and wakes it immediately even if
  // it's currently blocked waiting on a pending overlapped read
  void RequestStop();

  // Thread-safe queue of fully-formed SLCAN commands for transmission
  // Wakes the worker thread immediately via m_write_pending_event
  void QueueWrite(const std::string& command);

private:
  bool OpenPort();
  void ClosePort();
  void ParseReceivedData(const char* buf, size_t len,
                         std::vector<can_frame>* out_frames);
  bool DecodeSLCANFrame(const std::string& line, can_frame* frame);
  void DequeueWrite();

  // Overlapped write.
  bool WriteFrame(const std::string& bytes);

  CommDriverN2KCanable* m_parent;
  wxString m_portName;

  std::atomic<bool> m_stopRequested;

  // Write queue & accompanying mutex
  std::mutex m_transmitMutex;
  std::vector<std::string> m_transmitQueue;

  // Reassembly state for ParseReceivedData(), carried across successive reads
  // since a SLCAN line may span more than one read.
  std::string m_partialLine;
  // If we have received the 'T' start delimiter
  bool m_hasStartDelimiter = false;

  // Serial port handle
  HANDLE m_serialPortHandle = INVALID_HANDLE_VALUE;

  // Overlapped I/O state. m_stop_event and m_write_pending_event are
  // created in the constructor, independent of the port's own lifetime,
  // so RequestStop()/QueueWrite() are always safe to call even if
  // OpenPort() hasn't run (or failed) yet.
  // m_read_overlapped.hEvent and m_write_overlapped.hEvent are created in
  // OpenPort() and destroyed in ClosePort(), tied to the open handle,
  // a fresh OVERLAPPED and event pair per open, matching the handle's own
  // lifetime.
  HANDLE m_stopEvent = nullptr;
  HANDLE m_pendingWriteEvent = nullptr;
  OVERLAPPED m_overlappedRead = {};
  OVERLAPPED m_overlappedWrite = {};

  // Flag while waiting on a pending read. Used to prevent issuing multiple
  // reads
  bool m_isReadPending = false;
};

// NMEA2000 communications driver for a CANable "Contact" (or other
// SLCAN-firmware) USB-CAN adapter attached as a serial port.
// Windows only as SLCAN devices support SocketCAN on Linux
class CommDriverN2KCanable : public CommDriverN2K, public DriverStatsProvider {
  friend class CanableSerialThread;

public:
  CommDriverN2KCanable(const ConnectionParams* params,
                       DriverListener& listener);
  ~CommDriverN2KCanable() override;

  void SetListener(DriverListener& l) override {}

  bool SendMessage(std::shared_ptr<const NavMsg> msg,
                   std::shared_ptr<const NavAddr> addr) override;

  void Open();
  void Close();

  // Queues a complete SLCAN frame.
  bool WriteFrame(const std::string& bytes);

  // Send PGN 60928 Address Claim.
  // Mirrors CommDriverN2KSocketCanImpl::SendAddressClaim()
  bool SendAddressClaim(int proposed_source_address);

  // Send PGN 126996 Product Information
  // Mirrors CommDriverN2KSocketCanImpl::SendProductInfo()
  bool SendProductInfo();

  // Persist our network address
  void UpdateAttrCanAddress();

  // Return "crude" statistics - populates the tick box in the connection
  // dialog? Mirrors CommDriverN2KSocketCAN::GetDriverStats()
  DriverStats GetDriverStats() const override { return m_driverStatistics; }

private:
  // Encode message into Actisense format
  void EncodeActisenseMessage(const can_frame& frame);

  // Parses a limited set of NMEA Messages, namely PGN 60928 Address Claim
  // and PGN 59904 ISO Requests
  // Mirrors Worker::ProcessRxMessages() in comm_drv_n2k_socketcan.cpp.
  // Called from HandleCanFrameInput(), so also always on the worker thread
  void ProcessMessages(const std::shared_ptr<const Nmea2000Msg>& msg);

  // Constructs our unique NMEA 2000 NAME
  void SetN2kName();

  // Encodes NMEA 2000 messages into the SLCAN format
  std::string EncodeSLCANFrame(uint32_t can_id,
                               const std::vector<uint8_t>& data);

  // Fragments Fast Messages if required
  std::vector<std::string> FormatCanMessage(uint8_t priority, uint8_t source,
                                            uint8_t destination, uint64_t pgn,
                                            const std::vector<uint8_t>& data);

  ConnectionParams m_connectionParameters;
  DriverListener& m_driverListener;

  wxString m_portName;

  // Flag to indicate that the worker thread's port is actually open
  bool m_isPortOpen = false;

  // Basic statis, interface name, Tx & Rx counts
  DriverStats m_driverStatistics;

  // The fast message map - based on TwoCan's without any acknowledgemnt; the
  // bastards
  std::unique_ptr<FastMessageMap> m_fastMessageMap;

  // The worker thread
  std::unique_ptr<CanableSerialThread> m_serialThread;

  // Guards m_sequenceNumber
  std::mutex m_sequenceNumberMutex;

  // Fast Message sequence number
  int m_sequenceNumber = 0;

  N2kName m_deviceInformation;
  // Used for NMEA 2000 NAME and Product Information
  int m_uniqueNumber = 1;

  // Seems to be the OpenCPN  default source address. See also
  // comm_drv_n2k_socketcan's
  static const int kDefaultSourceAddress = 72;
  std::atomic<int> m_sourceAddress{kDefaultSourceAddress};

  // Driver Stats are updated by a timer, not called by OpenCPN ?
  StatsTimer m_statsTimer;
};

#endif
