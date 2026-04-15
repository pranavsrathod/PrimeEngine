#ifndef __PYENGINE_2_0_LINESCENENODE_H__
#define __PYENGINE_2_0_LINESCENENODE_H__

// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
#include <assert.h>

// Inter-Engine includes
#include "PrimeEngine/MemoryManagement/Handle.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "../Events/Component.h"
#include "../Utils/Array/Array.h"
#include "SceneNode.h"

namespace PE {
	namespace Components {

		struct LineSceneNode : public SceneNode
		{
			PE_DECLARE_CLASS(LineSceneNode);

			// Constructor -------------------------------------------------------------
			LineSceneNode(PE::GameContext& context, PE::MemoryArena arena, Handle hMyself);

			virtual ~LineSceneNode() {}

			void setSelfAndMeshAssetEnabled(bool enabled);

			// Component ------------------------------------------------------------
			virtual void addDefaultComponents();

			// Individual events -------------------------------------------------------
			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PRE_GATHER_DRAWCALLS);
			virtual void do_PRE_GATHER_DRAWCALLS(Events::Event* pEvt);

			enum DrawType
			{
				InWorld,
				Overlay2D
			};

			void loadFromData_needsRC(float* lineData, int numPoints, DrawType drawType, int& threadOwnershipMask);

			DrawType m_drawType;
			float m_scale;
			Handle m_hMyLineMesh;
			Handle m_hMyLineMeshInstance;
			float m_cachedAspectRatio;

		}; // class LineSceneNode

	}; // namespace Components
}; // namespace PE

#endif