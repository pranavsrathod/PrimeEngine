#include "SoundComponent.h"
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"
#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "DebugRenderer.h"
#include <../Code/PrimeEngine/Sound/SoundManager.h>
using namespace PE::Events;

namespace PE {
    namespace Components {

        PE_IMPLEMENT_CLASS1(SoundComponent, Component);

        // Constructor
        SoundComponent::SoundComponent(PE::GameContext& context, PE::MemoryArena arena, Handle hMyself)
            : Component(context, arena, hMyself)
        {
            m_arena = arena;
            m_pContext = &context;
        }

        // Destructor
        SoundComponent::~SoundComponent()
        {
            alDeleteSources(1, &m_source);
            alDeleteBuffers(1, &m_buffer);
        }

        void SoundComponent::addDefaultComponents()
        {
           
            Component::addDefaultComponents();
            PE_REGISTER_EVENT_HANDLER(Events::Event_UPDATE, SoundComponent::do_UPDATE);
            // Optionally, register events here in future (e.g. play on trigger etc.)
        }

        bool SoundComponent::loadWav(const char* filename)
        {
            alcMakeContextCurrent(SoundManager::Instance()->m_pContext);

            // Load WAV using libsndfile
            SF_INFO sfInfo;
            SNDFILE* sndFile = sf_open(filename, SFM_READ, &sfInfo);
            if (!sndFile)
            {
                PEINFO("SoundComponent: Failed to open WAV file: %s\n", filename);
                return false;
            }

            short* membuf = new short[sfInfo.frames * sfInfo.channels];
            sf_read_short(sndFile, membuf, sfInfo.frames * sfInfo.channels);
            sf_close(sndFile);

            ALenum format;
            if (sfInfo.channels == 1)
                format = AL_FORMAT_MONO16;
            else if (sfInfo.channels == 2)
                format = AL_FORMAT_STEREO16;
            else
            {
                PEINFO("SoundComponent: Unsupported channel count: %d\n", sfInfo.channels);
                delete[] membuf;
                return false;
            }

            alGenBuffers(1, &m_buffer);
            alBufferData(m_buffer, format, membuf, sfInfo.frames * sfInfo.channels * sizeof(short), sfInfo.samplerate);
            ALenum error = alGetError();
            if (error != AL_NO_ERROR)
            {
                PEINFO("SoundComponent: Error after alBufferData: %d\n", error);
            }
            else
            {
                PEINFO("SoundComponent: alBufferData success\n");
            }
            delete[] membuf;

            alGenSources(1, &m_source);
            alSourcei(m_source, AL_BUFFER, m_buffer);
            // Set 3D attenuation parameters
            alSourcef(m_source, AL_REFERENCE_DISTANCE, 1.0f);   // Full volume within 1 unit
            alSourcef(m_source, AL_MAX_DISTANCE, 20.0f);        // Stops attenuating beyond 20 units
            alSourcef(m_source, AL_ROLLOFF_FACTOR, 1.0f);       // Default fade speed
            alSourcef(m_source, AL_GAIN, 1.0f);
            PEINFO("SoundComponent: Source gain set to 1.0\n");

            PEINFO("SoundComponent: Loaded WAV file successfully: %s\n", filename);
            return true;
        }

