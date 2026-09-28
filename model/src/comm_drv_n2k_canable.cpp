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
 * Implements comm_drv_n2k_canable.h. NMEA 2000 SLCAN driver for Windows
 */

#include "model/comm_drv_n2k_canable.h"

#include "config.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <sstream>

// Used by the fast message map
static const int kNotFound = -1;

// Pads fixed length string. Used for PGN 126996 Product Information fields
// Mirrors comm_drv_n2k_socketcan.cpp's file-local AddStr()
static void AddStr(std::vector<uint8_t>* data, const std::string& text,
                   size_t length) {
  size_t i = 0;
  for (; i < text.size() && i < length; i++) data->push_back(text[i]);
  for (; i < length; i++) data->push_back(0);
}

// Canable Worker Thread
CanableSerialThread::CanableSerialThread(CommDriverN2KCanable* parent,
                                         const wxString& port_name)
    : wxThread(wxTHREAD_JOINABLE),
      m_parent(parent),
      m_portName(port_name),
      m_stopRequested(false) {
  // Events are created in the ctor as they must be safe to call at any point
  // in this object's life, independent of whether the port has been opened.

  // Stop event is set upon closing and consumed by the overlapped read. It is a
  // manual-reset event
  m_stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);

  // Write event is set upon Write and also consumed by overlapped read. It is
  // an auto-reset event
  m_pendingWriteEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
}

// Close our handkes correctly
CanableSerialThread::~CanableSerialThread() {
  if (m_stopEvent) {
    CloseHandle(m_stopEvent);
  }
  if (m_pendingWriteEvent) {
    CloseHandle(m_pendingWriteEvent);
  }
}

// Handle a stop request when OpenCPN is shutting down or our driver is
// uninstalled
void CanableSerialThread::RequestStop() {
  m_stopRequested = true;
  if (m_stopEvent) {
    SetEvent(m_stopEvent);
  }
}

// As it says on the tin...
bool CanableSerialThread::OpenPort() {
  wxString serialPortName = m_portName;
  // Prefix the port name correctly (eg. COM10)
  if (!serialPortName.StartsWith("\\\\.\\")) {
    serialPortName = "\\\\.\\" + serialPortName;
  }

  // Open the serial port using overlapped I/O
  m_serialPortHandle =
      CreateFileW(serialPortName.wc_str(), GENERIC_READ | GENERIC_WRITE, 0,
                  nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);

  if (m_serialPortHandle == INVALID_HANDLE_VALUE) {
    return false;
  }

  // Configure the comm settings & timeouts
  DCB dcb = {0};
  dcb.DCBlength = sizeof(dcb);

  if (!GetCommState(m_serialPortHandle, &dcb)) {
    CloseHandle(m_serialPortHandle);
    m_serialPortHandle = INVALID_HANDLE_VALUE;
    return false;
  }
  // For most SLCAN adapters (including CANable) the actual UART parameters are
  // irrelevant to the USB link, but nonetheless use the same defaults
  // as used by the TwoCan cantact driver as they seem to work OK
  dcb.BaudRate = CBR_115200;
  dcb.ByteSize = 8;
  dcb.Parity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;
  dcb.fBinary = TRUE;
  dcb.fDtrControl = DTR_CONTROL_ENABLE;
  dcb.fRtsControl = RTS_CONTROL_ENABLE;  // Some USB controllers gate on RTS
  SetCommState(m_serialPortHandle, &dcb);

  // Even though using overlapped I/O these are still useful
  COMMTIMEOUTS timeouts = {0};
  timeouts.ReadIntervalTimeout = 10;
  timeouts.ReadTotalTimeoutConstant = 100;
  timeouts.ReadTotalTimeoutMultiplier = 0;
  timeouts.WriteTotalTimeoutConstant = 50;
  timeouts.WriteTotalTimeoutMultiplier = 0;
  SetCommTimeouts(m_serialPortHandle, &timeouts);

  // Configure the Overlapped I/O
  ZeroMemory(&m_overlappedRead, sizeof(m_overlappedRead));
  m_overlappedRead.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);

  ZeroMemory(&m_overlappedWrite, sizeof(m_overlappedWrite));
  m_overlappedWrite.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);

  // The flag used to prevent issuing multiple read requests
  m_isReadPending = false;

  return true;
}

