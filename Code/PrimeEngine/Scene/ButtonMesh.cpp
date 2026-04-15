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
#include "ButtonMesh.h"
#include "SceneNode.h"
#include "CameraManager.h"
#include "../Lua/LuaEnvironment.h"
#include "PrimeEngine/Events/StandardEvents.h"
namespace PE {
	namespace Components {

		PE_IMPLEMENT_CLASS1(ButtonMesh, Mesh);

		void ButtonMesh::addDefaultComponents()
		{
			//add this handler before Mesh's handlers so we can intercept draw and modify transform
			PE_REGISTER_EVENT_HANDLER(Events::Event_GATHER_DRAWCALLS, ButtonMesh::do_GATHER_DRAWCALLS);
			Mesh::addDefaultComponents();
		}

	void ButtonMesh::loadFromSource_needsRC(const char* filename, const char* techName, int& threadOwnershipMask, float width, float height)
	{
		m_width = width;
		m_height = height;

		if (!m_meshCPU.isValid())
		{
			m_meshCPU = Handle("MeshCPU ButtonMesh", sizeof(MeshCPU));
			new (m_meshCPU) MeshCPU(*m_pContext, m_arena);
		}

		MeshCPU& mcpu = *m_meshCPU.getObject<MeshCPU>();
		/*"cobble2_color.dds"*/
		if (!m_loaded)
			mcpu.createBillboardMeshWithColorTexture("cobble2_color.dds", "Default", 32, 32, SamplerState_NoMips_NoMinTexelLerp_NoMagTexelLerp_Clamp);

		mcpu.m_manualBufferManagement = true;

		PositionBufferCPU* pVB = mcpu.m_hPositionBufferCPU.getObject<PositionBufferCPU>();
		TexCoordBufferCPU* pTCB = mcpu.m_hTexCoordBufferCPU.getObject<TexCoordBufferCPU>();
		NormalBufferCPU* pNB = mcpu.m_hNormalBufferCPU.getObject<NormalBufferCPU>();
		IndexBufferCPU* pIB = mcpu.m_hIndexBufferCPU.getObject<IndexBufferCPU>();

		// Allocate space
		pVB->m_values.reset(4 * 3);    // 4 vertices, each with x,y,z
		pTCB->m_values.reset(4 * 2);   // 4 texcoords, each with u,v
		pNB->m_values.reset(4 * 3);    // 4 normals, each with x,y,z
		pIB->m_values.reset(6);        // 2 triangles (6 indices)

		// Fill vertex positions (assuming center-aligned quad)
		float hw = width / 2.0f;
		float hh = height / 2.0f;

		pVB->m_values.add(-hw, hh, 0); // top-left
		pVB->m_values.add(hw, hh, 0); // top-right
		pVB->m_values.add(hw, -hh, 0); // bottom-right
		pVB->m_values.add(-hw, -hh, 0); // bottom-left

		// Triangle indices (0-1-2, 2-3-0)
		pIB->m_values.add(0, 1, 2);
		pIB->m_values.add(2, 3, 0);

		pIB->m_indexRanges[0].m_start = 0;
		pIB->m_indexRanges[0].m_end = 5;
		pIB->m_indexRanges[0].m_minVertIndex = 0;
		pIB->m_indexRanges[0].m_maxVertIndex = 3;

		pIB->m_minVertexIndex = 0;
		pIB->m_maxVertexIndex = 3;

		// Texture coordinates (full quad)
		pTCB->m_values.add(0, 0); // top-left
		pTCB->m_values.add(1, 0); // top-right
		pTCB->m_values.add(1, 1); // bottom-right
		pTCB->m_values.add(0, 1); // bottom-left

		// Normals
		for (int i = 0; i < 4; ++i)
			pNB->m_values.add(0, 0, 1); // facing out of screen

		if (!m_loaded)
		{
			loadFromMeshCPU_needsRC(mcpu, threadOwnershipMask);

			if (techName)
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
			updateGeoFromMeshCPU_needsRC(mcpu, threadOwnershipMask);
		}
	}

		void ButtonMesh::do_GATHER_DRAWCALLS(Events::Event* pEvt)
		{

		}


	}; // namespace Components
}; // namespace PE
