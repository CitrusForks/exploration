#ifndef SOUND_H
#define SOUND_H

#include <fmod.hpp>
#include <vector>

class Sound
{
private:
    FMOD::System *system;
    FMOD_SPEAKERMODE speakermode;
    FMOD_CAPS caps;
    char name[256];

    std::vector<FMOD::Sound *> samples;

public:
    Sound(void);
    ~Sound(void);

    void perFrameUpdate(); // calls system->update(); call once per frame, as pre documentation
    int loadSound(char *path); // returns a handle to a loaded sound; the class will take care of deallocating the sound in the destructor
    void play(int sound, FMOD::Channel **out_channel = nullptr);
};

#endif