// Close down nicely...
void CanableSerialThread::ClosePort() {
  if (m_overlappedRead.hEvent) {
    CloseHandle(m_overlappedRead.hEvent);
    m_overlappedRead.hEvent = nullptr;
  }
  if (m_overlappedWrite.hEvent) {
    CloseHandle(m_overlappedWrite.hEvent);
    m_overlappedWrite.hEvent = nullptr;
  }
  if (m_serialPortHandle != INVALID_HANDLE_VALUE) {
    CloseHandle(m_serialPortHandle);
    m_serialPortHandle = INVALID_HANDLE_VALUE;
  }
}

// Write the NMEA 2000 message
bool CanableSerialThread::WriteFrame(const std::string& payload) {
  ResetEvent(m_overlappedWrite.hEvent);
  DWORD bytesWritten = 0;
  BOOL result = WriteFile(m_serialPortHandle, payload.data(),
                          static_cast<DWORD>(payload.size()), &bytesWritten,
                          &m_overlappedWrite);

  if (result) {
    // The write completed synchronously.
    return bytesWritten == payload.size();
  }

  if (GetLastError() != ERROR_IO_PENDING) {
    return false;
  }

  // A write this small should complete almost immediately;
  // If it doesn't, cancel it rather than risk blocking indefinitely.
  DWORD wait_result = WaitForSingleObject(m_overlappedWrite.hEvent, 200);
  if (wait_result != WAIT_OBJECT_0) {
    CancelIoEx(m_serialPortHandle, &m_overlappedWrite);
    return false;
  }
  // Overlapped write has completed
  DWORD overlappedBytesWritten = 0;
  if (!GetOverlappedResult(m_serialPortHandle, &m_overlappedWrite,
                           &overlappedBytesWritten, FALSE)) {
    return false;
  }
  // Completed successfully
  return overlappedBytesWritten == payload.size();
}

// BUG BUG I need to understand why the use of a names space
// namespace {

// Parses one complete SLCAN line into a can_frame.
// Eg. "T1D11220381122334455667788\r"
// Note the 'T' denotes an extended frame, ie. 29bit Can Id
bool CanableSerialThread::DecodeSLCANFrame(const std::string& line,
                                           can_frame* frame) {
  if (line.size() < 10) {  // Minimum potential size "T1D1122030" - no data!
    return false;
  }

  // Parse the Can Id
  frame->can_id =
      static_cast<uint32_t>(strtoul(line.substr(1, 8).c_str(), nullptr, 16)) &
      0x1FFFFFFFu;
  // The data length
  int dlc = line[9] - '0';
  // Guard against an invalid length. Should either be 3 (PGN 59904) or 8 (any
  // other PGN)
  if (dlc < 0 || dlc > 8) {
    return false;
  }

  frame->can_dlc = static_cast<uint8_t>(dlc);

  std::memset(frame->data, 0, sizeof(frame->data));
  // Index into the payload
  size_t index = 10;
  for (int i = 0; i < dlc; i++) {
    if (index + 2 > line.size()) {
      return false;
    }
    // Parse each pair of hex characters
    frame->data[i] = static_cast<uint8_t>(
        strtoul(line.substr(index, 2).c_str(), nullptr, 16));
    index += 2;
  }
  return true;
}

//}  // namespace

void CanableSerialThread::ParseReceivedData(
    const char* buf, size_t len, std::vector<can_frame>* out_frames) {
  // Mirrors cantact.c's ReadThread() assembly loop: scan a raw chunk of
  // serial bytes for 'T'-delimited, CR-terminated lines, tolerating
  // partial lines that span two reads and resynchronizing if a CR arrives
  // without a preceding 'T'.
  for (size_t i = 0; i < len; i++) {
    char c = buf[i];
    // We have the start delimiter
    if (c == 'T') {
      m_hasStartDelimiter = true;
      m_partialLine.clear();
      m_partialLine += c;
      continue;
    }
    // We have the terminating CR
    if (c == '\r' || c == '\n') {
      if (c == '\r' && m_hasStartDelimiter && !m_partialLine.empty()) {
        can_frame frame;
        // Process the received frame
        if (DecodeSLCANFrame(m_partialLine, &frame)) {
          out_frames->push_back(frame);
        }
      }
      // Reset for next frame
      m_hasStartDelimiter = false;
      m_partialLine.clear();
      continue;
    }
    // We've  had the start delimiter, now append a normal character
    if (m_hasStartDelimiter) {
      m_partialLine += c;
      // Guard against a malformed stream
      if (m_partialLine.size() > 64) {
        m_hasStartDelimiter = false;
        m_partialLine.clear();
      }
    }
  }
}
// Enqueue a complete SLCAN frame for transmission
void CanableSerialThread::QueueWrite(const std::string& slcan_string) {
  {
    std::lock_guard<std::mutex> lock(m_transmitMutex);
    m_transmitQueue.push_back(slcan_string);
  }
  // Wake up the thread in case it is blocked waiting on a read
  if (m_pendingWriteEvent) {
    SetEvent(m_pendingWriteEvent);
  }
}

