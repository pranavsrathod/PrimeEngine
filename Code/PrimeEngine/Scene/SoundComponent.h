#ifndef __PYENGINE_2_0_SOUND_COMPONENT__
#define __PYENGINE_2_0_SOUND_COMPONENT__

#define NOMINMAX

// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
#include <AL/al.h>
#include <AL/alc.h>
#include <sndfile.h>

// Inter-Engine includes
#include "PrimeEngine/MemoryManagement/Handle.h"
#include "../Events/Component.h"
#include "../Events/Event.h"
#include "../Events/StandardEvents.h"

// Sibling/Children includes
#include "PrimeEngine/Math/Vector3.h"

namespace PE {
    namespace Components {

        struct SoundComponent : public Component
        {
            PE_DECLARE_CLASS(SoundComponent);

            SoundComponent(PE::GameContext& context, PE::MemoryArena arena, Handle hMyself);
            virtual ~SoundComponent();

            // Component ------------------------------------------------------------
            virtual void addDefaultComponents();

            // Sound API
            bool loadWav(const char* filename);
            void play(bool loop = false);
            void stop();
            void pause();
            void resume();
            void setPosition(const Vector3& pos);

            // Live controls (called by ImGui panel)
            void setGain(float gain);
            void setMute(bool mute);
            void setLooping(bool loop);
            void setMinDistance(float dist);
            void setMaxDistance(float dist);
            void setRolloff(float rolloff);

            PE::MemoryArena m_arena;
            PE::GameContext* m_pContext;

            // UI-visible state — public so ImGui can hold direct pointers
            float m_gain        = 1.0f;
            float m_savedGain   = 1.0f;  // restored when unmuting
            bool  m_muted       = false;
            bool  m_looping     = false;
            float m_minDistance = 1.0f;
            float m_maxDistance = 20.0f;
            float m_rolloff     = 1.0f;
            bool  m_stoppedByDistance = false; // true only when do_UPDATE silenced it

            bool isPlaying()
            {
                ALint state;
                alGetSourcei(m_source, AL_SOURCE_STATE, &state);
                return state == AL_PLAYING;
            }

            PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
            virtual void do_UPDATE(Events::Event* pEvt);

        protected:
            ALuint m_buffer = 0;
            ALuint m_source = 0;
            Vector3 m_position;
        };

    }; // namespace Components
}; // namespace PE

#endif
