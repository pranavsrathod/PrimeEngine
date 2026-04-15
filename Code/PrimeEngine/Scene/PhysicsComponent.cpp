#include "PhysicsComponent.h"
#include "PrimeEngine/Scene/RootSceneNode.h"
#include "PrimeEngine/Scene/DebugRenderer.h"

// Inter-Engine includes
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "Mesh.h"
// Outer-Engine includes

// Inter-Engine includes
#include "PrimeEngine/FileSystem/FileReader.h"
#include "PrimeEngine/APIAbstraction/GPUMaterial/GPUMaterialSet.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/APIAbstraction/Texture/Texture.h"
#include "PrimeEngine/APIAbstraction/Effect/EffectManager.h"
#include "PrimeEngine/APIAbstraction/GPUBuffers/VertexBufferGPUManager.h"
#include "PrimeEngine/../../GlobalConfig/GlobalConfig.h"
#include "Light.h"

// Sibling/Children includes

#include "MeshInstance.h"
#include "SceneNode.h"
#include "DrawList.h"
#include "SH_DRAW.h"
#include "../Lua/LuaEnvironment.h"

PhysicsComponent::PhysicsComponent(ColliderType type, PE::Components::SceneNode* p_sceneNode)
	: m_type(type), p_sceneNode(p_sceneNode)
{
}

void PhysicsComponent::setCenter(Vector3 pos)
{
	sphere.center = pos;

}