// Dequeue any write requests
void CanableSerialThread::DequeueWrite() {
  std::vector<std::string> slcan_strings;
  {
    std::lock_guard<std::mutex> lock(m_transmitMutex);
    if (m_transmitQueue.empty()) {
      return;
    }
    slcan_strings.swap(m_transmitQueue);
  }
  for (const auto& slcan_string : slcan_strings) {
    WriteFrame(slcan_string);
    // Wait a little to ensure we don't overwrite the device's buffer
    wxMilliSleep(5);
  }
}

// The wxThread entry point
void* CanableSerialThread::Entry() {
  if (!OpenPort()) {
    wxLogMessage("CANable: failed to open serial port %s", m_portName);
    return nullptr;
  }

  // Once we've opened the port, announce that stats may be available
  m_parent->m_driverStatistics.available = true;

  // Initialize the Canable adapter
  // Close the port
  WriteFrame("C\r");
  wxMilliSleep(20);
  // Set the bus speed to 250k
  WriteFrame("S5\r");
  wxMilliSleep(20);
  // Open the port
  WriteFrame("O\r");
  wxMilliSleep(50);

  // Send our address claim
  m_parent->SendAddressClaim(CommDriverN2KCanable::kDefaultSourceAddress);

  // Begin reading from the serial port
  char serialBuffer[4096];
  std::vector<can_frame> frames;

  for (;;) {
    if (!m_isReadPending) {
      ResetEvent(m_overlappedRead.hEvent);
      DWORD bytesRead = 0;
      BOOL result =
          ReadFile(m_serialPortHandle, serialBuffer, sizeof(serialBuffer),
                   &bytesRead, &m_overlappedRead);
      if (result) {
        // Completed synchronously
        if (bytesRead > 0) {
          frames.clear();
          ParseReceivedData(serialBuffer, bytesRead, &frames);
          for (const auto& frame : frames) {
            // Send the frame to both OpenCPN and for local parsing of Address
            // Claims etc.
            m_parent->EncodeActisenseMessage(frame);
          }
        }
        // Anything to write ?
        DequeueWrite();
        if (m_stopRequested) {
          break;
        }
        continue;
      }
      if (GetLastError() != ERROR_IO_PENDING) {
        // Perhaps a real error (e.g. the device was unplugged). Back off
        // briefly rather than spin-looping on a persistent failure, but keep
        // trying, RequestStop() is the only intended way to exit.
        wxMilliSleep(50);
        if (m_stopRequested) {
          break;
        }
        continue;
      }
      m_isReadPending = true;
    }

    // Wait on whichever happens first: the pending read completes, a
    // stop is requested, or a write is queued.
    HANDLE handles[3] = {m_overlappedRead.hEvent, m_stopEvent,
                         m_pendingWriteEvent};
    DWORD wait_result = WaitForMultipleObjects(3, handles, FALSE, 1000);

    if (wait_result == WAIT_OBJECT_0) {
      // Read completed
      // GetOverlappedResult() provides the number of bytes read.
      DWORD overlappedBytesRead = 0;
      if (GetOverlappedResult(m_serialPortHandle, &m_overlappedRead,
                              &overlappedBytesRead, FALSE) &&
          overlappedBytesRead > 0) {
        frames.clear();
        ParseReceivedData(serialBuffer, overlappedBytesRead, &frames);
        for (const auto& frame : frames) {
          m_parent->EncodeActisenseMessage(frame);
        }
      }
      m_isReadPending = false;
    } else if (wait_result == WAIT_OBJECT_0 + 1) {
      // Stop requested: cancel the in-flight read so it doesn't leak as
      // an orphaned pending I/O when the handle is closed below, then
      // wait for the cancellation itself to be acknowledged before
      // proceeding
      CancelIoEx(m_serialPortHandle, &m_overlappedRead);
      DWORD dummy = 0;
      GetOverlappedResult(m_serialPortHandle, &m_overlappedRead, &dummy, TRUE);
      m_isReadPending = false;
      break;
    } else if (wait_result == WAIT_OBJECT_0 + 2) {
      // Write queued: the pending read is untouched and still in flight,
      // nothing to do here except fall through to DrainWriteQueue() below.
    } else if (wait_result == WAIT_TIMEOUT) {
      // No read completion, no stop, no write queued within 1s
      // Purely a defensive fallback the read stays pending, loop back and wait.
    } else {
      // WAIT_FAILED or an abandoned handle: bail out defensively
      CancelIoEx(m_serialPortHandle, &m_overlappedRead);
      DWORD dummy = 0;
      GetOverlappedResult(m_serialPortHandle, &m_overlappedRead, &dummy, TRUE);
      m_isReadPending = false;
      break;
    }

    DequeueWrite();
    if (m_stopRequested) {
      if (m_isReadPending) {
        CancelIoEx(m_serialPortHandle, &m_overlappedRead);
        DWORD dummy = 0;
        GetOverlappedResult(m_serialPortHandle, &m_overlappedRead, &dummy,
                            TRUE);
        m_isReadPending = false;
      }
      break;
    }
  }

  // Close the CAN channel before closing the port.
  WriteFrame("C\r");
  ClosePort();
  return nullptr;
}

