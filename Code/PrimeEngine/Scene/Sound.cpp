#include "Sound.h"
#include <stdio.h>

namespace PE {
    namespace Components {
        
        Sound::Sound(const char* filename)
            : m_buffer(0), m_source(0)
        {
            // Open the sound file
            SF_INFO sfInfo;
            SNDFILE* sndFile = sf_open(filename, SFM_READ, &sfInfo);
            if (!sndFile)
            {
                printf("Failed to open sound file: %s\n", filename);
                return;
            }

            // Check format
            if (sfInfo.frames < 1 || sfInfo.frames >(sf_count_t)(INT_MAX / sizeof(short)))
            {
                printf("Bad sample count in %s\n", filename);
                sf_close(sndFile);
                return;
            }

            // Read samples
            short* membuf = new short[sfInfo.frames * sfInfo.channels];
            sf_read_short(sndFile, membuf, sfInfo.frames * sfInfo.channels);
            sf_close(sndFile);

            // Determine format
            ALenum format;
            if (sfInfo.channels == 1)
                format = AL_FORMAT_MONO16;
            else if (sfInfo.channels == 2)
                format = AL_FORMAT_STEREO16;
            else
            {
                printf("Unsupported channel count: %d\n", sfInfo.channels);
                delete[] membuf;
                return;
            }

            // Create buffer and upload data
            alGenBuffers(1, &m_buffer);
            alBufferData(m_buffer, format, membuf, sfInfo.frames * sfInfo.channels * sizeof(short), sfInfo.samplerate);
            delete[] membuf;

            // Create source
            alGenSources(1, &m_source);
            alSourcei(m_source, AL_BUFFER, m_buffer);
        }

        Sound::~Sound()
        {
            alDeleteSources(1, &m_source);
            alDeleteBuffers(1, &m_buffer);
        }

        void Sound::play(bool loop)
        {
            alSourcei(m_source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
            alSourcePlay(m_source);
        }

        void Sound::stop()
        {
            alSourceStop(m_source);
        }

        void Sound::setPosition(const Vector3& pos)
        {
            alSource3f(m_source, AL_POSITION, pos.m_x, pos.m_y, pos.m_z);
        }

    }; // namespace Components
}; // namespace PE