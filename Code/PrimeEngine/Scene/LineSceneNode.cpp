#include "LineSceneNode.h"
#include "LineMesh.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "MeshManager.h"
#include "MeshInstance.h"
#include "../Lua/LuaEnvironment.h"

namespace PE {
    namespace Components {

        PE_IMPLEMENT_CLASS1(LineSceneNode, SceneNode);

        // Constructor -------------------------------------------------------------
        LineSceneNode::LineSceneNode(PE::GameContext& context, PE::MemoryArena arena, Handle hMyself)
            : SceneNode(context, arena, hMyself)
        {
            m_cachedAspectRatio = 1.0f;
            m_scale = 1.0f;
            if (IRenderer* pS = context.getGPUScreen())
                m_cachedAspectRatio = float(pS->getWidth()) / float(pS->getHeight());
        }

        void LineSceneNode::addDefaultComponents()
        {
            SceneNode::addDefaultComponents();
            PE_REGISTER_EVENT_HANDLER(Events::Event_PRE_GATHER_DRAWCALLS, LineSceneNode::do_PRE_GATHER_DRAWCALLS);
        }

        void LineSceneNode::setSelfAndMeshAssetEnabled(bool enabled)
        {
            setEnabled(enabled);

            if (m_hMyLineMesh.isValid())
            {
                m_hMyLineMesh.getObject<Component>()->setEnabled(enabled);
            }
        }

        void LineSceneNode::loadFromData_needsRC(float* lineData, int numPoints, DrawType drawType, int& threadOwnershipMask)
        {
            m_drawType = drawType;

            LineMesh* pLineMesh = NULL;
            if (m_hMyLineMesh.isValid())
            {
                pLineMesh = m_hMyLineMesh.getObject<LineMesh>();
            }
            else
            {
                m_hMyLineMesh = PE::Handle("LINEMESH", sizeof(LineMesh));
                pLineMesh = new(m_hMyLineMesh) LineMesh(*m_pContext, m_arena, m_hMyLineMesh);
                pLineMesh->addDefaultComponents();

                m_pContext->getMeshManager()->registerAsset(m_hMyLineMesh);

                m_hMyLineMeshInstance = PE::Handle("MeshInstance", sizeof(MeshInstance));
                MeshInstance* pInstance = new(m_hMyLineMeshInstance) MeshInstance(*m_pContext, m_arena, m_hMyLineMeshInstance);
                pInstance->addDefaultComponents();
                pInstance->initFromRegisteredAsset(m_hMyLineMesh);

                addComponent(m_hMyLineMeshInstance);
            }

            PE::IRenderer::checkForErrors("");

            const char* tech = "StdMesh_2D_Diffuse_A_RGBIntensity_Tech";
            pLineMesh->loadFrom3DPoints_needsRC(lineData, numPoints, tech, threadOwnershipMask);
        }

        void LineSceneNode::do_PRE_GATHER_DRAWCALLS(Events::Event* pEvt)
        {
            Events::Event_PRE_GATHER_DRAWCALLS* pDrawEvent = (Events::Event_PRE_GATHER_DRAWCALLS*)(pEvt);

            if (!m_hMyLineMeshInstance.isValid())
                return;

            float referenceWidth = 100.0f * 0.5f;

            if (m_drawType == Overlay2D)
            {
                Matrix4x4 worldMatrix = m_worldTransform;

                float factor = 1.0f / referenceWidth * m_scale;
                Matrix4x4 scale;
                scale.importScale(factor, factor * m_cachedAspectRatio, 1.0f);

                m_worldTransform = worldMatrix * scale;
            }
        }

    }; // namespace Components
}; // namespace PE