void CanableSerialThread::OnExit() {}

// CANable Cantact driver
CommDriverN2KCanable::CommDriverN2KCanable(const ConnectionParams* params,
                                           DriverListener& listener)
    : CommDriverN2K(params->GetStrippedDSPort()),
      m_connectionParameters(*params),
      m_driverListener(listener),
      m_portName(params->GetStrippedDSPort()),
      m_statsTimer(*this, std::chrono::milliseconds(2000)) {
  // Stats driver mirrors that of CommDriverN2KSocketCanImpl
  m_driverStatistics.driver_bus = NavAddr::Bus::N2000;
  m_driverStatistics.driver_iface = params->GetStrippedDSPort();

  this->attributes["canAdapter"] = std::string("CANable Cantact (slcan)");
  this->attributes["serialPort"] = m_portName.ToStdString();
  this->attributes["bitrate"] = std::string("250000");
  this->attributes["canAddress"] = std::to_string(kDefaultSourceAddress);
  this->attributes["ioDirection"] = PortDirectionToString(params->direction);

  // Generate our NMEA 2000 NAME
  SetN2kName();

  // Fast Message Map. Used to reassemble fast messages
  m_fastMessageMap = std::make_unique<FastMessageMap>();

  // Open the Port
  Open();
}

CommDriverN2KCanable::~CommDriverN2KCanable() {
  // Close the port
  Close();
}

void CommDriverN2KCanable::SetN2kName() {
  // Mirrors CommDriverN2KSocketCanImpl::SetN2K_Name():
  m_deviceInformation.value.Name = 0;

  // 16-bit hash of the hostname, used as a pseudo-unique serial number so
  // two OpenCPN instances on different machines don't collide with
  // identical NAMEs on the same bus.
  wxString hostname = wxGetHostName();
  std::string str(hostname.mb_str());
  int hash = 0;
  for (char ch : str) hash = hash + (hash << 5) + ch + (ch << 7);
  m_uniqueNumber = ((hash) ^ (hash >> 16)) & 0xffff;

  // Used for PGN 126996 Product Information
  // Matches OpenCPN's "manufacturer code"
  m_deviceInformation.SetManufacturerCode(2046);
  m_deviceInformation.SetUniqueNumber(m_uniqueNumber);
  m_deviceInformation.SetDeviceFunction(130);
  m_deviceInformation.SetDeviceClass(120);
  m_deviceInformation.SetIndustryGroup(4);
  m_deviceInformation.SetSystemInstance(0);
}

// Persist our CAN address
void CommDriverN2KCanable::UpdateAttrCanAddress() {
  this->attributes["canAddress"] = std::to_string(m_sourceAddress.load());
}

// Start the worker thread (in turn opens the serial port)
void CommDriverN2KCanable::Open() {
  m_serialThread = std::make_unique<CanableSerialThread>(this, m_portName);
  m_serialThread->Run();
  m_isPortOpen = true;
}

