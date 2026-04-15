#ifndef __PYENGINE_2_0_CAMERA_SCENE_NODE_H__
#define __PYENGINE_2_0_CAMERA_SCENE_NODE_H__

// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
#include <assert.h>

// Inter-Engine includes
#include "PrimeEngine/Render/IRenderer.h"
#include "PrimeEngine/MemoryManagement/Handle.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "../Events/Component.h"
#include "../Utils/Array/Array.h"
#include "PrimeEngine/Math/CameraOps.h"

#include "SceneNode.h"
#include "Mesh.h"


// Sibling/Children includes

namespace PE {
namespace Components {

	struct Plane {
		Vector3 normal;
		float distance;
		Plane() {};
		Plane(const Vector3& p1, const Vector3& p2, const Vector3& p3) {
			Vector3 v1 = p1 - p2;
			Vector3 v2 = p2 - p3;
			normal = v1.crossProduct(v2);
			normal.normalize();
			distance = -normal.dotProduct(p1);
		}
		bool isPointInFront(Vector3& point) {
			return normal.dotProduct(point) + distance < 0.0f;
		}
		void normalize() {
			float length = normal.length();
			normal /= length;
			distance /= length;
		}
	};

	struct Frustum {
		Plane planes[6];
		Vector3 corners[8];
		Frustum() {};
		bool isBoxInside(AABB& aabb) {
			for (int i = 0; i < 6; ++i) {
				int outCount = 0;
				for (int j = 0; j < 8; ++j) {
					if (!planes[i].isPointInFront(aabb.corners[j])) {
						outCount++;
					}
				}
				if (outCount == 8) {
					return false;
				}
			}
			return true;
		}
	};

    //struct FrustumPlane {
    //    Vector3 normal;  // Plane normal (A, B, C)
    //    float d;         // Distance from origin (D)

    //    // Constructor
    //    FrustumPlane() : normal(Vector3(0, 0, 0)), d(0) {}

    //    FrustumPlane(float a, float b, float c, float d) {
    //        normal = Vector3(a, b, c);
    //        this->d = d;
    //        normalize();
    //    }

    //    // Normalize the plane
    //    void normalize() {
    //        float length = normal.length();
    //        if (length > 0) {
    //            normal /= length;
    //            d /= length;
    //        }
    //    }
    //    // Define dotProduct inside the struct
    //    float dotProduct(const Vector3& point) const {
    //        return (normal.m_x * point.m_x) + (normal.m_y * point.m_y) + (normal.m_z * point.m_z);
    //    }

    //    // Frustum Plane test function
    //    bool isPointInside(const Vector3& point) const {
    //        return (dotProduct(point) + d) >= 0;
    //    }
    //};

struct CameraSceneNode : public SceneNode
{

	PE_DECLARE_CLASS(CameraSceneNode);

	// Constructor -------------------------------------------------------------
	CameraSceneNode(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself);

	virtual ~CameraSceneNode(){}

	// Component ------------------------------------------------------------
	virtual void addDefaultComponents();

	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_CALCULATE_TRANSFORMATIONS);
	virtual void do_CALCULATE_TRANSFORMATIONS(Events::Event *pEvt);
	void frustumValues(Matrix4x4& combinedMatrix, PrimitiveTypes::Float32 verticalFov, PrimitiveTypes::Float32 aspect);
    bool isAABBInFrustum(const AABB& box);
    void definePlanes();

	// Individual events -------------------------------------------------------

	
	Matrix4x4 m_worldToViewTransform; // objects in world space are multiplied by this to get them into camera's coordinate system (view space)
	Matrix4x4 m_worldToViewTransform2;
	Matrix4x4 m_worldTransform2;
	Matrix4x4 m_viewToProjectedTransform; // objects in local (view) space are multiplied by this to get them to screen space
	float m_near, m_far;
    //FrustumPlane m_frustumPlanes[6];  // Stores the six frustum planes
	Frustum frustum;
};
}; // namespace Components
}; // namespace PE
#endif
