/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef CHARTDLDR_SCHEDULER_H_
#define CHARTDLDR_SCHEDULER_H_

class chartdldr_pi;
class wxTimer;
class wxTimerEvent;
class wxWindow;

class ChartDldrScheduler {
public:
  explicit ChartDldrScheduler(chartdldr_pi* pi);
  ~ChartDldrScheduler();

  void Attach(wxWindow* parent);
  void Detach();
  void Restart();

private:
  void OnTimer(wxTimerEvent& event);

  chartdldr_pi* pi_;
  wxWindow* parent_;
  wxTimer* timer_;
};

#endif  // CHARTDLDR_SCHEDULER_H_