// Stop the worker thread
void CommDriverN2KCanable::Close() {
  m_isPortOpen = false;
  if (m_serialThread) {
    m_serialThread->RequestStop();
    m_serialThread->Wait();
    m_serialThread.reset();
  }
}

// Queue a SLCAN formatted frame for sending
bool CommDriverN2KCanable::WriteFrame(const std::string& payload) {
  // Guard against the thread not having been started
  if (!m_serialThread) {
    return false;
  }
  m_serialThread->QueueWrite(payload);
  return true;
}

// Generate PGN 60928 Address Claim
bool CommDriverN2KCanable::SendAddressClaim(int proposed_source_address) {
  // Update our address
  m_sourceAddress.store(proposed_source_address);
  UpdateAttrCanAddress();

  std::vector<uint8_t> data;
  uint32_t unique_and_mfg = m_deviceInformation.value.UnicNumberAndManCode;

  data.push_back(unique_and_mfg & 0xFF);
  data.push_back((unique_and_mfg >> 8) & 0xFF);
  data.push_back((unique_and_mfg >> 16) & 0xFF);
  data.push_back((unique_and_mfg >> 24) & 0xFF);
  data.push_back(m_deviceInformation.value.DeviceInstance);
  data.push_back(m_deviceInformation.value.DeviceFunction);
  data.push_back(m_deviceInformation.value.DeviceClass);
  data.push_back(m_deviceInformation.value.IndustryGroupAndSystemInstance);

  std::vector<std::string> slcan_strings = FormatCanMessage(
      6, static_cast<uint8_t>(proposed_source_address), 0xFF, 60928, data);

  bool result = true;
  for (const auto& slcan_string : slcan_strings) {
    result = WriteFrame(slcan_string) && result;
  }
  return result;
}

// Generates PGN 126996 Product Information
bool CommDriverN2KCanable::SendProductInfo() {
  std::vector<uint8_t> payload;
  payload.push_back(2100 & 0xFF);  // N2K version
  payload.push_back((2100 >> 8) & 0xFF);
  payload.push_back(0xEC);  // Product code, arbitrary (1772, same as
  payload.push_back(0x06);  // comm_drv_n2k_socketcan.cpp placeholder)

  AddStr(&payload, "OpenCPN", 32);                       // Model ID
  AddStr(&payload, PACKAGE_VERSION, 32);                 // Software version
  AddStr(&payload, PACKAGE_VERSION, 32);                 // Model version
  AddStr(&payload, std::to_string(m_uniqueNumber), 32);  // Serial number

  payload.push_back(0);  // Certification level
  payload.push_back(0);  // Load equivalency

  int source = m_sourceAddress.load();
  // Guard against an invalid source address
  if (source < 0 || source > 253) {
    return false;
  }

  std::vector<std::string> slcan_strings =
      FormatCanMessage(6, static_cast<uint8_t>(source), 0xFF, 126996, payload);
  bool result = true;
  for (const auto& slcan_string : slcan_strings) {
    result = WriteFrame(slcan_string) && result;
  }
  return result;
}

// Process Messages (which are in the stupid f...g Actisense format) and handle
// the housekeeping PGN's Address Claims and ISO Requests
// BUG BUG Contemplate support for PGN 126993 Heartbeat
void CommDriverN2KCanable::ProcessMessages(
    const std::shared_ptr<const Nmea2000Msg>& msg) {
  // Mirrors Worker::ProcessRxMessages() in comm_drv_n2k_socketcan.cpp
  uint8_t sourceAddress = m_sourceAddress.load();

  // Handle PGGN 59904 ISO request if addressed directly to us or a broadcast
  if (msg->PGN.pgn == 59904 && msg->payload.size() > 15 &&
      (msg->payload.at(6) == static_cast<uint8_t>(sourceAddress) ||
       msg->payload.at(6) == 0xFF)) {
    unsigned long requested_pgn = 0;
    requested_pgn = static_cast<unsigned long>(msg->payload.at(15)) << 16;
    requested_pgn += static_cast<unsigned long>(msg->payload.at(14)) << 8;
    requested_pgn += msg->payload.at(13);

    switch (requested_pgn) {
      case 60928:
        SendAddressClaim(sourceAddress);
        break;
      case 126996:
        SendProductInfo();
        break;
      default:
        break;
    }

    // Handle PGN 60928 Address Claim
  } else if (msg->PGN.pgn == 60928 && msg->payload.size() >= 21) {
    // Perform the address claim dance if it is our source address
    if (msg->payload.at(7) == static_cast<uint8_t>(sourceAddress)) {
      uint64_t myName = m_deviceInformation.GetName();
      uint64_t otherName = 0;
      auto* p = reinterpret_cast<uint8_t*>(&otherName);
      for (unsigned int i = 0; i < 8; i++) {
        *p++ = msg->payload.at(13 + i);
      }
      // Compare the two NAME's
      if (otherName < myName) {
        // We lose; try the next address up.
        sourceAddress++;
        // Out of valid addresses!
        if (sourceAddress > 253) {
          sourceAddress = 254;
          wxLogMessage("Canable: Could not claim a valid address");
        }
      }
      // Resend our address claim, either the same address if we won,
      // or the new address after yielding.
      SendAddressClaim(sourceAddress);
    }
  }
}

