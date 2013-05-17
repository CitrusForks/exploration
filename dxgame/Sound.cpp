#include "StdAfx.h"
#include "Sound.h"
#include <iostream>

#pragma comment(lib, "fmodex_vc.lib")



static void ERRCHECK(FMOD_RESULT rc)
{
    if (rc != FMOD_OK)
    {
        std::cerr << "FMOD error. :(" << std::endl;
    }
}


Sound::Sound(void)
{
    // This is the required startup sequence from the "getting started" document! Do not change! 
    // If anything needs adding, append it after the required startup code.
    FMOD_RESULT result;
    unsigned int version;
    int numdrivers;
    char name[256];
    /*
    Create a System object and initialize.
    */
    result = FMOD::System_Create(&system);
    ERRCHECK(result);
    result = system->getVersion(&version);
    ERRCHECK(result);
    if (version < FMOD_VERSION)
    {
        printf("Error! You are using an old version of FMOD %08x. This program requires %08x\n", version, FMOD_VERSION);
        system = nullptr;
        return;
    }
    result = system->getNumDrivers(&numdrivers);
    ERRCHECK(result);
    if (numdrivers == 0)
    {
        result = system->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
        ERRCHECK(result);
    }
    else
    {
        result = system->getDriverCaps(0, &caps, 0, &speakermode); // this argument list was wrong in their document .__.
        ERRCHECK(result);
        /*
        Set the user selected speaker mode.
        */
        result = system->setSpeakerMode(speakermode);
        ERRCHECK(result);
        if (caps & FMOD_CAPS_HARDWARE_EMULATED)
        {
            /*
            The user has the 'Acceleration' slider set to off! This is really bad
            for latency! You might want to warn the user about this.
            */
            result = system->setDSPBufferSize(1024, 10);
            ERRCHECK(result);
        }
        result = system->getDriverInfo(0, name, 256, 0);
        ERRCHECK(result);
        if (strstr(name, "SigmaTel"))
        {
            /*
            Sigmatel sound devices crackle for some reason if the format is PCM 16bit.
            PCM floating point output seems to solve it.
            */
            result = system->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0,0,
            FMOD_DSP_RESAMPLER_LINEAR);
            ERRCHECK(result);
        }
    }
    result = system->init(100, FMOD_INIT_NORMAL, 0);
    if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)
    {
        /*
        Ok, the speaker mode selected isn't supported by this soundcard. Switch it
        back to stereo...
        */
        result = system->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
        ERRCHECK(result);
        /*
        ... and re-init.
        */
        result = system->init(100, FMOD_INIT_NORMAL, 0);
    }
    ERRCHECK(result);

    // end of required startup code
}

Sound::~Sound(void)
{
    // uh...

    for (auto i = samples.begin(); i != samples.end(); ++i) (*i)->release(); 

    system->release(); 
}

void Sound::perFrameUpdate()
{
    system->update();
}

int Sound::loadSound(char *path)
{
    FMOD::Sound *s;
    
    if (system->createSound(path, FMOD_DEFAULT, 0, &s) != FMOD_OK)
    {
        std::cerr << "Could not load: " << path << std::endl;
        return -1;
    }

    samples.push_back(s);

    return samples.size() - 1;
}

void Sound::play(int sound, FMOD::Channel **channel)
{
    if (sound < 0 || (unsigned)sound >= samples.size())
    {
        std::cerr << "There is no sound numbered " << sound << std::endl;
        return;
    }

    if (system->playSound(FMOD_CHANNEL_FREE, samples[sound], false, channel) != FMOD_OK)
    {
        std::cerr << "Error playing a sound. :(" << std::endl;
    }
}