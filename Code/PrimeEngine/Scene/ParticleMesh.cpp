#define NOMINMAX
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes

// Inter-Engine includes
#include "PrimeEngine/FileSystem/FileReader.h"
#include "PrimeEngine/APIAbstraction/GPUMaterial/GPUMaterialSet.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/APIAbstraction/Texture/Texture.h"
#include "PrimeEngine/APIAbstraction/Effect/EffectManager.h"
#include "PrimeEngine/APIAbstraction/GPUBuffers/VertexBufferGPUManager.h"
#include "Light.h"
#include "PrimeEngine/GameObjectModel/Camera.h"

// Sibling/Children includes
#include "ParticleMesh.h"
#include "SceneNode.h"
#include "CameraManager.h"
#include "../Lua/LuaEnvironment.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include <PrimeEngine/Geometry/ParticleSystemCPU/ParticleSystemCPU.h>
#include <PrimeEngine/Sound/SoundManager.h>
namespace PE {
	namespace Components {

		PE_IMPLEMENT_CLASS1(ParticleMesh, Mesh);



		void ParticleMesh::addDefaultComponents()
		{
			//add this handler before Mesh's handlers so we can intercept draw and modify transform
			PE_REGISTER_EVENT_HANDLER(Events::Event_GATHER_DRAWCALLS, ParticleMesh::do_GATHER_DRAWCALLS);
			PE_REGISTER_EVENT_HANDLER(Events::Event_UPDATE, ParticleMesh::do_UPDATE);
			Mesh::addDefaultComponents();
		}

		void ParticleMesh::createParticleMesh(Particle pTemplate, ParticleSystemType systemType) {
			if (!h_ParticleCPU.isValid()) {
				h_ParticleCPU = Handle("PARTICLESYSTEMCPU", sizeof(ParticleSystemCPU));
				new (h_ParticleCPU) ParticleSystemCPU(*m_pContext, m_arena, pTemplate);

			}
            m_particleTemplate = pTemplate;
			ParticleSystemCPU& pCPU = *h_ParticleCPU.getObject<ParticleSystemCPU>();
            pCPU.m_systemType = systemType;
			Handle h_parent = getFirstParentByType<SceneNode>();
			if (h_parent.isValid()) {
				SceneNode* pSN = h_parent.getObject<SceneNode>();
				/*Matrix4x4 particleBase = pSN->m_worldTransform * m_offset;*/
                Matrix4x4 particleBase;
                particleBase.setPos(Vector3(0.0f, 0.0f, 5.0f)); // example position
                //m_offset.setPos(Vector3(0, 2, 0));
				m_hasTexture = strlen(pTemplate.m_texture) > 0;
                m_hasColor = (pTemplate.color.m_x != 0.0f || pTemplate.color.m_y != 0.0f || pTemplate.color.m_z != 0.0f);
				pCPU.create(particleBase);
			}
			else {
				PEINFO("PARTICLE MESH : createParticleMesh - no parent SceneNode");
			}

		}

