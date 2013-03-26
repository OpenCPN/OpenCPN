#ifndef __OCPN_SOUND_H__
#define __OCPN_SOUND_H__

#include <wx/sound.h>

#ifdef OCPN_USE_PORTAUDIO
	#include "OCPNSoundData.h"
#endif

class OCPN_Sound: public wxSound
{
public:
    OCPN_Sound();
    ~OCPN_Sound();

    bool IsOk() const;
    bool Create(const wxString& fileName, bool isResource = false);
    bool Play(unsigned flags = wxSOUND_ASYNC) const;
    bool IsPlaying() const;
    void Stop();

private:
    bool m_OK;

#ifdef OCPN_USE_PORTAUDIO
    bool LoadWAV(const wxUint8 *data, size_t length, bool copyData);
    void FreeMem(void);

    OCPNSoundData *m_osdata;
    PaStream *m_stream;
#endif

};

#endif
