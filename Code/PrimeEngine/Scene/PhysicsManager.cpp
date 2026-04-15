#include "PhysicsManager.h"
#include "PrimeEngine/Math/Matrix4x4.h"  
#include "PrimeEngine/Scene/SceneNode.h"
#include "CharacterControl/Characters/SoldierNPC.h"
#include "CharacterControl/Characters/SoldierNPCMovementSM.h"
#include "PrimeEngine/Scene/Mesh.h"

using namespace CharacterControl::Events;
using namespace CharacterControl::Components;
using namespace PE::Components;

// Static instance initialization
PhysicsManager PhysicsManager::s_instance;

const bool checkCorners(Vector3* closestPoint, Vector3* max, Vector3* min)
{
    bool a = closestPoint->getX() == max->getX() && closestPoint->getZ() == max->getZ();
    bool b = closestPoint->getX() == min->getX() && closestPoint->getZ() == min->getZ();
    bool c = closestPoint->getX() == max->getX() && closestPoint->getZ() == min->getZ();
    bool d = closestPoint->getX() == min->getX() && closestPoint->getZ() == max->getZ();
    if (a || b || c || d)
    {
        return false;
    }
    return true;
}

void PhysicsManager::addComponent(PhysicsComponent* pComponent)
{
    // Add the component to the list
    m_components.push_back(pComponent);
}

/*void PhysicsManager::start(float m_framerate)
{
    Vector3 update;
    // Run the physics simulation for sphere components
    for (PhysicsComponent* pcSphere : m_components)
    {
        if (pcSphere->m_type == PhysicsComponent::SPHERE)
        {
            // Test against every box in the list
            for (PhysicsComponent* pcBox : m_components)
            {
                if (pcBox->m_type == PhysicsComponent::BOX)
                {
                    Vector3 closestPoint;
                    Matrix4x4 boxTransMatrix = pcBox->p_sceneNode->m_base;
                    Matrix4x4 boxLocalTrans = pcBox->p_sceneNode->m_base.inverse();
                    Vector3 sphereCenter = boxLocalTrans * pcSphere->sphere.center;
                    Vector3 boxOriginMax = boxLocalTrans * pcBox->box.max;
                    Vector3 boxOriginMin = boxLocalTrans * pcBox->box.min;

                    float x = std::max(boxOriginMin.getX(), std::min(sphereCenter.getX(), boxOriginMax.getX()));
                    float y = std::max(boxOriginMin.getY(), std::min(sphereCenter.getY(), boxOriginMax.getY()));
                    float z = std::max(boxOriginMin.getZ(), std::min(sphereCenter.getZ(), boxOriginMax.getZ()));
                    y = 0; // Force Y to 0
                    closestPoint = Vector3(x, y, z);
                    float distance = (closestPoint - sphereCenter).length();

                    if (boxOriginMax.getY() != 0.0 && distance <= pcSphere->sphere.radius && checkCorners(&closestPoint, &boxOriginMax, &boxOriginMin))
                    {
                        char buf[80];
                        sprintf(buf, "collide!: x:%f, y:%f, z:%f", pcSphere->sphere.center.getX(), pcSphere->sphere.center.getY(), pcSphere->sphere.center.getZ());

                        SoldierNPC* pSol = pcSphere->p_sceneNode->getFirstParentByTypePtr<SoldierNPC>();
                        if (pSol)
                        {  // Check if it's really a soldier
                            SoldierNPCMovementSM* movementSM = pSol->getFirstComponent<SoldierNPCMovementSM>();
                            if (movementSM)
                            {
                                // Calculate normal vector
                                Vector3 norm = (closestPoint - sphereCenter);
                                if (norm.length() != 0)
                                {
                                    norm.normalize();
                                }
                                // Calculate velocity vector
                                SceneNode* pSN = pcSphere->p_sceneNode;
                                Vector3 curPos = boxLocalTrans * pSN->m_base.getPos();
                                Vector3 m_targetPostion = boxLocalTrans * movementSM->m_targetPostion;
                                Vector3 velosity = m_targetPostion - curPos;
                                if (velosity.length() != 0)
                                {
                                    velosity.normalize();
                                }

                                if (norm.dotProduct(velosity) > 0)
                                {
                                    velosity = 1.4f * velosity;
                                    Vector3 v_result = velosity - norm * (velosity.dotProduct(norm));

                                    // Convert back to world space
                                    Vector3 convertedResult;
                                    convertedResult.m_x = v_result.dotProduct(boxTransMatrix.getN());
                                    convertedResult.m_y = v_result.dotProduct(boxTransMatrix.getV());
                                    convertedResult.m_z = v_result.dotProduct(boxTransMatrix.getU());
                                    Vector3 convertedNorm;
                                    convertedNorm.m_x = norm.dotProduct(boxTransMatrix.getN());
                                    convertedNorm.m_y = norm.dotProduct(boxTransMatrix.getV());
                                    convertedNorm.m_z = norm.dotProduct(boxTransMatrix.getU());

                                    // Set updated position
                                    update = -1 * convertedNorm * (1 - distance + 0.03) + convertedResult * m_framerate;
                                    pcSphere->p_sceneNode->m_base.setPos(pSN->m_base.getPos() + update);
                                }
                            }
                        }
                    }
                    else
                    {
                        float xSphere = sphereCenter.getX();
                        float zSphere = sphereCenter.getZ();

                        if (boxOriginMax.getX() > xSphere && xSphere > boxOriginMin.getX() && boxOriginMax.getZ() > zSphere && zSphere > boxOriginMin.getZ())
                        {
                            float yGround = 0.0f;
                            float ySphere = pcSphere->sphere.center.getY();
                            if (ySphere < yGround)
                            {
                                Vector3 update = { pcSphere->p_sceneNode->m_base.getPos().getX(), 0.2, pcSphere->p_sceneNode->m_base.getPos().getZ() };
                                pcSphere->p_sceneNode->m_base.setPos(update);
                            }
                        }
                    }
                }
            }
        }
    }
}*/


