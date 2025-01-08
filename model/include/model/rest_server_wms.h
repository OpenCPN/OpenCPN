
/***************************************************************************
 *   Copyright (C) 2022 David Register                                     *
 *   Copyright (C) 2022-2023  Alec Leamas                                  *
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

#ifndef RESTSERVERWMS_H_
#define RESTSERVERWMS_H_

#include <atomic>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <string>
#include <thread>
#include <unordered_map>

#include "gdal/cpl_string.h"

#include "observable_evtvar.h"

#include "../gui/include/gui/WmsEvent.h"
#include "../gui/include/gui/viewport.h"
#include "../gui/include/gui/chcanv.h"

#include "../model/georef.h"

#include <wx/event.h>
#include <wx/string.h>
#include <wx/thread.h>  // for wxSemaphore, std::semaphore is c++20





class RestServerWms {
public:
  RestServerWms();
  ~RestServerWms();

  bool StartServer(std::function<void(WmsReqParams)> FCallback);
  void StopServer();

  static unsigned int RestServerWms::m_hitcount;
  /* static wxFrame* m_pWxFrame;
  static ChartCanvas* m_pChartCanvas;

   


  static wxStaticText* pText;*/

  static unsigned int lastSize_W;
  static unsigned int lastSize_H;

  //static void* jpegdatabuffer;

  static std::function<void(WmsReqParams)> fCallback;
  static std::mutex ret_mutex;

private:
  void Run();
  
  /** IoThread interface: Guards return_status */
  

  /** IoThread interface: Guards return_status */
  std::condition_variable return_status_cv;

  const std::string m_endpoint;

  std::atomic<bool> m_alive = true;

  std::thread m_workerthread;

 
};
#endif  // guard
