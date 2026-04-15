#define NOMINMAX
#include "UIManager.h"
#include "../Lua/LuaEnvironment.h"
#include "UIButton.h"
using namespace PE::Events;
namespace PE {
	namespace Components {

		PE_IMPLEMENT_CLASS1(UIManager, Component);

		Handle UIManager::s_hInstance;

		UIManager::UIManager(PE::GameContext& context, PE::MemoryArena arena, Handle hMyself) : Component(context, arena, hMyself)
		{
			m_UIList.reset(NUM_MaxUI);
			m_cachedAspectRatio = 1.0f;
			m_context = &context;
			if (IRenderer* pS = context.getGPUScreen()) {
				m_maxWidth = pS->getWidth();
				m_maxHeight = pS->getHeight();
				m_cachedAspectRatio = float(pS->getWidth()) / float(pS->getHeight());
			}
		}

		void UIManager::Construct(PE::GameContext& context, PE::MemoryArena arena)
		{
			Handle handle("UIManager", sizeof(UIManager));
			UIManager* pUIManager = new(handle) UIManager(context, arena, handle);
			pUIManager->addDefaultComponents();
			SetInstance(handle);
			RootSceneNode::Instance()->addComponent(handle);

		}

		void UIManager::SetInstance(Handle h) { s_hInstance = h; }
		UIManager* UIManager::Instance() { return s_hInstance.getObject<UIManager>(); }

		void UIManager::setUIActive(Array<PrimitiveTypes::Int32> UIIndices, bool isActive) {

		}

		void UIManager::createButton(Event_CREATE_BUTTON* pRealEvent)
		{
			if (m_UIList.m_size >= NUM_MaxUI)
			{
				return;
			}

			m_pContext->getGPUScreen()->AcquireRenderContextOwnership(pRealEvent->m_threadOwnershipMask);
			Handle h = PE::Handle("UI_BUTTON", sizeof(UIButton));
			UIButton* pButton = new(h) UIButton(*m_pContext, m_arena, h, pRealEvent);
			pButton->addDefaultComponents();
			addComponent(h);

			Vector3 pos;
			if (pButton->m_drawType == UIElement::Overlay2D)
			{
				pos.m_x = -1.0f + 2.0f * (pRealEvent->m_pos.m_x / UIManager::Instance()->m_maxWidth);
				pos.m_y = -1.0f + 2.0f * (1.0f - (pRealEvent->m_pos.m_y / UIManager::Instance()->m_maxHeight));
			}
			else {
				pos = pRealEvent->m_pos;
			}

			//pButton->setPos(pos);
			pButton->createSpriteSceneNode(pButton->m_buttonImageSource, ButtonSceneNode::Overlay2D, m_context->m_gameThreadThreadOwnershipMask, pButton->m_width, pButton->m_height);
			pButton->createTextSceneNode(pButton->m_buttonText, TextSceneNode::Overlay2D, m_context->m_gameThreadThreadOwnershipMask);
			pButton->setSelfAndMeshAssetEnabled(pRealEvent->isActive);
			pButton->currentIndex = m_UIList.m_size;
			m_UIList.add(h);
		}




	}; // namespace Components
}; // namespace PE
