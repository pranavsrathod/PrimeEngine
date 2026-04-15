#ifndef __PYENGINE_2_0_BUTTONMESH_H__
#define __PYENGINE_2_0_BUTTONMESH_H__

#define NOMINMAX
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
#include <assert.h>

// Inter-Engine includes
#include "PrimeEngine/APIAbstraction/Effect/Effect.h"

// Sibling/Children includes
#include "Mesh.h"

namespace PE {
	namespace Components {

		struct ButtonMesh : public Mesh
		{
			PE_DECLARE_CLASS(ButtonMesh);

			// Constructor -------------------------------------------------------------
			ButtonMesh(PE::GameContext& context, PE::MemoryArena arena, Handle hMyself) : Mesh(context, arena, hMyself)
			{
				m_loaded = false;
			}

			virtual ~ButtonMesh() {}

			virtual void addDefaultComponents();

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_GATHER_DRAWCALLS);
			virtual void do_GATHER_DRAWCALLS(Events::Event* pEvt);

			void loadFromSource_needsRC(const char* filename, const char* techName, int& threadOwnershipMask, float width, float height);

			PrimitiveTypes::Float32 m_width, m_height;
			PrimitiveTypes::Bool m_loaded;
			Handle m_meshCPU;
		};

	}; // namespace Components
}; // namespace PE
#endif
