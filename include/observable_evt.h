/*************************************************************************
 *
 * Project:  OpenCPN
 * Purpose: wxCommandEvt subclass which can carry also a shared_ptr<void>
 *
 * Copyright (C) 2022 Alec Leamas
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.
 **************************************************************************/

#ifndef OBSERVABLE_EVT_H  // Guard also used in ocpn_plugin.h
#define OBSERVABLE_EVT_H

#include <memory>

#include <wx/event.h>

/** The event used by notify/listen. */
class ObservedEvt;

wxDECLARE_EVENT(obsNOTIFY, ObservedEvt);

/** Adds a std::shared<void> element to wxCommandEvent. */
class ObservedEvt : public wxCommandEvent {
public:
  ObservedEvt(wxEventType commandType = obsNOTIFY, int id = 0)
      : wxCommandEvent(commandType, id) {}

  ObservedEvt(const ObservedEvt& event) : wxCommandEvent(event) {
    this->m_shared_ptr = event.m_shared_ptr;
  }

  wxEvent* Clone() const { return new ObservedEvt(*this); }

  std::shared_ptr<const void> GetSharedPtr() const { return m_shared_ptr; }

  void SetSharedPtr(std::shared_ptr<const void> p) { m_shared_ptr = p; }

private:
  std::shared_ptr<const void> m_shared_ptr;
};

#endif  // OBSERVABLE_EVT_H
