#ifndef __PYENGINE_2_0_BUTTONSCENENODE_H__
#define __PYENGINE_2_0_BUTTONSCENENODE_H__

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

//#define USE_DRAW_COMPONENT

namespace PE {
	namespace Components {
		struct ButtonSceneNode : public SceneNode
		{
			PE_DECLARE_CLASS(ButtonSceneNode);

			// Constructor -------------------------------------------------------------
			ButtonSceneNode(PE::GameContext& context, PE::MemoryArena arena, Handle hMyself);

			virtual ~ButtonSceneNode() {}

			void setSelfAndMeshAssetEnabled(bool enabled);

			// Component ------------------------------------------------------------

			virtual void addDefaultComponents();

			// Individual events -------------------------------------------------------

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PRE_GATHER_DRAWCALLS);
			virtual void do_PRE_GATHER_DRAWCALLS(Events::Event* pEvt);

			enum DrawType
			{
				InWorld,
				InWorldFacingCamera,
				Overlay2D,
				Overlay2D_3DPos
			};
			void loadFromSource_needsRC(const char* filename, DrawType drawType, int& threadOwnershipMask, float width, float height);

			DrawType m_drawType;
			float m_scale;
			Handle m_hMyButtonMesh;
			Handle m_hMyButtonMeshInstance;
			float m_cachedAspectRatio;

			bool m_canBeRecreated;

		}; // class ButtonSceneNode

	}; // namespace Components
}; // namespace PE
#endif
