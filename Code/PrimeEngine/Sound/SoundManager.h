#ifndef __PYENGINE_2_0_SOUND_MANAGER__
#define __PYENGINE_2_0_SOUND_MANAGER__

#define NOMINMAX
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes

// Inter-Engine includes
#include "PrimeEngine/MemoryManagement/Handle.h"
#include "../Events/Component.h"
#include "../Events/Event.h"
#include "../Events/StandardEvents.h"
#include <AL/alc.h>
#include <AL/al.h>
#include <PrimeEngine/Scene/CameraManager.h>


// Sibling/Children includes
namespace PE {
	namespace Components {

		struct SoundManager : public Component
		{
			PE_DECLARE_CLASS(SoundManager);

			SoundManager(PE::GameContext& context, PE::MemoryArena arena, Handle hMyself, const char* wbFilename, bool isActive = true) : Component(context, arena, hMyself)
			{
#if APIABSTRACTION_D3D9 || APIABSTRACTION_D3D11
#endif
			}

			virtual ~SoundManager() {
				if (m_pContext)
				{
					alcMakeContextCurrent(nullptr);
					alcDestroyContext(m_pContext);
				}
				if (m_pDevice)
				{
					alcCloseDevice(m_pDevice);
				}
			}

			// Component ------------------------------------------------------------

			virtual void addDefaultComponents()
			{
				Component::addDefaultComponents();

				PE_REGISTER_EVENT_HANDLER(Events::Event_UPDATE, SoundManager::do_UPDATE);
			}

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
			virtual void do_UPDATE(Events::Event* pEvt)
			{
				PEINFO("SOUND MANAGER : DO UPDATE");
#		if APIABSTRACTION_D3D9 || APIABSTRACTION_D3D11
#		endif
			}

			void updateListenerFromCamera()
			{
				Camera* cam = PE::Components::CameraManager::Instance()->getActiveCamera();
				if (!cam)
					return;

				CameraSceneNode* pCamSN = cam->getCamSceneNode();
				if (!pCamSN)
					return;

				Matrix4x4& camTransform = pCamSN->m_worldTransform;
				Vector3 camPos = camTransform.getPos();
				Vector3 forward(camTransform.m[0][2], camTransform.m[1][2], camTransform.m[2][2]);
				Vector3 up(camTransform.m[0][1], camTransform.m[1][1], camTransform.m[2][1]);

				alListener3f(AL_POSITION, camPos.m_x, camPos.m_y, camPos.m_z);

				float orientation[] = {
					forward.m_x, forward.m_y, forward.m_z,
					up.m_x, up.m_y, up.m_z
				};
				alListenerfv(AL_ORIENTATION, orientation);
			}

			static void Construct(PE::GameContext& context, PE::MemoryArena arena, const char* wbFilename, bool isActive = true)
			{
				Handle h("SOUND_MANAGER", sizeof(SoundManager));
				SoundManager* pSoundManager = new(h) SoundManager(context, arena, h, wbFilename, isActive);
				pSoundManager->addDefaultComponents();

				SetInstance(h);
				s_isActive = isActive;

				// --- OpenAL Init ---
				pSoundManager->m_pDevice = alcOpenDevice(nullptr); // Use default device
				if (!pSoundManager->m_pDevice)
				{
					PEINFO("SoundManager: Failed to open OpenAL device\n");
					return;
				}

				pSoundManager->m_pContext = alcCreateContext(pSoundManager->m_pDevice, nullptr);
				if (!pSoundManager->m_pContext)
				{
					PEINFO("SoundManager: Failed to create OpenAL context\n");
					alcCloseDevice(pSoundManager->m_pDevice);
					return;
				}

				alcMakeContextCurrent(pSoundManager->m_pContext);
				alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
				PEINFO("SoundManager: OpenAL initialized successfully\n");
			}



			static void SetInstance(Handle h) { s_hInstance = h; }

			static SoundManager* Instance() { return s_hInstance.getObject<SoundManager>(); }
			static Handle InstanceHandle() { return s_hInstance; }
			static Handle s_hInstance;
			static bool s_isActive;
			ALCdevice* m_pDevice = nullptr;
			ALCcontext* m_pContext = nullptr;

#if APIABSTRACTION_D3D9 || APIABSTRACTION_D3D11
			Handle m_hXACT;

#endif
		};
	}; // namespace Components
}; // namespace PE
#endif