void PhysicsManager::start(float m_framerate)
{
    Vector3 update;
    // Run the physics simulation for sphere component
    for (PhysicsComponent* pcSphere : m_components)
    {
        if (pcSphere->m_type == PhysicsComponent::SPHERE) {
            //test against every box in array
            for (PhysicsComponent* pcBox : m_components)
            {
                if (pcBox->m_type == PhysicsComponent::BOX) {
                    
                        Vector3 closestPoint;
                        Matrix4x4 boxTransMatrix = pcBox->p_sceneNode->m_base;
                        Matrix4x4 boxLocalTrans = pcBox->p_sceneNode->m_base.inverse();
                        Vector3 sphereCenter = boxLocalTrans * pcSphere->sphere.center;
                        Vector3 boxOriginMax = boxLocalTrans * pcBox->box.max;
                        Vector3 boxOriginMin = boxLocalTrans * pcBox->box.min;




                        float x = std::max(boxOriginMin.getX(), std::min(sphereCenter.getX(), boxOriginMax.getX()));
                        float y = std::max(boxOriginMin.getY(), std::min(sphereCenter.getY(), boxOriginMax.getY()));
                        float z = std::max(boxOriginMin.getZ(), std::min(sphereCenter.getZ(), boxOriginMax.getZ()));
                        //float y = std::max(pcBox->box.min.getY(), std::min(sphereCenter.getY(), pcBox->box.max.getY()));
                        //float z = std::max(pcBox->box.min.getZ(), std::min(sphereCenter.getZ(), pcBox->box.max.getZ l
                         y = 0;
                        closestPoint = Vector3(x, y, z);
                        float distance = (closestPoint - sphereCenter).length();

                        if (boxOriginMax.getY() != 0.0 && distance <= pcSphere->sphere.radius && checkCorners(&closestPoint ,&boxOriginMax ,&boxOriginMin) )
                        {
                            char buf[80];
                            sprintf(buf, "collide!: x:%f, y:%f, z:%f", pcSphere->sphere.center.getX(), pcSphere->sphere.center.getY(), pcSphere->sphere.center.getZ());

                            SoldierNPC* pSol = pcSphere->p_sceneNode->getFirstParentByTypePtr<SoldierNPC>();
                            if (pSol) {  // check if it's really a soldier
                                SoldierNPCMovementSM* movementSM = pSol->getFirstComponent<SoldierNPCMovementSM>();
                                if (movementSM) {
                                    //calculate norm
                                    Vector3 norm = (closestPoint - sphereCenter);
                                    if (norm.length() != 0) {
                                        norm.normalize();
                                    }
                                    //calculate velosity
                                    SceneNode* pSN = pcSphere->p_sceneNode;
                                    Vector3 curPos = boxLocalTrans * pSN->m_base.getPos();
                                    Vector3 m_targetPostion = boxLocalTrans * movementSM->m_targetPostion;
                                    Vector3 velosity = m_targetPostion - curPos;
                                    if (velosity.length() != 0) {
                                        velosity.normalize();
                                    }

                                    if (norm.dotProduct(velosity) > 0) {
                                        velosity = 1.4f  * velosity;
                                        //calculate 
                                        Vector3 v_result = velosity - norm * (velosity.dotProduct(norm));
                                        
                                        // Convert back to world space
                                        Vector3 convertedResult;
                                        convertedResult.m_x = v_result.dotProduct(boxTransMatrix.getN());
                                        convertedResult.m_y = v_result.dotProduct(boxTransMatrix.getV());
                                        convertedResult.m_z = v_result.dotProduct(boxTransMatrix.getU());
                                        Vector3 convertedNorm;
                                        convertedNorm.m_x = norm.dotProduct(boxTransMatrix.getN());
                                        convertedNorm.m_y = norm.dotProduct(boxTransMatrix.getV());
                                        convertedNorm.m_z = norm.dotProduct(boxTransMatrix.getU());

                                        
                                        // Set updated position
                                        updateVector = -1 * convertedNorm * (1 - distance + 0.03) + convertedResult * m_framerate;

                                        pcSphere->p_sceneNode->m_base.setPos(pSN->m_base.getPos() + updateVector);
                                    }


                                }
                            }

                        }
                       else
                        {
                            float xSphere = sphereCenter.getX();
                            float zSphere = sphereCenter.getZ();

                            if (boxOriginMax.getX() > xSphere && xSphere > boxOriginMin.getX() && boxOriginMax.getZ() > zSphere && zSphere > boxOriginMin.getZ())
                            {
                                float yGround = 0.0f;
                                float ySphere = pcSphere->sphere.center.getY();
                                if (ySphere < yGround)
                                {
                                    //pcSphere->p_sceneNode->m_base.getPos().m_y = 0;
                                    Vector3 update = { pcSphere->p_sceneNode->m_base.getPos().getX(),0.2,pcSphere->p_sceneNode->m_base.getPos().getZ() };
                                    pcSphere->p_sceneNode->m_base.setPos(update);
                                }
                            }

                        }
                    
                        
                        


                    
                }
            }
        }
    }
}

void PhysicsManager::end()
{
}