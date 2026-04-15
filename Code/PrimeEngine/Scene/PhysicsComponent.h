#ifndef PHYSICS_COMPONENT_H
#define PHYSICS_COMPONENT_H

#include "PrimeEngine/Scene/SceneNode.h"
#include "PrimeEngine/Math/Vector3.h"
#include "PrimeEngine/Events/Component.h"
using namespace PE::Components;

namespace PE {
	namespace Components {
		class SceneNode;
	}
}
struct PhysicsComponent
{
	enum ColliderType {
		BOX,
		SPHERE,
	};
	struct {
		float radius;
		Vector3 center;
	}sphere;
	struct {
		Vector3 min;
		Vector3 max;
	}box;
	ColliderType m_type;
	PE::Components::SceneNode* p_sceneNode;
	PhysicsComponent(ColliderType type, PE::Components::SceneNode* p_sceneNode);
	void setCenter(Vector3 pos);
};

#endif