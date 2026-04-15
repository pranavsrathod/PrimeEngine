// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "MeshManager.h"
#include <sstream>    // For std::ostringstream
// Outer-Engine includes

// Inter-Engine includes
#include "PrimeEngine/FileSystem/FileReader.h"
#include "PrimeEngine/APIAbstraction/GPUMaterial/GPUMaterialSet.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/APIAbstraction/Texture/Texture.h"
#include "PrimeEngine/APIAbstraction/Effect/EffectManager.h"
#include "PrimeEngine/APIAbstraction/GPUBuffers/VertexBufferGPUManager.h"
#include "PrimeEngine/../../GlobalConfig/GlobalConfig.h"

#include "PrimeEngine/Geometry/SkeletonCPU/SkeletonCPU.h"

#include "PrimeEngine/Scene/RootSceneNode.h"

#include "Light.h"

// Sibling/Children includes

#include "MeshInstance.h"
#include "Skeleton.h"
#include "SceneNode.h"
#include "DrawList.h"
#include "SH_DRAW.h"
#include "PrimeEngine/Lua/LuaEnvironment.h"

namespace PE {
namespace Components{

PE_IMPLEMENT_CLASS1(MeshManager, Component);
MeshManager::MeshManager(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself)
	: Component(context, arena, hMyself)
	, m_assets(context, arena, 256)
{
}

PE::Handle MeshManager::getAsset(const char *asset, const char *package, int &threadOwnershipMask)
{
	char key[StrTPair<Handle>::StrSize];
	sprintf(key, "%s/%s", package, asset);
	
	int index = m_assets.findIndex(key);
	if (index != -1)
	{
		return m_assets.m_pairs[index].m_value;
	}
	Handle h;

	if (StringOps::endswith(asset, "skela"))
	{
		PE::Handle hSkeleton("Skeleton", sizeof(Skeleton));
		Skeleton *pSkeleton = new(hSkeleton) Skeleton(*m_pContext, m_arena, hSkeleton);
		pSkeleton->addDefaultComponents();

		pSkeleton->initFromFiles(asset, package, threadOwnershipMask);
		h = hSkeleton;
	}
	else if (StringOps::endswith(asset, "mesha"))
	{
		MeshCPU mcpu(*m_pContext, m_arena);
		mcpu.ReadMesh(asset, package, "");
		
		PE::Handle hMesh("Mesh", sizeof(Mesh));
		Mesh *pMesh = new(hMesh) Mesh(*m_pContext, m_arena, hMesh);
		pMesh->addDefaultComponents();

		pMesh->loadFromMeshCPU_needsRC(mcpu, threadOwnershipMask);

#if PE_API_IS_D3D11
		// todo: work out how lods will work
		//scpu.buildLod();
#endif
		PositionBufferCPU* vertices = pMesh->m_hPositionBufferCPU.getObject<PositionBufferCPU>();

		if (!(vertices->m_values.m_size < 1)) {
			Vector3 vertexMin = Vector3(vertices->m_values[0], vertices->m_values[1], vertices->m_values[2]);
			Vector3 vertexMax = Vector3(vertices->m_values[0], vertices->m_values[1], vertices->m_values[2]);

			for (PrimitiveTypes::UInt32 i = 0; i < vertices->m_values.m_size; i += 3) {
				vertexMin.m_x = std::min(vertexMin.m_x, vertices->m_values[i + 0]);
				vertexMin.m_y = std::min(vertexMin.m_y, vertices->m_values[i + 1]);
				vertexMin.m_z = std::min(vertexMin.m_z, vertices->m_values[i + 2]);

				vertexMax.m_x = std::max(vertexMax.m_x, vertices->m_values[i + 0]);
				vertexMax.m_y = std::max(vertexMax.m_y, vertices->m_values[i + 1]);
				vertexMax.m_z = std::max(vertexMax.m_z, vertices->m_values[i + 2]);
			}
			pMesh->m_minBB = vertexMin;
			pMesh->m_maxBB = vertexMax;
			pMesh->m_BB = AABB(vertexMin, vertexMax);
			//std::ostringstream debugOutput;
			//debugOutput << "vertexMin: (" << vertexMin.m_x << ", " << vertexMin.m_y << ", " << vertexMin.m_z << ")\n";
			//debugOutput << "vertexMax: (" << vertexMax.m_x << ", " << vertexMax.m_y << ", " << vertexMax.m_z << ")\n";

			//// Output to debug console
			//OutputDebugStringA(debugOutput.str().c_str());

		}
        // generate collision volume here. or you could generate it in MeshCPU::ReadMesh()
        pMesh->m_performBoundingVolumeCulling = true; // will now perform tests for this mesh

		h = hMesh;
	}


	PEASSERT(h.isValid(), "Something must need to be loaded here");

	RootSceneNode::Instance()->addComponent(h);
	m_assets.add(key, h);
	return h;
}

void MeshManager::registerAsset(const PE::Handle &h)
{
	static int uniqueId = 0;
	++uniqueId;
	char key[StrTPair<Handle>::StrSize];
	sprintf(key, "__generated_%d", uniqueId);
	
	int index = m_assets.findIndex(key);
	PEASSERT(index == -1, "Generated meshes have to be unique");
	
	RootSceneNode::Instance()->addComponent(h);
	m_assets.add(key, h);
}

}; // namespace Components
}; // namespace PE
