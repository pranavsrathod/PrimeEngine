#ifndef __PE_SOUND__
#define __PE_SOUND__

#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"
#include "PrimeEngine/Math/Vector3.h"

// External libraries
#include <AL/al.h>
#include <AL/alc.h>
#include <sndfile.h>

namespace PE {
    namespace Components {

        struct Sound
        {
            /*PE_DECLARE_CLASS(SoundManager);*/
        public:
            Sound(const char* filename);
            ~Sound();

            void play(bool loop = false);
            void stop();
            void setPosition(const Vector3& pos);

        private:
            ALuint m_buffer;
            ALuint m_source;
        };

    }; // namespace Components
}; // namespace PE

#endif