// Encapsulate the received NMEA 2000 message into the Actisense format
// Build the legacy Actisense-style envelope expected by Nmea2000Msg's
// payload for received messages.
// I will never, ever understand this mind boggling stupid design decision
// Mirrors CommDriverN2KSocketCAN's PushCompleteMsg() / PushFastMsgFragment()
// produce.
void CommDriverN2KCanable::EncodeActisenseMessage(const can_frame& frame) {
  CanHeader header(frame);
  int position = kNotFound;
  bool isFinalMessage = true;

  if (header.IsFastMessage()) {
    position = m_fastMessageMap->FindMatchingEntry(header, frame.data[0]);
    if (position == kNotFound) {
      position = m_fastMessageMap->AddNewEntry();
      isFinalMessage =
          m_fastMessageMap->InsertEntry(header, frame.data, position);
    } else {
      isFinalMessage =
          m_fastMessageMap->AppendEntry(header, frame.data, position);
    }
  }
  if (!isFinalMessage) {
    // Still waiting on more fast packet frames.
    return;
  }
  // Have the complete fast message
  std::vector<uint8_t> envelope;
  envelope.push_back(0x93);  // Actisense Command Id
  if (header.IsFastMessage()) {
    const FastMessageMap::Entry& e = (*m_fastMessageMap)[position];
    envelope.push_back(static_cast<uint8_t>(e.expected_length + 11));
    envelope.push_back(header.priority);
    envelope.push_back(header.pgn & 0xFF);
    envelope.push_back((header.pgn >> 8) & 0xFF);
    envelope.push_back((header.pgn >> 16) & 0xFF);
    envelope.push_back(header.destination);
    envelope.push_back(header.source);
    envelope.push_back(0xFF);  // Time fields: not generated
    envelope.push_back(0xFF);
    envelope.push_back(0xFF);
    envelope.push_back(0xFF);
    envelope.push_back(e.expected_length);
    for (size_t n = 0; n < e.expected_length; n++) {
      envelope.push_back(e.data[n]);
    }
    envelope.push_back(0x55);  // Dummy CRC, not checked downstream.

    // Remove this complete message from the map
    m_fastMessageMap->Remove(position);

    // Single frame message
  } else {
    envelope.push_back(0x13);
    envelope.push_back(header.priority);
    envelope.push_back(header.pgn & 0xFF);
    envelope.push_back((header.pgn >> 8) & 0xFF);
    envelope.push_back((header.pgn >> 16) & 0xFF);
    envelope.push_back(header.destination);
    envelope.push_back(header.source);
    envelope.push_back(0xFF);
    envelope.push_back(0xFF);
    envelope.push_back(0xFF);
    envelope.push_back(0xFF);
    envelope.push_back(CAN_MAX_DLEN);
    for (size_t n = 0; n < CAN_MAX_DLEN; n++) {
      envelope.push_back(frame.data[n]);
    }
    envelope.push_back(0x55);
  }

  // Send the formatted messages to the listener as well as locally for NMEA
  // 2000 housekeeping
  auto address = GetAddress(m_deviceInformation);
  auto message =
      std::make_shared<const Nmea2000Msg>(header.pgn, envelope, address);

  // Perform local housekeeping
  ProcessMessages(message);

  // Send to OpenCPN
  m_driverListener.Notify(message);

  // Should really be the NMEA 2000  data size rather then the whole Actisense
  // encapsulation
  m_driverStatistics.rx_count += envelope.size();
}

