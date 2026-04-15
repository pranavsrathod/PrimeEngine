#ifndef __PYENGINE_2_0_PARTICLEMESH_H__
#define __PYENGINE_2_0_PARTICLEMESH_H__

#define NOMINMAX
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
#include <assert.h>
#include <AL/alc.h>

// Inter-Engine includes
#include "PrimeEngine/APIAbstraction/Effect/Effect.h"

// Sibling/Children includes
#include "Mesh.h"

namespace PE {
	namespace Components {
		struct Particle
		{
			const char* m_texture;
			Vector3 color;

			PrimitiveTypes::Int16 m_rate;
			PrimitiveTypes::Float32 m_speed;
			PrimitiveTypes::Float32 m_duration;
			PrimitiveTypes::Bool isLooping;
			Vector2 m_size;
			//Shape m_shape;
		};

		struct ParticleMesh : public Mesh
		{
			PE_DECLARE_CLASS(ParticleMesh);

			// Constructor -------------------------------------------------------------
			ParticleMesh(PE::GameContext& context, PE::MemoryArena arena, Handle hMyself) : Mesh(context, arena, hMyself)
			{
				m_offset = Matrix4x4();
				m_offset.setPos(Vector3(1, 1, 0));
				m_arena = arena;
				m_pContext = &context;
			}

			// In ParticleMesh.h or ParticleSystemCPU.h (better)
			enum ParticleSystemType
			{
				PST_SPHERE,
				PST_BURST,
				PST_FOUNTAIN,
				PST_SPIRAL,
				PST_FIRE
				// (Later you could add PST_Spiral, PST_Snow, PST_Shockwave, etc.)
			};

			PE::MemoryArena m_arena;
			PE::GameContext* m_pContext;

			virtual ~ParticleMesh() {}

			virtual void addDefaultComponents();

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_GATHER_DRAWCALLS);
			virtual void do_GATHER_DRAWCALLS(Events::Event* pEvt);

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
			virtual void do_UPDATE(Events::Event* pEvt);

			/*void createParticleMesh(Particle pTemplate);*/
			void createParticleMesh(Particle pTemplate, ParticleSystemType systemType);

			void loadParticle_needsRC(int& threadOwnnershipMask);

			Handle h_ParticleCPU, h_meshCPU;

			Matrix4x4 m_offset;
			Particle m_particleTemplate; // store the particle template passed to createParticleMesh

			PrimitiveTypes::Bool m_loaded, m_hasTexture, m_hasColor;
			ParticleSystemType m_systemType;

		};

	}; // namespace Components
}; // namespace PE
#endif
