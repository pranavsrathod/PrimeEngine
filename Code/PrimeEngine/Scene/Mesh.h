#ifndef __PYENGINE_2_0_MESH_H__
#define __PYENGINE_2_0_MESH_H__

#define NOMINMAX
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
#include <assert.h>

// Inter-Engine includes
#include "PrimeEngine/MemoryManagement/Handle.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "../Events/Component.h"
#include "../Utils/Array/Array.h"
#include "../Geometry/MeshCPU/MeshCPU.h"
#include "../Math/Matrix4x4.h"

#include "PrimeEngine/APIAbstraction/GPUBuffers/VertexBufferGPU.h"
#include "PrimeEngine/APIAbstraction/GPUBuffers/IndexBufferGPU.h"

#include "PrimeEngine/APIAbstraction/Effect/Effect.h"

// Sibling/Children includes

namespace PE {
struct MaterialSetCPU;
namespace Components {
struct AABB {
	Vector3 min;
	Vector3 max;
	Vector3 corners[8]; // Store the 8 corners
	Vector3 pos;

	// Initialize AABB with extreme values
	AABB() {}
	AABB(Vector3 min, Vector3 max) {
		this->min = min;
		this->max = max;
		corners[0] = min;
		corners[1] = Vector3(min.m_x, min.m_y, max.m_z);
		corners[2] = Vector3(min.m_x, max.m_y, min.m_z);
		corners[3] = Vector3(min.m_x, max.m_y, max.m_z);
		corners[4] = Vector3(max.m_x, min.m_y, min.m_z);
		corners[5] = Vector3(max.m_x, min.m_y, max.m_z);
		corners[6] = Vector3(max.m_x, max.m_y, min.m_z);
		corners[7] = max;
	}

	Vector3 getCenter() const {
		return Vector3(
			(min.m_x + max.m_x) / 2.0f,
			(min.m_y + max.m_y) / 2.0f,
			(min.m_z + max.m_z) / 2.0f
		);
	}
	void setMinMax(Vector3& minVal, Vector3& maxVal, Matrix4x4& base) {

		updateCorners(minVal, maxVal, base);
		min = base * minVal;
		max = base * maxVal;
	}
	private:
		void updateCorners(Vector3& minVal, Vector3& maxVal, Matrix4x4& base) {
			corners[0] = base * minVal;
			corners[1] = base * Vector3(minVal.getX(), minVal.getY(), maxVal.getZ());
			corners[2] = base * Vector3(minVal.getX(), maxVal.getY(), minVal.getZ());
			corners[3] = base * Vector3(minVal.getX(), maxVal.getY(), maxVal.getZ());
			corners[4] = base * Vector3(maxVal.getX(), minVal.getY(), minVal.getZ());
			corners[5] = base * Vector3(maxVal.getX(), minVal.getY(), maxVal.getZ());
			corners[6] = base * Vector3(maxVal.getX(), maxVal.getY(), minVal.getZ());
			corners[7] = base * maxVal;
		}
};

struct Mesh : public Component
{
	PE_DECLARE_CLASS(Mesh);

	// Constructor -------------------------------------------------------------
	Mesh(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself);

	virtual ~Mesh(){}

	virtual void addDefaultComponents();

	// need this to maintain m_instances
	virtual void addComponent(Handle hComponent, int *pAllowedEvents = NULL);
	virtual void removeComponent(int index);


	// Methods -----------------------------------------------------------------

	// Builds a Mesh from the data in system memory
	void loadFromMeshCPU_needsRC(MeshCPU &mcpu, int &threadOwnershipMask);
	EPEVertexFormat updateGeoFromMeshCPU_needsRC(MeshCPU &mcpu, int &threadOwnershipMask);

	// Component ------------------------------------------------------------

	// Individual events -------------------------------------------------------
	
	Handle &nextAdditionalShaderValue(int size)
	{
		m_additionalShaderValues.add(Handle("RAW_DATA", size));
		return m_additionalShaderValues[m_additionalShaderValues.m_size-1];
	}


	void overrideEffects(Handle newEffect);
	void overrideZOnlyEffects(Handle newEffect);
	

	void popEffects();

	PrimitiveTypes::Bool hasPushedEffects();
	// Member variables --------------------------------------------------------
	//Handle m_hVertexBufferGPU;
	Handle m_hTexCoordBufferCPU;
	
	Handle m_hIndexBufferGPU;
	
	Handle m_hMaterialSetGPU;

	PrimitiveTypes::Bool m_processShowEvt;

	Handle m_hPositionBufferCPU;
	Handle m_hNormalBufferCPU;
	Handle m_hTangentBufferCPU;

	Handle m_hSkinWeightsCPU;

	Array<Handle> m_additionalShaderValues;

	PEStaticVector<Handle, 4> m_vertexBuffersGPUHs;

	Array< PEStaticVector<Handle, 4> > m_effects;
	Array< PEStaticVector<Handle, 4> > m_zOnlyEffects;
	Array< PEStaticVector<Handle, 4> > m_instanceEffects;

	Array<Handle, 1> m_instances; // special cahce of instances
	Array<Handle> m_lods;
    int m_numVisibleInstances;
	
	Handle m_hAnimationSetGPU; // reference to animation stored in gpu buffer

	Vector3 m_maxBB;
	Vector3 m_minBB;
	AABB m_BB;
	
	bool m_bDrawControl;
    
    bool m_performBoundingVolumeCulling;
};

}; // namespace Components
}; // namespace PE
#endif
