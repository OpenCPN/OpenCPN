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
 * Implement port_audio_sound.h -- Sound backend based on PortAudio
 */

#include <algorithm>
#include <atomic>
#include <memory>

#include <portaudio.h>
#include <wx/log.h>

#include "snd_config.h"
#include "o_sound/port_audio_sound.h"

using namespace o_sound_private;

/**
 * The module contains a critical region, where notably isAsynch
 * and m_onFinished are subject to race conditions between the
 * code in the main thread and the callbacks. To make things more
 * interesting we cannot know for sure in which context the callbacks
 * are invoked. Simple spin locks which should work in any case
 * are used to resolve the mess.
 *
 */

static constexpr int BUFSIZE = 1024;  // Frames per buffer.
static constexpr int LOCK_SLEEP_MS = 2;
static constexpr int LOCK_MAX_TRIES = 100;

static inline void IgnoreRetval(const FILE* f) { (void)f; }

/*
 * Called by the PortAudio engine when audio is needed. It may be called at
 * interrupt level on some platforms so calls which might block such as
 * malloc()/free() or IO should be avoided.
 */
static int PortAudioSoundCallback(const void* inputBuff, void* outputBuff,
                                  unsigned long framesPerBuff,
                                  const PaStreamCallbackTimeInfo* timeInfo,
                                  PaStreamCallbackFlags flags, void* userData) {
  auto paSound = static_cast<PortAudioSound*>(userData);
  return paSound->SoundCallback(outputBuff, framesPerBuff, timeInfo, flags);
}

/** Callback invoked when the stream has completed. */
static void PortAudioDoneCallback(void* userData) {
  PortAudioSound* sound = static_cast<PortAudioSound*>(userData);
  sound->DoneCallback();
}

/** Ensure the stream is successfully started, possibly restarted. */
static bool startStream(PaStream* stream) {
  PaError err = paNoError;
  if (!Pa_IsStreamStopped(stream)) {
    err = Pa_StopStream(stream);
    if (err != paNoError) {
      wxLogWarning("PortAudio: startStream: Cannot stop stream: %s\n",
                   Pa_GetErrorText(err));
    }
  }
  err = Pa_StartStream(stream);
  if (err != paNoError) {
    wxLogWarning("PortAudio: startStream: Cannot start stream: %s\n",
                 Pa_GetErrorText(err));
  }
  return err == paNoError;
}

/** Do the Pa_OpenStream dirty work. */
static bool openStream(PaStream** stream, int deviceIx,
                       std::unique_ptr<AbstractSoundLoader>& sound_loader,
                       PaStreamCallback callback,
                       PortAudioSound* portAudioSound)

{
  PaStreamParameters outputParameters;
  outputParameters.device = deviceIx;
  outputParameters.channelCount = sound_loader->GetChannelCount();
  outputParameters.sampleFormat = paInt16;
  outputParameters.suggestedLatency =
      Pa_GetDeviceInfo(outputParameters.device)->defaultHighOutputLatency;
  outputParameters.hostApiSpecificStreamInfo = NULL;
  PaError err = Pa_OpenStream(stream, NULL, /* no input channels */
                              &outputParameters, sound_loader->GetSamplingRate(),
                              BUFSIZE, paNoFlag, callback, portAudioSound);
  if (err != paNoError) {
    wxLogWarning("PortAudio Create() error: %s", Pa_GetErrorText(err));
    return false;
  }
  return true;
}

/** Dump the complete soundLoader buffer to stream using blocking write. */
static bool writeSynchronous(int deviceIx,
                             std::unique_ptr<AbstractSoundLoader>& soundLoader,
                             PortAudioSound* portAudioSound) {
  PaStream* stream;
  if (!openStream(&stream, deviceIx, soundLoader, 0, portAudioSound)) {
    return false;
  }
  if (!startStream(stream)) {
    return false;
  }
  unsigned buff[BUFSIZE * soundLoader->GetBytesPerSample()];
  PaError pe = paNoError;
  int len = soundLoader->Get(buff, sizeof(buff));
  for (; len > 0; len = soundLoader->Get(buff, sizeof(buff))) {
    len /= soundLoader->GetBytesPerSample();
    pe = Pa_WriteStream(stream, buff, len);
    if (pe != paNoError) {
      wxLogWarning("PortAudio: Cannot write stream: %s", Pa_GetErrorText(pe));
      break;
    }
  }
  Pa_CloseStream(stream);
  return pe == paNoError;
}

