
#include "ButtonSceneNode.h"
#include "../Lua/LuaEnvironment.h"
#include "ButtonMesh.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "MeshManager.h"
#include "MeshInstance.h"

namespace PE {
	namespace Components {

		PE_IMPLEMENT_CLASS1(ButtonSceneNode, SceneNode);


		// Constructor -------------------------------------------------------------
		ButtonSceneNode::ButtonSceneNode(PE::GameContext& context, PE::MemoryArena arena, Handle hMyself)
			: SceneNode(context, arena, hMyself)
		{
			m_cachedAspectRatio = 1.0f;
			m_scale = 1.0f;
			if (IRenderer* pS = context.getGPUScreen())
				m_cachedAspectRatio = float(pS->getWidth()) / float(pS->getHeight());
		}

		void ButtonSceneNode::addDefaultComponents()
		{
			SceneNode::addDefaultComponents();


			// event handlers
			PE_REGISTER_EVENT_HANDLER(Events::Event_PRE_GATHER_DRAWCALLS, ButtonSceneNode::do_PRE_GATHER_DRAWCALLS);
		}

		void ButtonSceneNode::setSelfAndMeshAssetEnabled(bool enabled)
		{
			setEnabled(enabled);

			if (m_hMyButtonMesh.isValid())
			{
				m_hMyButtonMesh.getObject<Component>()->setEnabled(enabled);
			}
		}


		void ButtonSceneNode::loadFromSource_needsRC(const char* filename, DrawType drawType, int& threadOwnershipMask, float width, float height)
		{
			m_drawType = drawType;

			ButtonMesh* pButtonMesh = NULL;
			if (m_hMyButtonMesh.isValid())
			{
				pButtonMesh = m_hMyButtonMesh.getObject<ButtonMesh>();
			}
			else
			{
				m_hMyButtonMesh = PE::Handle("BUTTONMESH", sizeof(ButtonMesh));
				pButtonMesh = new(m_hMyButtonMesh) ButtonMesh(*m_pContext, m_arena, m_hMyButtonMesh);
				pButtonMesh->addDefaultComponents();

				m_pContext->getMeshManager()->registerAsset(m_hMyButtonMesh);

				m_hMyButtonMeshInstance = PE::Handle("MeshInstance", sizeof(MeshInstance));
				MeshInstance* pInstance = new(m_hMyButtonMeshInstance) MeshInstance(*m_pContext, m_arena, m_hMyButtonMeshInstance);
				pInstance->addDefaultComponents();
				pInstance->initFromRegisteredAsset(m_hMyButtonMesh);


				addComponent(m_hMyButtonMeshInstance);
			}

			PE::IRenderer::checkForErrors("");

			const char* tech = 0;
			if (drawType == Overlay2D_3DPos || drawType == Overlay2D)
				tech = "StdMesh_2D_Diffuse_A_RGBIntensity_Tech";
			if (drawType == InWorldFacingCamera)
				tech = "StdMesh_Diffuse_Tech";

			pButtonMesh->loadFromSource_needsRC(filename, tech, threadOwnershipMask, width, height);
		}

		void ButtonSceneNode::do_PRE_GATHER_DRAWCALLS(Events::Event* pEvt)
		{
			Events::Event_PRE_GATHER_DRAWCALLS* pDrawEvent = NULL;
			pDrawEvent = (Events::Event_PRE_GATHER_DRAWCALLS*)(pEvt);

			Matrix4x4 projectionViewWorldMatrix = pDrawEvent->m_projectionViewTransform;
			Matrix4x4 worldMatrix;

			if (!m_hMyButtonMeshInstance.isValid())
				return;

			ButtonMesh* pButtonMesh = m_hMyButtonMesh.getObject<ButtonMesh>();

			if (m_drawType == InWorldFacingCamera)
			{
				m_worldTransform.turnTo(pDrawEvent->m_eyePos);
			}

			float numCharsInFullLine = 100.0f;
			numCharsInFullLine *= .5; // need to divide by 2.0 since screen goes from -1 to 1, not 0..1

			if (m_drawType == Overlay2D)
			{
				worldMatrix = m_worldTransform;

				float factor = 1.0f / (numCharsInFullLine)*m_scale;

				Matrix4x4 scale;
				scale.importScale(factor, factor * m_cachedAspectRatio, 1.f);
				m_worldTransform = worldMatrix * scale;
			}


		}

	}; // namespace Components
}; // namespace PE
