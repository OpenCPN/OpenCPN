/***************************************************************************
 *   Copyright (C) 2023 Alec Leamas                                        *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#ifndef _WX_INST_CHECK__
#define _WX_INST_CHECK__

#include <wx/snglinst.h>

#include "model/instance_check.h"

/**  Thin wrapper for wxSingleInstanceChecker implementing InstanceCheck */
class WxInstanceCheck : public InstanceCheck {
public:
  WxInstanceCheck();

  ~WxInstanceCheck() { if (m_checker) delete m_checker; }

  bool IsMainInstance() override;

  void CleanUp() override;

  void OnExit() override;


private:
  void Init();

  wxSingleInstanceChecker* m_checker;
  bool is_inited;
};

#endif   // _WX_INST_CHECK__
