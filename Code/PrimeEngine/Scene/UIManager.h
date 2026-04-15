#ifndef __PYENGINE_2_0_UI_MANAGER__
#define __PYENGINE_2_0_UI_MANAGER__

#define NOMINMAX
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes

// Inter-Engine includes
#include "../Utils/Array/Array.h"
#include "../Utils/PEString.h"

// Sibling/Children includes
#include "../GameObjectModel/Camera.h"
#include "RootSceneNode.h"
#include "UI_Events.h"
namespace PE {
	namespace Components {


		struct UIManager : public Component
		{
			PE_DECLARE_CLASS(UIManager);



			UIManager(PE::GameContext& context, PE::MemoryArena arena, PE::Handle hMyself);
			static void Construct(PE::GameContext& context, PE::MemoryArena arena);
			virtual ~UIManager() {}

			void setUIActive(Array<PrimitiveTypes::Int32> UIIndices, bool isActive);
			//void createButton(Event_CREATE_BUTTON* pRealEvent);
			void createButton(Events::Event_CREATE_BUTTON* pRealEvent);
			void createButtonGroup();

			static void SetInstance(Handle h);

			static UIManager* Instance();

			static Handle s_hInstance;

			Array<Handle> m_UIList;

			PrimitiveTypes::Int32 m_maxWidth;
			PrimitiveTypes::Int32 m_maxHeight;
			float m_cachedAspectRatio;

		private:
			static Handle s_myHandle;
			static const int NUM_MaxUI = 64;
			int m_last;
			GameContext* m_context;


		};

	}; // namespace Components
}; // namespace PE
#endif
