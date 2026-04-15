#include "UIButton.h"
#include "../Lua/LuaEnvironment.h"
#include "PrimeEngine/Scene/TextSceneNode.h"
#include "PrimeEngine/Scene/ButtonSceneNode.h"
#include "PrimeEngine/Scene/RootSceneNode.h"
#include "PrimeEngine/Scene/SceneNode.h"
#include "PrimeEngine/Scene/TextMesh.h"
#include "PrimeEngine/Scene/MeshManager.h"
#include "PrimeEngine/Scene/MeshInstance.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "PrimeEngine/Events/StandardGameEvents.h"
#include "DebugRenderer.h"
#include "UIManager.h"



namespace PE {
	namespace Components {

		PE_IMPLEMENT_CLASS1(UIButton, UIElement);

		UIButton::UIButton(PE::GameContext& context, PE::MemoryArena arena, Handle hMyself, PE::Events::Event_CREATE_BUTTON *pEvt)
			: UIElement(context, arena, hMyself)
		{
			m_buttonText = pEvt->m_text;
			//m_buttonImageSource = pEvt->m_image;
			m_name = pEvt->m_name;
			/*m_drawType = pEvt->m_drawType;*/
			m_width = pEvt->m_width;
			m_height = pEvt->m_height;
		}
		UIButton::UIButton(PE::GameContext& context, PE::MemoryArena arena, Handle hMyself,
			Vector2 pos, float width, float height, char* label)
			: UIElement(context, arena, hMyself)
		{
			m_position = pos;
			m_width = width;
			m_height = height;
			m_label = label;
			m_context = &context;
			/*m_isHovered = false;
			m_isPressed = false;*/
		}

		void UIButton::Construct(PE::GameContext& context, PE::MemoryArena arena,
			Vector2 pos, float width, float height, char* label)
		{
			
			PE::Handle h("UI_BUTTON", sizeof(UIButton));
			UIButton* pButton = new(h) UIButton(context, arena, h, pos, width, height, label);
			pButton->addDefaultComponents();
			RootSceneNode::Instance()->addComponent(h);
		}

		void UIButton::addDefaultComponents()
		{
			UIElement::addDefaultComponents();
			PE_REGISTER_EVENT_HANDLER(Events::Event_PRE_GATHER_DRAWCALLS, UIButton::do_PRE_GATHER_DRAWCALLS);
			PE_REGISTER_EVENT_HANDLER(Events::Event_LEFT_CLICK, UIButton::do_TEST_ONCLICK);

			int& mask = m_pContext->m_gameThreadThreadOwnershipMask;

			// Label (calls DebugRenderer)
			DebugRenderer::Instance()->createTextMesh(
				m_label,
				true, false, false, false,
				1, // TTL
				Vector3(m_position.m_x - 0.025, m_position.m_y - 0.02f, 0.0f),
				1.0f,  // scale
				mask
			);
			DebugRenderer::Instance()->createRectangleMesh(Vector3(m_position.m_x, m_position.m_y, 0.0f), m_width, m_height, 3.0f, 0.2f, m_pContext->m_gameThreadThreadOwnershipMask);
		}


		void UIButton::createTextSceneNode(const char* str, TextSceneNode::DrawType drawType, int& threadOwnershipMask)
		{
			// Basic placeholder to fix linker error
			Handle h("TEXT_SCENE_NODE", sizeof(TextSceneNode));
			TextSceneNode* pTextSN = new(h) TextSceneNode(*m_pContext, m_arena, h);
			pTextSN->addDefaultComponents();
			pTextSN->loadFromString_needsRC(str, drawType, threadOwnershipMask);
			pTextSN->m_base.setPos(Vector3(0, 0, 0));
			pTextSN->m_scale = 1.0f;
			addComponent(h);
			m_hMyTextSN = h;
		}

		void UIButton::createSpriteSceneNode(const char* bgFile, ButtonSceneNode::DrawType drawType, int& threadOwnershipMask, float width, float height)
		{
			m_hMySpriteSN = Handle("BUTTON_SCENE_NODE", sizeof(ButtonSceneNode));
			ButtonSceneNode* pSpriteSN = new(m_hMySpriteSN) ButtonSceneNode(*m_pContext, m_arena, m_hMySpriteSN);
			pSpriteSN->addDefaultComponents();

			m_pContext->getMeshManager()->registerAsset(m_hMySpriteSN);

			// Load image
			pSpriteSN->loadFromSource_needsRC(bgFile, drawType, threadOwnershipMask, width, height);

			addComponent(m_hMySpriteSN);
		}

		void UIButton::do_PRE_GATHER_DRAWCALLS(Events::Event* pEvt)
		{
			// TODO: Hover effects, etc. can go here if needed.
		}

		void UIButton::do_TEST_ONCLICK(Events::Event* pEvt)
		{

			Events::Event_LEFT_CLICK* pClick = static_cast<Events::Event_LEFT_CLICK*>(pEvt);

			float mouseX = pClick->m_x; // Pixel X
			float mouseY = pClick->m_y; // Pixel Y

			// Convert screen-space mouse position to normalized [0, 1] coordinates
			IRenderer* pRenderer = m_context->getGPUScreen();
			float screenWidth = (float)pRenderer->getWidth();
			float screenHeight = (float)pRenderer->getHeight();

			float normalizedX = mouseX / screenWidth;
			float normalizedY = mouseY / screenHeight;

			// Flip Y because screen origin is top-left
			//normalizedY = 1.0f - normalizedY;

			// Check if inside the button bounds
			PEINFO("Mouse Click Normalized: X = %f, Y = %f\n", normalizedX, normalizedY);

			/*if (normalizedY < 0.55) {

				PE::Handle h("Event_PAUSE_GAME", sizeof(Events::Event_PAUSE_GAME));
				Events::Event_PAUSE_GAME* pEvent = new(h) Events::Event_PAUSE_GAME;
				Events::EventQueueManager::Instance()->add(h, Events::QT_GENERAL);
			}
			else {

					PE::Handle h("Event_RESUEME_GAME", sizeof(Events::Event_RESUEME_GAME));
					Events::Event_RESUEME_GAME* pEvent = new(h) Events::Event_RESUEME_GAME;
					Events::EventQueueManager::Instance()->add(h, Events::QT_GENERAL);
			}*/

			
		}

		void UIButton::setSelfAndMeshAssetEnabled(bool isActive)
		{
			if (m_hMySpriteSN.isValid())
				m_hMySpriteSN.getObject<Component>()->setEnabled(isActive);
			if (m_hMyTextSN.isValid())
				m_hMyTextSN.getObject<Component>()->setEnabled(isActive);
		}

	} // namespace Components
} // namespace PE