        void SoundComponent::play(bool loop)
        {
            alcMakeContextCurrent(SoundManager::Instance()->m_pContext);

            m_looping = loop;
            alSourcei(m_source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
            alSourcePlay(m_source);
            ALenum errorPlay = alGetError();
            if (errorPlay != AL_NO_ERROR)
            {
                PEINFO("SoundComponent: Error after alSourcePlay: %d\n", errorPlay);
            }
            else
            {
                ALint state;
                alGetSourcei(m_source, AL_SOURCE_STATE, &state);
                if (state == AL_PLAYING)
                {
                    PEINFO("SoundComponent: Sound is playing!\n");
                }
                else
                {
                    PEINFO("SoundComponent: Sound is NOT playing, state = %d\n", state);
                }
            }
        }

        void SoundComponent::stop()
        {
            alSourceStop(m_source);
        }

        void SoundComponent::pause()
        {
            alSourcePause(m_source);
        }

        void SoundComponent::resume()
        {
            alSourcePlay(m_source);
        }

        void SoundComponent::setGain(float gain)
        {
            m_gain = gain;
            if (!m_muted)
                alSourcef(m_source, AL_GAIN, m_gain);
        }

        void SoundComponent::setMute(bool mute)
        {
            m_muted = mute;
            if (m_muted)
            {
                m_savedGain = m_gain;
                alSourcef(m_source, AL_GAIN, 0.0f);
            }
            else
            {
                alSourcef(m_source, AL_GAIN, m_gain);
            }
        }

        void SoundComponent::setLooping(bool loop)
        {
            m_looping = loop;
            alSourcei(m_source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
        }

        void SoundComponent::setMinDistance(float dist)
        {
            m_minDistance = dist;
            alSourcef(m_source, AL_REFERENCE_DISTANCE, dist);
        }

        void SoundComponent::setMaxDistance(float dist)
        {
            m_maxDistance = dist;
            alSourcef(m_source, AL_MAX_DISTANCE, dist);
        }

        void SoundComponent::setRolloff(float rolloff)
        {
            m_rolloff = rolloff;
            alSourcef(m_source, AL_ROLLOFF_FACTOR, rolloff);
        }

        void SoundComponent::setPosition(const Vector3& pos)
        {
            m_position = pos;
            alSource3f(m_source, AL_POSITION, pos.m_x, pos.m_y, pos.m_z);
        }

        //void SoundComponent::do_UPDATE(Events::Event* pEvt)
        //{
        //    Events::Event_UPDATE* pUpdateEvt = static_cast<Events::Event_UPDATE*>(pEvt);
        //    PEINFO("SOUND COMPONENT DO UPDATE");
        //    /*PE::Components::SoundManager::Instance()->updateListenerFromCamera();*/

        //}

        void SoundComponent::do_UPDATE(Events::Event* pEvt)
        {
            Events::Event_UPDATE* pUpdateEvt = static_cast<Events::Event_UPDATE*>(pEvt);

            PE::Components::SoundManager::Instance()->updateListenerFromCamera();

            Camera* cam = CameraManager::Instance()->getActiveCamera();
            if (!cam) return;

            CameraSceneNode* camSN = cam->getCamSceneNode();
            if (!camSN) return;

            Vector3 camPos = camSN->m_worldTransform.getPos();
            float distance = (m_position - camPos).length();
            float cutoffDistance = 15.0f;

            ALint state;
            alGetSourcei(m_source, AL_SOURCE_STATE, &state);

            if (distance > cutoffDistance && state == AL_PLAYING)
            {
                alSourceStop(m_source);
                m_stoppedByDistance = true;
            }
            else if (distance <= cutoffDistance && m_stoppedByDistance && state != AL_PLAYING)
            {
                alSourcePlay(m_source);
                m_stoppedByDistance = false;
            }
        }
        //void SoundComponent::do_UPDATE(Events::Event* pEvt)
        //{
        //    Events::Event_UPDATE* pUpdateEvt = static_cast<Events::Event_UPDATE*>(pEvt);
        //    PEINFO("SOUND COMPONENT DO UPDATE\n");

        //    PE::Components::SoundManager::Instance()->updateListenerFromCamera();

        //    Camera* cam = CameraManager::Instance()->getActiveCamera();
        //    if (!cam) return;

        //    CameraSceneNode* camSN = cam->getCamSceneNode();
        //    if (!camSN) return;

        //    Vector3 camPos = camSN->m_worldTransform.getPos();
        //    float distance = (m_position - camPos).length();
        //    float cutoffDistance = 15.0f;

        //    ALint state;
        //    alGetSourcei(m_source, AL_SOURCE_STATE, &state);

        //    if (distance > cutoffDistance && state == AL_PLAYING)
        //    {
        //        alSourceStop(m_source);
        //        PEINFO("SoundComponent: Sound stopped � camera too far.\n");
        //    }
        //    else if (distance <= cutoffDistance && state != AL_PLAYING)
        //    {
        //        alSourcePlay(m_source);
        //        PEINFO("SoundComponent: Sound resumed � camera in range.\n");
        //    }

        //    // --- Debug text rendering ---
        //    ALfloat gain = 0.0f;
        //    alGetSourcef(m_source, AL_GAIN, &gain);

        //    char buf[128];
        //    sprintf(buf, "Distance: %.2f | Gain: %.2f", distance, gain);

        //    Vector3 textPos = m_position + Vector3(0.0f, 0.5f, 0.0f);

        //    if (DebugRenderer::Instance())
        //    {
        //        //DebugRenderer::Instance()->createTextMesh(
        //        //    buf,
        //        //    false,  // isOverlay2D
        //        //    false,  // is3D
        //        //    true,   // is3DFacedToCamera
        //        //    false,  // is3DFacedToCameraLockedYAxis
        //        //    2.0f,   // timeToLive (avoids exhausting the pool)
        //        //    textPos,
        //        //    0.01f,
        //        //    m_pContext->m_gameThreadThreadOwnershipMask  // fallback thread ownership mask
        //        //);
        //    }
        //}
    }; // namespace Components
}; // namespace PE