#ifndef PHYSICS_MANAGER_H
#define PHYSICS_MANAGER_H

#include <vector>
#include "PhysicsComponent.h"
#include "PrimeEngine/Math/Vector3.h"

struct PhysicsManager
{
private:
	// Private constructor
	PhysicsManager() {}

	// Static instance of the PhysicsManager
	static PhysicsManager s_instance;

public:
	// Deleted copy constructor and assignment operator to prevent copies
	PhysicsManager(const PhysicsManager&) = delete;
	PhysicsManager& operator=(const PhysicsManager&) = delete;

	// GetInstance method to provide access to the instance
	static PhysicsManager& getInstance()
	{
		return s_instance;
	}



	std::vector<PhysicsComponent*> m_components; // a list of all physics components

	Vector3 updateVector;// the displacement vector add to current pos after collision detection.



	void addComponent(PhysicsComponent* pComponent);

	void start(float m_framerate); // run physics simulation
	void end();
};
#endif