
#include "UIElement.h"
#include "../Lua/LuaEnvironment.h"
#include "TextMesh.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "MeshManager.h"
#include "MeshInstance.h"

namespace PE {
	namespace Components {

		PE_IMPLEMENT_CLASS1(UIElement, SceneNode);


		// Constructor -------------------------------------------------------------
		UIElement::UIElement(PE::GameContext& context, PE::MemoryArena arena, Handle hMyself)
			: SceneNode(context, arena, hMyself)
		{
			m_cachedAspectRatio = 1.0f;
			m_scale = 1.0f;
			if (IRenderer* pS = context.getGPUScreen())
				m_cachedAspectRatio = float(pS->getWidth()) / float(pS->getHeight());
		}

		void UIElement::addDefaultComponents()
		{
			SceneNode::addDefaultComponents();


			// event handlers
			/*PE_REGISTER_EVENT_HANDLER(Events::Event_PRE_GATHER_DRAWCALLS, UIElement::do_PRE_GATHER_DRAWCALLS);*/
		}

		void UIElement::setSelfAndMeshAssetEnabled(bool enabled)
		{
			setEnabled(enabled);

			for (int i = 0; i < m_uiSceneNodes.m_size; i++) {
				Handle currentSN = m_uiSceneNodes[i];
				if (currentSN.isValid()) {
					Component* curComponent = currentSN.getObject<Component>();
					curComponent->setEnabled(enabled);
				}
			}

			/*if (m_hMyTextMesh.isValid())
			{
				m_hMyTextMesh.getObject<Component>()->setEnabled(enabled);
			}*/
		}

		//void UIElement::do_PRE_GATHER_DRAWCALLS(Events::Event* pEvt)
		//{
		//	


		//}

	}; // namespace Components
}; // namespace PE
