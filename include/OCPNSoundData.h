#ifndef __OCPNSOUNDDATA_H__
#define __OCPNSOUNDDATA_H__

#ifdef OCPN_USE_PORTAUDIO

/// Sound data, as loaded from .wav file:
class OCPNSoundData
{
public:
    OCPNSoundData();
    ~OCPNSoundData();

    // .wav header information:
    unsigned m_channels;       // num of channels (mono:1, stereo:2)
    unsigned m_samplingRate;
    unsigned m_bitsPerSample;  // if 8, then m_data contains unsigned 8bit
    // samples (wxUint8), if 16 then signed 16bit
    // (wxInt16)
    unsigned m_samples;        // length in samples:

    // wave data:
    size_t   m_dataBytes;
    wxUint8 *m_data;           // m_dataBytes bytes of data

    wxUint8 *m_dataWithHeader; // ditto, but prefixed with .wav header
};

#endif

#endif
