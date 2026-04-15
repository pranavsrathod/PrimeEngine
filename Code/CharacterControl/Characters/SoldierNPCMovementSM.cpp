#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"

#include "PrimeEngine/Scene/PhysicsComponent.h"

#include "SoldierNPCMovementSM.h"
#include "SoldierNPCAnimationSM.h"
#include "SoldierNPC.h"
#include <PrimeEngine/Scene/PhysicsManager.h>
using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;

namespace CharacterControl{

// Events sent by behavior state machine (or other high level state machines)
// these are events that specify where a soldier should move
namespace Events{

PE_IMPLEMENT_CLASS1(SoldierNPCMovementSM_Event_MOVE_TO, Event);

SoldierNPCMovementSM_Event_MOVE_TO::SoldierNPCMovementSM_Event_MOVE_TO(Vector3 targetPos /* = Vector3 */)
: m_targetPosition(targetPos), m_running(false)
{ }

PE_IMPLEMENT_CLASS1(SoldierNPCMovementSM_Event_STOP, Event);

PE_IMPLEMENT_CLASS1(SoldierNPCMovementSM_Event_TARGET_REACHED, Event);
}

namespace Components{

PE_IMPLEMENT_CLASS1(SoldierNPCMovementSM, Component);


SoldierNPCMovementSM::SoldierNPCMovementSM(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself) 
: Component(context, arena, hMyself)
, m_state(STANDING)
{}

SceneNode *SoldierNPCMovementSM::getParentsSceneNode()
{
	PE::Handle hParent = getFirstParentByType<Component>();
	if (hParent.isValid())
	{
		// see if parent has scene node component
		return hParent.getObject<Component>()->getFirstComponent<SceneNode>();
		
	}
	return NULL;
}

void SoldierNPCMovementSM::addDefaultComponents()
{
	Component::addDefaultComponents();

	PE_REGISTER_EVENT_HANDLER(SoldierNPCMovementSM_Event_MOVE_TO, SoldierNPCMovementSM::do_SoldierNPCMovementSM_Event_MOVE_TO);
	PE_REGISTER_EVENT_HANDLER(SoldierNPCMovementSM_Event_STOP, SoldierNPCMovementSM::do_SoldierNPCMovementSM_Event_STOP);
	//PE_REGISTER_EVENT_HANDLER(SoldierNPCMovementSM_Event_STOP, SoldierNPCMovementSM::do_SoldierNPCMovementSM_Event_STAND_SHOOT);
	
	PE_REGISTER_EVENT_HANDLER(Event_UPDATE, SoldierNPCMovementSM::do_UPDATE);
}

void SoldierNPCMovementSM::do_SoldierNPCMovementSM_Event_MOVE_TO(PE::Events::Event *pEvt)
{
	SoldierNPCMovementSM_Event_MOVE_TO *pRealEvt = (SoldierNPCMovementSM_Event_MOVE_TO *)(pEvt);
	
	
	m_targetPostion = pRealEvt->m_targetPosition;

	OutputDebugStringA("PE: Progress : SoldierNPCMovementSM::do_SoldierNPCMovementSM_Event_MOVE_TO(): ");
	OutputDebugStringA(pRealEvt->m_running? "true\n" : "false\n");

	// change state of this state machine
	if (pRealEvt->m_running) {
		m_state = RUNNING_TO_TARGET;

		PE::Handle h("SoldierNPCAnimSM_Event_RUN", sizeof(SoldierNPCAnimSM_Event_RUN));
		Events::SoldierNPCAnimSM_Event_RUN* pOutEvt = new(h) SoldierNPCAnimSM_Event_RUN();

		SoldierNPC* pSol = getFirstParentByTypePtr<SoldierNPC>();
		pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

		// release memory now that event is processed
		h.release();
	}
	else {
		m_state = WALKING_TO_TARGET;
		PE::Handle h("SoldierNPCAnimSM_Event_WALK", sizeof(SoldierNPCAnimSM_Event_WALK));
		Events::SoldierNPCAnimSM_Event_WALK * pOutEvt = new(h) SoldierNPCAnimSM_Event_WALK();

		SoldierNPC* pSol = getFirstParentByTypePtr<SoldierNPC>();
		pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

		// release memory now that event is processed
		h.release();
	}
	

	// make sure the animations are playing
	
	//PE::Handle h("SoldierNPCAnimSM_Event_WALK", sizeof(SoldierNPCAnimSM_Event_WALK));
	//Events::SoldierNPCAnimSM_Event_WALK *pOutEvt = new(h) SoldierNPCAnimSM_Event_WALK();
	//
	//SoldierNPC *pSol = getFirstParentByTypePtr<SoldierNPC>();
	//pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

	//// release memory now that event is processed
	//h.release();
}

void SoldierNPCMovementSM::do_SoldierNPCMovementSM_Event_STOP(PE::Events::Event *pEvt)
{
	Events::SoldierNPCAnimSM_Event_STOP Evt;

	SoldierNPC *pSol = getFirstParentByTypePtr<SoldierNPC>();
	pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(&Evt);
}

void SoldierNPCMovementSM::do_UPDATE(PE::Events::Event *pEvt)
{
	if (m_state == WALKING_TO_TARGET || m_state == RUNNING_TO_TARGET)
	{
		// see if parent has scene node component
		SceneNode *pSN = getParentsSceneNode();
		if (pSN)
		{
			Vector3 curPos = pSN->m_base.getPos();
			float dsqr = (m_targetPostion - curPos).lengthSqr();

			bool reached = true;
			if (dsqr > 0.01f)
			{
				// not at the spot yet
				Event_UPDATE *pRealEvt = (Event_UPDATE *)(pEvt);
				float speed = (m_state == WALKING_TO_TARGET) ? 1.4f : 2.1f;
				float allowedDisp = speed * pRealEvt->m_frameTime;

				Vector3 dir = (m_targetPostion - curPos);
				dir.normalize();
				float dist = sqrt(dsqr);
				if (dist > allowedDisp)
				{
					dist = allowedDisp; // can move up to allowedDisp
					reached = false; // not reaching destination yet
				}

				// instantaneous turn
				pSN->m_base.turnInDirection(dir, 3.1415f);
				pSN->m_base.setPos(curPos + dir * dist);

				// Ensure the soldier falls if not on any surface
				/*
				PhysicsComponent* pc = pSN->pc;
				if (pc) {
					bool isOnGround = false;
					float groundLevel = 0.2f;  // Default ground level

					// Check if the soldier is standing on a platform
					for (PhysicsComponent* pcBox : PhysicsManager::getInstance().m_components) {
						if (pcBox->m_type == PhysicsComponent::BOX) {  // Ensure it's a box collider
							Vector3 boxMin = pcBox->box.min;
							Vector3 boxMax = pcBox->box.max;
							Vector3 soldierPos = pSN->m_base.getPos();

							// Check if soldier's X-Z position is within the box's area
							if (soldierPos.getX() >= boxMin.getX() && soldierPos.getX() <= boxMax.getX() &&
								soldierPos.getZ() >= boxMin.getZ() && soldierPos.getZ() <= boxMax.getZ()) {

								// If the soldier is close to the top surface of the box, consider it on the ground
								if (std::abs(soldierPos.getY() - boxMax.getY()) < 0.1f) {
									isOnGround = true;
									groundLevel = boxMax.getY();  // Set new ground level
									break;  // No need to check further, soldier is on a box
								}
							}
						}
					}

					// If the soldier is not on any surface, apply gravity
					if (!isOnGround) {
						float gravity = 9.81f;
						float maxFallSpeed = 20.0f;
						Event_UPDATE* pRealEvt = (Event_UPDATE*)(pEvt);

						// Reduce Y position by gravity factor
						pc->sphere.center.m_y -= gravity * pRealEvt->m_frameTime;
						pc->sphere.center.m_y = std::max(pc->sphere.center.m_y, -maxFallSpeed);

						// Update SceneNode to match physics component position
						Vector3 updatedPos = pSN->m_base.getPos();
						updatedPos.m_y = pc->sphere.center.m_y;
						pSN->m_base.setPos(updatedPos);

						PEINFO("Gravity applied: Soldier falling to (%f, %f, %f)\n",
							updatedPos.m_x, updatedPos.m_y, updatedPos.m_z);
					}
				}
				*/
			}

			if (reached)
			{
				m_state = STANDING;
				
				// target has been reached. need to notify all same level state machines (components of parent)
				{
					PE::Handle h("SoldierNPCMovementSM_Event_TARGET_REACHED", sizeof(SoldierNPCMovementSM_Event_TARGET_REACHED));
					Events::SoldierNPCMovementSM_Event_TARGET_REACHED *pOutEvt = new(h) SoldierNPCMovementSM_Event_TARGET_REACHED();

					PE::Handle hParent = getFirstParentByType<Component>();
					if (hParent.isValid())
					{
						hParent.getObject<Component>()->handleEvent(pOutEvt);
					}
					
					// release memory now that event is processed
					h.release();
				}

				if (m_state == STANDING)
				{
					// no one has modified our state based on TARGET_REACHED callback
					// this means we are not going anywhere right now
					// so can send event to animation state machine to stop
					{
						Events::SoldierNPCAnimSM_Event_STOP evt;
						
						SoldierNPC *pSol = getFirstParentByTypePtr<SoldierNPC>();
						pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(&evt);
						
					}
					{
						PE::Handle h("SoldierNPCAnimSM_Event_STAND_SHOOT", sizeof(SoldierNPCAnimSM_Event_STAND_SHOOT));
						Events::SoldierNPCAnimSM_Event_STAND_SHOOT* pOutEvt = new(h) SoldierNPCAnimSM_Event_STAND_SHOOT();

						SoldierNPC* pSol = getFirstParentByTypePtr<SoldierNPC>();
						pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

						// release memory now that event is processed
						h.release();
					}
				}
			}
		}
	}
}

}}




