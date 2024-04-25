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
#ifndef _STD_INSTANCE_CHECK_H__
#define _STD_INSTANCE_CHECK_H__

#include <wx/snglinst.h>

#include "model/instance_check.h"

/**  InstanceCheck implementation based on <unistd.h> i. e. Linux/MacOS */
class StdInstanceCheck : public InstanceCheck {
public:
  StdInstanceCheck();

  virtual ~StdInstanceCheck();

  bool IsMainInstance() override;

  void CleanUp() override;

private:
  std::string m_path;
  bool m_is_main_instance;
};

#endif  // _STD_INSTANCE_CHECK_H__