PortAudioSound::PortAudioSound()
    : m_sound_loader(SoundLoaderFactory()), m_lock(ATOMIC_FLAG_INIT) {
  if (!getenv("OCPN_DEBUG_ALSA"))
    IgnoreRetval(freopen("/dev/null", "w", stderr));
  m_stream = NULL;
  m_is_asynch = false;
  m_is_pa_initialized = false;
  PaError err = Pa_Initialize();
  if (err != paNoError) {
    IgnoreRetval(freopen("/dev/tty", "w", stderr));
    wxLogError("PortAudio; cannot initialize: %s", Pa_GetErrorText(err));
    return;
  }
  m_is_pa_initialized = true;
  PortAudioSound::SetDeviceIndex(-1);
  for (int i = 0; i < PortAudioSound::DeviceCount(); i += 1) {
    const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
    wxLogDebug("Device: %d: %s", i, info->name);
  }
  IgnoreRetval(freopen("/dev/tty", "w", stderr));
}

PortAudioSound::~PortAudioSound() {
  if (m_is_pa_initialized) {
    PaError pe = Pa_Terminate();
    if (pe != paNoError) {
      wxLogWarning("PortAudio: Terminate error: %s", Pa_GetErrorText(pe));
    }
  }
}

/** Simple spin lock to critical module region. */

void PortAudioSound::lock() {
  for (int i = 0; i < LOCK_MAX_TRIES; i += 1) {
    if (m_lock.test_and_set(std::memory_order_acquire) == 0) {
      return;
    }
    Pa_Sleep(LOCK_SLEEP_MS);  // spin
  }
  wxLogWarning("PortAudio: Cannot acquire lock.");
  return;
}

/** Release lock acquired with lock(). */
void PortAudioSound::unlock() { m_lock.clear(std::memory_order_release); }

void PortAudioSound::SetFinishedCallback(AudioDoneCallback cb, void* userData) {
  lock();
  m_on_finished = cb;
  m_callback_data = userData;
  m_is_asynch = (bool)cb;
  unlock();
}

std::string PortAudioSound::GetDeviceInfo(int deviceIndex) {
  return std::string(Pa_GetDeviceInfo(deviceIndex)->name);
}

bool PortAudioSound::Load(const char* path, int deviceIndex) {
  lock();
  m_ok = false;
  if (!m_sound_loader->Load(path)) {
    wxLogWarning("Cannot load sound file %s", path);
    unlock();
    return false;
  }
  m_ok = true;
  unlock();
  return true;
}

void PortAudioSound::UnLoad() {
  lock();
  m_sound_loader->UnLoad();
  m_ok = false;
  unlock();
}

bool PortAudioSound::SetDeviceIndex(int device_index) {
  if (device_index < -1 || device_index >= DeviceCount()) {
    wxLogWarning("SetDeviceIndex: Illegal index: %d, using default",
                 device_index);
    m_device_ix = -1;
  }
  m_device_ix = device_index == -1 ? Pa_GetDefaultOutputDevice() : device_index;
  return true;
}

bool PortAudioSound::IsOutputDevice(int deviceIndex) const {
  if (deviceIndex < -1 || deviceIndex >= DeviceCount()) {
    wxLogWarning("IsOutputDevice: Illegal index: %d", deviceIndex);
    return false;
  }
  return Pa_GetDeviceInfo(deviceIndex)->maxOutputChannels > 0;
}

int PortAudioSound::DeviceCount() const { return Pa_GetDeviceCount(); }

bool PortAudioSound::Play() {
  if (!m_ok) {
    wxLogWarning("PortAudioSound: cannot play (not loaded)");
    return false;
  }
  lock();
  if (!m_is_asynch) {
    bool ok = writeSynchronous(m_device_ix, m_sound_loader, this);
    unlock();
    return ok;
  }
  if (!m_stream) {
    if (!openStream(&m_stream, m_device_ix, m_sound_loader,
                    PortAudioSoundCallback, this)) {
      unlock();
      return false;
    }
    PaError pe = Pa_SetStreamFinishedCallback(m_stream, PortAudioDoneCallback);
    if (pe != paNoError) {
      wxLogWarning("PortAudio set stream finished callback error: %s",
                   Pa_GetErrorText(pe));
      unlock();
      return false;
    }
  }
  bool ok = startStream(m_stream);
  unlock();
  return ok;
}

bool PortAudioSound::Stop() { return Pa_StopStream(m_stream) == paNoError; }

int PortAudioSound::SoundCallback(void* outputBuffer,
                                  unsigned long framesPerBuffer,
                                  const PaStreamCallbackTimeInfo* timeInfo,
                                  PaStreamCallbackFlags statusFlags) {
  int16_t* dest = static_cast<int16_t*>(outputBuffer);
  int bufferlen = framesPerBuffer * m_sound_loader->GetBytesPerSample();
  int len = m_sound_loader->Get(dest, bufferlen);
  return len == bufferlen ? paContinue : paComplete;
}

void PortAudioSound::DoneCallback() {
  lock();
  if (m_on_finished) {
    m_on_finished(m_callback_data);
  }
  m_on_finished = nullptr;
  unlock();
}
