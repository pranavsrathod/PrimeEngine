#pragma once
#ifndef __PYENGINE_2_0_UIELEMENT_H__
#define __PYENGINE_2_0_UIELEMENT_H__

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
		struct UIElement : public SceneNode
		{
			PE_DECLARE_CLASS(UIElement);

			// Constructor -------------------------------------------------------------
			UIElement(PE::GameContext& context, PE::MemoryArena arena, Handle hMyself);

			virtual ~UIElement() {}

			void setSelfAndMeshAssetEnabled(bool enabled);

			// Component ------------------------------------------------------------
			void setPos(Vector3 pos);
			virtual void addDefaultComponents();

			// Individual events -------------------------------------------------------

			/*PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PRE_GATHER_DRAWCALLS);
			virtual void do_PRE_GATHER_DRAWCALLS(Events::Event* pEvt);*/

			enum DrawType
			{
				InWorld,
				InWorldFacingCamera,
				Overlay2D,
				Overlay2D_3DPos
			};

			DrawType m_drawType = Overlay2D;
			float m_scale = 1.0;

			Vector3 m_pos;
			Array<Handle> m_uiSceneNodes;

			PrimitiveTypes::Int32 currentIndex;

			bool isActive;
			float m_cachedAspectRatio;
			/*Handle m_hMyTextMesh;
			Handle m_hMyTextMeshInstance;
			float m_cachedAspectRatio;

			bool m_canBeRecreated;*/

		}; // class UIElement

	}; // namespace Components
}; // namespace PE
#endif
