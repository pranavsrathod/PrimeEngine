#include "CameraSceneNode.h"
#include "../Lua/LuaEnvironment.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "PrimeEngine/Scene/RootSceneNode.h"

#define Z_ONLY_CAM_BIAS 0.0f
namespace PE {
    namespace Components {

        PE_IMPLEMENT_CLASS1(CameraSceneNode, SceneNode);

        CameraSceneNode::CameraSceneNode(PE::GameContext& context, PE::MemoryArena arena, Handle hMyself) : SceneNode(context, arena, hMyself)
        {
            m_near = 0.05f;
            m_far = 2000.0f;
        }
        void CameraSceneNode::addDefaultComponents()
        {
            Component::addDefaultComponents();
            PE_REGISTER_EVENT_HANDLER(Events::Event_CALCULATE_TRANSFORMATIONS, CameraSceneNode::do_CALCULATE_TRANSFORMATIONS);
        }

        void CameraSceneNode::do_CALCULATE_TRANSFORMATIONS(Events::Event* pEvt)
        {
            Handle hParentSN = getFirstParentByType<SceneNode>();
            if (hParentSN.isValid())
            {
                Matrix4x4 parentTransform = hParentSN.getObject<PE::Components::SceneNode>()->m_worldTransform;
                m_worldTransform = parentTransform * m_base;
            }

            Matrix4x4& mref_worldTransform = m_worldTransform;

            Vector3 pos = Vector3(mref_worldTransform.m[0][3], mref_worldTransform.m[1][3], mref_worldTransform.m[2][3]);
            Vector3 n = Vector3(mref_worldTransform.m[0][2], mref_worldTransform.m[1][2], mref_worldTransform.m[2][2]);
            Vector3 target = pos + n;
            Vector3 up = Vector3(mref_worldTransform.m[0][1], mref_worldTransform.m[1][1], mref_worldTransform.m[2][1]);

            m_worldToViewTransform = CameraOps::CreateViewMatrix(pos, target, up);

            m_worldTransform2 = mref_worldTransform;

            m_worldTransform2.moveForward(Z_ONLY_CAM_BIAS);

            Vector3 pos2 = Vector3(m_worldTransform2.m[0][3], m_worldTransform2.m[1][3], m_worldTransform2.m[2][3]);
            Vector3 n2 = Vector3(m_worldTransform2.m[0][2], m_worldTransform2.m[1][2], m_worldTransform2.m[2][2]);
            Vector3 target2 = pos2 + n2;
            Vector3 up2 = Vector3(m_worldTransform2.m[0][1], m_worldTransform2.m[1][1], m_worldTransform2.m[2][1]);

            m_worldToViewTransform2 = CameraOps::CreateViewMatrix(pos2, target2, up2);

            PrimitiveTypes::Float32 aspect = (PrimitiveTypes::Float32)(m_pContext->getGPUScreen()->getWidth()) / (PrimitiveTypes::Float32)(m_pContext->getGPUScreen()->getHeight());

            PrimitiveTypes::Float32 verticalFov = 0.33f * PrimitiveTypes::Constants::c_Pi_F32;
            if (aspect < 1.0f)
            {
                //ios portrait view
                static PrimitiveTypes::Float32 factor = 0.5f;
                verticalFov *= factor;
            }

            m_viewToProjectedTransform = CameraOps::CreateProjectionMatrix(verticalFov, aspect, m_near, m_far);

            Matrix4x4 combinedMatrix = m_viewToProjectedTransform * m_worldToViewTransform;

            frustumValues(combinedMatrix, verticalFov, aspect);// for different camera spec

            SceneNode::do_CALCULATE_TRANSFORMATIONS(pEvt);

        }

        void CameraSceneNode::frustumValues(Matrix4x4& combinedMatrix, PrimitiveTypes::Float32 verticalFov, PrimitiveTypes::Float32 aspect)
        {
            PrimitiveTypes::Float32 tanHalfFov = tanf(verticalFov / 4.0f);
            PrimitiveTypes::Float32 halfHeightNear = m_near * tanHalfFov;
            PrimitiveTypes::Float32 halfWidthNear = halfHeightNear * aspect;
            PrimitiveTypes::Float32 halfHeightFar = m_far * tanHalfFov;
            PrimitiveTypes::Float32 halfWidthFar = halfHeightFar * aspect;

            Vector3 camPos = Vector3(m_worldTransform.m[0][3], m_worldTransform.m[1][3], m_worldTransform.m[2][3]);
            Vector3 camDir = Vector3(m_worldTransform.m[0][2], m_worldTransform.m[1][2], m_worldTransform.m[2][2]);
            Vector3 camUp = Vector3(m_worldTransform.m[0][1], m_worldTransform.m[1][1], m_worldTransform.m[2][1]);
            Vector3 camRight = camDir.crossProduct(camUp);

            Vector3 nearCenter = camPos + camDir * m_near;
            Vector3 farCenter = camPos + camDir * m_far;

            frustum.corners[0] = nearCenter + halfHeightNear * camUp - halfWidthNear * camRight;
            frustum.corners[1] = nearCenter + halfHeightNear * camUp + halfWidthNear * camRight;
            frustum.corners[2] = nearCenter - halfHeightNear * camUp - halfWidthNear * camRight;
            frustum.corners[3] = nearCenter - halfHeightNear * camUp + halfWidthNear * camRight;
            frustum.corners[4] = farCenter + halfHeightFar * camUp - halfWidthFar * camRight;
            frustum.corners[5] = farCenter + halfHeightFar * camUp + halfWidthFar * camRight;
            frustum.corners[6] = farCenter - halfHeightFar * camUp - halfWidthFar * camRight;
            frustum.corners[7] = farCenter - halfHeightFar * camUp + halfWidthFar * camRight;

            frustum.planes[0] = Plane(frustum.corners[3], frustum.corners[2], frustum.corners[0]);
            frustum.planes[1] = Plane(frustum.corners[4], frustum.corners[6], frustum.corners[7]);
            frustum.planes[2] = Plane(frustum.corners[6], frustum.corners[2], frustum.corners[3]);
            frustum.planes[3] = Plane(frustum.corners[1], frustum.corners[0], frustum.corners[4]);
            frustum.planes[4] = Plane(frustum.corners[0], frustum.corners[2], frustum.corners[6]);
            frustum.planes[5] = Plane(frustum.corners[7], frustum.corners[3], frustum.corners[1]);

            for (int i = 0; i < 6; ++i)
            {
                frustum.planes[i].normalize();
            }
            //M1 3rd wind source: camera pos
            RootSceneNode* pRoot = RootSceneNode::Instance();
            //pRoot->pos2 = camPos;
        }
    }; // namespace Components
}; // namespace PE
