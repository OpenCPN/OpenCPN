/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin
 * Author:   David Register
 *           Dave Cowell
 *           Kees Verruijt
 *           Douwe Fokkema
 *           Sean D'Epagnier
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register              bdbcat@yahoo.com *
 *   Copyright (C) 2012-2013 by Dave Cowell                                *
 *   Copyright (C) 2012-2016 by Kees Verruijt         canboat@verruijt.net *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#ifndef _RADARRECEIVE_H_
#define _RADARRECEIVE_H_

#include "RadarControl.h"

PLUGIN_BEGIN_NAMESPACE

//
// The base class for a specific implementation of a thread
// that receives data from a radar.
//

class RadarReceive : public wxThread {
 public:
  RadarReceive(radar_pi *pi, RadarInfo *ri) : wxThread(wxTHREAD_JOINABLE) {
    Create(1024 * 1024);  // Stack size, be liberal
    m_pi = pi;            // This allows you to access the main plugin stuff
    m_ri = ri;            // and this the per-radar stuff
  }

  virtual ~RadarReceive() {}

  virtual void *Entry(void) = 0;

  /*
   * GetInfoStatus
   *
   * Return a string that explains whether the radar has been seen,
   * if interesting at which IP address or whatever, and whether it is functional.
   *
   * It can include newlines. It is presented to the end users, so it must be
   * a translated string.
   */
  virtual wxString GetInfoStatus() = 0;
  virtual void SetInfoStatus(wxString s) {};

  /*
   * Shutdown
   *
   * Called when the thread should stop.
   * It should stop running.
   */
  virtual void Shutdown(void) = 0;

 protected:
  radar_pi *m_pi;
  RadarInfo *m_ri;
};

PLUGIN_END_NAMESPACE

#endif /* _RADARRECEIVE_H_ */