        void ParticleMesh::loadParticle_needsRC(int& threadOwnershipMask)
        {
            MeshCPU* mcpu;
            if (!h_meshCPU.isValid())
            {
                h_meshCPU = Handle("MeshCPU SpriteMesh", sizeof(MeshCPU));
                mcpu = new(h_meshCPU) MeshCPU(*m_pContext, m_arena);
            }
            else
            {
                mcpu = h_meshCPU.getObject<MeshCPU>();
            }

            if (!m_loaded)
            {
                mcpu->createEmptyMesh();
                // mcpu->createBillboardMeshWithColorTexture("cobble2_color.dds", "Default", 32, 32, SamplerState_NoMips_NoMinTex);
            }

            mcpu->m_manualBufferManagement = true;

            ParticleSystemCPU* pSysCPU = h_ParticleCPU.getObject<ParticleSystemCPU>();
            ParticleBufferCPU<ParticleCPU>* pPb = pSysCPU->m_hParticleBufferCPU.getObject<ParticleBufferCPU<ParticleCPU>>();
            PrimitiveTypes::Int16 particleCount = pPb->m_values.m_size;

            PositionBufferCPU* pVB = mcpu->m_hPositionBufferCPU.getObject<PositionBufferCPU>();
            IndexBufferCPU* pIB = mcpu->m_hIndexBufferCPU.getObject<IndexBufferCPU>();
            ColorBufferCPU* pCB;
            TexCoordBufferCPU* pTCB;
            NormalBufferCPU* pNB;
            MaterialSetCPU* msCPU;

            pVB->m_values.reset(particleCount * 4 * 3); // 4 verts * (x,y,z)
            pIB->m_values.reset(particleCount * 6);     // 2 tris

            pIB->m_indexRanges[0].m_start = 0;
            pIB->m_indexRanges[0].m_end = particleCount * 6 - 1;
            pIB->m_indexRanges[0].m_minVertIndex = 0;
            pIB->m_indexRanges[0].m_maxVertIndex = particleCount * 4 - 1;

            pIB->m_minVertexIndex = pIB->m_indexRanges[0].m_minVertIndex;
            pIB->m_maxVertexIndex = pIB->m_indexRanges[0].m_maxVertIndex;

            if (!mcpu->m_hTexCoordBufferCPU.isValid()) {
                mcpu->m_hTexCoordBufferCPU = Handle("TEXCOORD_BUFFER_CPU", sizeof(TexCoordBufferCPU));
                new(mcpu->m_hTexCoordBufferCPU) TexCoordBufferCPU(*m_pContext, m_arena);
            }
            pTCB = mcpu->m_hTexCoordBufferCPU.getObject<TexCoordBufferCPU>();
            pTCB->m_values.reset(particleCount * 4 * 2);

            if (!mcpu->m_hNormalBufferCPU.isValid()) {
                mcpu->m_hNormalBufferCPU = Handle("NORMAL_BUFFER_CPU", sizeof(NormalBufferCPU));
                new(mcpu->m_hNormalBufferCPU) NormalBufferCPU(*m_pContext, m_arena);
            }
            pNB = mcpu->m_hNormalBufferCPU.getObject<NormalBufferCPU>();
            pNB->m_values.reset(particleCount * 4 * 3);

            if (m_hasColor)
            {
                pCB = mcpu->m_hColorBufferCPU.getObject<ColorBufferCPU>();
                pCB->m_values.reset(particleCount * 4 * 3);
            }

            for (int i = 0; i < particleCount; i++)
            {
                Matrix4x4 topLeftTransform = pPb->m_values[i].m_base;
                Matrix4x4 topRightTransform = pPb->m_values[i].m_base;
                Matrix4x4 bottomLeftTransform = pPb->m_values[i].m_base;
                Matrix4x4 bottomRightTransform = pPb->m_values[i].m_base;

                Vector2 curSize = pPb->m_values[i].m_size;

                topLeftTransform.moveLeft(curSize.m_x / 2.f);
                topLeftTransform.moveUp(curSize.m_y / 2.f);

                topRightTransform.moveRight(curSize.m_x / 2.f);
                topRightTransform.moveUp(curSize.m_y / 2.f);

                bottomLeftTransform.moveLeft(curSize.m_x / 2.f);
                bottomLeftTransform.moveDown(curSize.m_y / 2.f);

                bottomRightTransform.moveRight(curSize.m_x / 2.f);
                bottomRightTransform.moveDown(curSize.m_y / 2.f);

                pVB->m_values.add(topLeftTransform.getPos().m_x, topLeftTransform.getPos().m_y, topLeftTransform.getPos().m_z);     // top left
                pVB->m_values.add(topRightTransform.getPos().m_x, topRightTransform.getPos().m_y, topRightTransform.getPos().m_z);   // top right
                pVB->m_values.add(bottomRightTransform.getPos().m_x, bottomRightTransform.getPos().m_y, bottomRightTransform.getPos().m_z); // bottom right
                pVB->m_values.add(bottomLeftTransform.getPos().m_x, bottomLeftTransform.getPos().m_y, bottomLeftTransform.getPos().m_z);     // bottom left

                pIB->m_values.add(i * 4 + 0, i * 4 + 1, i * 4 + 2);
                pIB->m_values.add(i * 4 + 2, i * 4 + 3, i * 4 + 0);

                if (m_hasColor)
                {
                    pCB->m_values.add(0, 1.f, 0);
                    pCB->m_values.add(0, 1.f, 0);
                    pCB->m_values.add(0, 1.f, 0);
                    pCB->m_values.add(0, 1.f, 0);
                }

                if (m_hasTexture)
                {
                    pTCB->m_values.add(0, 0); // top left
                    pTCB->m_values.add(1, 0); // top right
                    pTCB->m_values.add(1, 1);
                    pTCB->m_values.add(0, 1);

                    pNB->m_values.add(0, 0, 0);
                    pNB->m_values.add(0, 0, 0);
                    pNB->m_values.add(0, 0, 0);
                    pNB->m_values.add(0, 0, 0);
                }
            }

            // first time creating GPU mesh
            if (!m_loaded)
            {
                if (m_hasTexture)
                {
                    msCPU = mcpu->m_hMaterialSetCPU.getObject<MaterialSetCPU>();
                    memcpy(msCPU, pSysCPU->m_hMaterialSetCPU.getObject<MaterialSetCPU>(), sizeof(msCPU));
                }

                loadFromMeshCPU_needsRC(*mcpu, threadOwnershipMask);

                const char* techName = "StdMesh_Diffuse_Tech";
                if (techName && !m_hasColor)
                {
                    Handle hEffect = EffectManager::Instance()->getEffectHandle(techName);
                    for (unsigned int imat = 0; imat < m_effects.m_size; imat++)
                    {
                        if (m_effects[imat].m_size)
                            m_effects[imat][0] = hEffect;
                    }
                }

                m_loaded = true;
            }
            else
            {
                updateGeoFromMeshCPU_needsRC(*mcpu, threadOwnershipMask);
            }
        }

		void ParticleMesh::do_GATHER_DRAWCALLS(Events::Event* pEvt)
		{

		}
        void ParticleMesh::do_UPDATE(Events::Event* pEvt)
        {
            Events::Event_UPDATE* pUpdateEvt = static_cast<Events::Event_UPDATE*>(pEvt);
            PEINFO("Particle Mesh : do_UPDATE");
            ParticleSystemCPU* pSys = h_ParticleCPU.getObject<ParticleSystemCPU>();
            pSys->updateParticleBuffer(pUpdateEvt->m_frameTime);
            PE::Components::SoundManager::Instance()->updateListenerFromCamera();
        }

	}; // namespace Components
}; // namespace PE
