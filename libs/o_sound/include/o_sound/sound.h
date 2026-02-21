/***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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
 * Top level sound interface
 */

#ifndef OCPN_SOUND_H_
#define OCPN_SOUND_H_

#include <functional>
#include <string>

typedef std::function<void(void* userPtr)> AudioDoneCallback;

namespace o_sound {

class Sound;  // forward
extern Sound *g_anchorwatch_sound;   ///< Global instance


/**
 * Sound  class supports playing a sound using synchronous or asynchronous
 * mode. Also supports sound device enumeration and various status
 * checks.
 *
 * Instances should normally be obtained using SoundFactory();
 */
class Sound {
  friend Sound* Factory(const char* system_command);

public:
  virtual ~Sound();

  /** Return number of available devices. */
  [[nodiscard]] virtual int DeviceCount() const { return 1; }

  /** Return free-format info on device or "" if not available. */
  virtual std::string GetDeviceInfo(int deviceIndex) { return ""; }

  /** Return true if given device is an output device. */
  [[nodiscard]] virtual bool IsOutputDevice(int deviceIndex) const { return true; }

  /**
   * Callback invoked as cb(userData) when audio stream is done and
   * drained. Should be called before Load() to be effective.
   *
   * Setting a non-null callback forces use of asynchronous mode.
   * Using a default, 0 argument restores to synchronous mode.
   *
   * Deleting a pointer owning the Sound object in the callback is
   * not supported.
   *
   * NOTE: Callback might be invoked in an interrupt or thread context,
   * avoid anything which might block (I/O, memory allocation etc.).
   */
  virtual void SetFinishedCallback(AudioDoneCallback cb, void* user_data);

  virtual void SetFinishedCallback(AudioDoneCallback cb) {
    return SetFinishedCallback(std::move(cb), nullptr);
  }

  virtual void SetFinishedCallback() {
    return SetFinishedCallback(nullptr, nullptr);
  }

  /**
   * Initiate the class, loading data from given path and using the
   * given device as output. DeviceIx == -1 implies default device.
   */
  virtual bool Load(const char* path, int deviceIx) = 0;
  virtual bool Load(const char* path) { return Load(path, -1); }

  /**
   * De-Initiate the class, closing any open sound file,
   * preparing for a new Load() using the same Sound instance
   */
  virtual void UnLoad() = 0;

  /**
   * Plays the file loaded by Load(). If a callback is defined using
   * SetFinishedCallback() the playback is asynchronous, otherwise the
   * call blocks until the playback has completed.
   *
   * If the sound has not been successfully Load()'ed or another sound
   * is currently played the request is dropped and logged returning
   * false.
   *
   * Otherwise, in synchronous mode returns success/failure from
   * backend. Asynchronous mode returns true.
   */
  virtual bool Play() = 0;

  /**
   * Stop possible playback and unload buffers, allowed even if
   * not playing. Returns true if stopping is supported, else false.
   */
  virtual bool Stop() = 0;

  /** Reflects loading errors. */
  [[ nodiscard]] virtual bool IsOk() const { return m_ok; }

  /** Hook to use as end of playing in synchronous mode, */
  virtual void OnSoundDone() {}

protected:
  /** Default ctor. Load() must be called before actual usage. */
  Sound();

  bool m_ok;
  int m_device_ix;
  std::string m_soundfile;
  AudioDoneCallback m_on_finished;
  void* m_callback_data;
};

} // namespace o_sound

#endif  // OCPN_SOUND_H_