// Builds one SLCAN "T<8-hex-id><1-hex-len><hex-data>\r" message from a can
// frame.
std::string CommDriverN2KCanable::EncodeSLCANFrame(
    uint32_t can_id, const std::vector<uint8_t>& data) {
  std::ostringstream os;
  os << "T" << std::hex << std::uppercase;
  os.width(8);
  os.fill('0');
  os << (can_id & 0x1FFFFFFFu);
  os << std::dec << data.size();
  os << std::hex << std::uppercase;
  for (uint8_t value : data) {
    os.width(2);
    os.fill('0');
    os << static_cast<int>(value);
  }
  os << "\r";
  return os.str();
}

// Formats a CAN Message into Can Id and Payload and then Encodes it in SLCAN
// Format returns a vector of SLCAN formatted strings
std::vector<std::string> CommDriverN2KCanable::FormatCanMessage(
    uint8_t priority, uint8_t source, uint8_t destination, uint64_t pgn,
    const std::vector<uint8_t>& data) {
  std::vector<std::string> slcan_strings;

  uint32_t can_id =
      static_cast<uint32_t>(BuildCanID(priority, source, destination, pgn));

  if (!IsFastMessagePGN(pgn)) {
    // Single frame message
    slcan_strings.push_back(EncodeSLCANFrame(can_id, data));
    return slcan_strings;
  }

  // Fast Packet fragmentation
  int sequence;
  {
    std::lock_guard<std::mutex> lock(m_sequenceNumberMutex);
    m_sequenceNumber = (m_sequenceNumber + 0x20) & 0xE0;
    sequence = m_sequenceNumber;
  }

  // First Frame [sequence][total length][6 data bytes]
  size_t offset = std::min<size_t>(6, data.size());
  {
    std::vector<uint8_t> frame;
    frame.push_back(static_cast<uint8_t>(sequence));
    frame.push_back(static_cast<uint8_t>(data.size()));
    frame.insert(frame.end(), data.begin(), data.begin() + offset);
    frame.resize(8, 0xFF);  // Pad unused bytes with 0xFF.
    slcan_strings.push_back(EncodeSLCANFrame(can_id, frame));
    sequence++;
  }

  // Subsequent Frames [sequence][7 data bytes].
  while (offset < data.size()) {
    std::vector<uint8_t> frame;
    frame.push_back(static_cast<uint8_t>(sequence));
    size_t n = std::min<size_t>(7, data.size() - offset);
    frame.insert(frame.end(), data.begin() + offset, data.begin() + offset + n);
    frame.resize(8, 0xFF);  // Pad unused bytes with 0xFF.
    slcan_strings.push_back(EncodeSLCANFrame(can_id, frame));
    offset += n;
    sequence++;
  }

  return slcan_strings;
}

// Invoked by OpenCPN to send a NMEA 2000 message to the outside world
bool CommDriverN2KCanable::SendMessage(std::shared_ptr<const NavMsg> msg,
                                       std::shared_ptr<const NavAddr> addr) {
  // Port never opened, or already closed.
  if (!m_isPortOpen) {
    return false;
  }
  if (!msg) {
    return false;
  }

  int source = m_sourceAddress.load();
  // Guard against an invalid address
  if (source < 0 || source > 253) {
    return false;
  }

  auto n2kMessage = std::dynamic_pointer_cast<const Nmea2000Msg>(msg);
  if (!n2kMessage) {
    return false;
  }
  auto destinationAddress = std::static_pointer_cast<const NavAddr2000>(addr);
  // Either a specific address or a broadcast address
  uint8_t destination = destinationAddress ? destinationAddress->address : 0xFF;

  // We receive an unencapsulated NMEA 2000 message (fortunately no Actisense
  // envelope)
  std::vector<std::string> slcan_strings =
      FormatCanMessage(n2kMessage->priority, static_cast<uint8_t>(source),
                       destination, n2kMessage->PGN.pgn, n2kMessage->payload);

  bool result = true;
  for (const auto& slcan_string : slcan_strings) {
    result = WriteFrame(slcan_string) && result;
  }

  // Update our stats, although not entirely accurate!
  m_driverStatistics.tx_count += n2kMessage->payload.size();
  return result;
}
