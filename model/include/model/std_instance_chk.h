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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Posix native instance check implementation
 */

#ifndef STD_INSTANCE_CHECK_H_
#define STD_INSTANCE_CHECK_H_

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

#endif  // STD_INSTANCE_CHECK_H_
