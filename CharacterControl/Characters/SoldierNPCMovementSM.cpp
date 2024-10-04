#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"

#include <string>

#include "SoldierNPCMovementSM.h"
#include "SoldierNPCAnimationSM.h"
#include "SoldierNPC.h"
using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;

namespace CharacterControl{

// Events sent by behavior state machine (or other high level state machines)
// these are events that specify where a soldier should move


namespace Events{

PE_IMPLEMENT_CLASS1(SoldierNPCMovementSM_Event_MOVE_TO, Event);

SoldierNPCMovementSM_Event_MOVE_TO::SoldierNPCMovementSM_Event_MOVE_TO(Vector3 targetPos /* = Vector3 */)
: m_targetPosition(targetPos)
, m_running(false)
{ }

PE_IMPLEMENT_CLASS1(SoldierNPCMovementSM_Event_STOP, Event);


// 修改 rotate 事件，接受 enemyPosition 和 myPosition 两个参数
PE_IMPLEMENT_CLASS1(SoldierNPCMovementSM_Event_ROTATE, Event);

// 构造函数，现在接受敌人位置和士兵位置
SoldierNPCMovementSM_Event_ROTATE::SoldierNPCMovementSM_Event_ROTATE(Vector3 enemyPos /* = Vector3() */, Vector3 myPos /* = Vector3() */)
	: m_enemyPosition(enemyPos), m_myPosition(myPos)  // 初始化两个位置变量
{ }


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
	PE_REGISTER_EVENT_HANDLER(SoldierNPCMovementSM_Event_ROTATE, SoldierNPCMovementSM::do_SoldierNPCMovementSM_Event_ROTATE);
	PE_REGISTER_EVENT_HANDLER(Event_UPDATE, SoldierNPCMovementSM::do_UPDATE);
}


void SoldierNPCMovementSM::do_SoldierNPCMovementSM_Event_MOVE_TO(PE::Events::Event *pEvt)
{
	SoldierNPCMovementSM_Event_MOVE_TO *pRealEvt = (SoldierNPCMovementSM_Event_MOVE_TO *)(pEvt);
	
	// change state of this state machine
	m_targetPostion = pRealEvt->m_targetPosition;

	OutputDebugStringA("PE: Process: SoldierNPCMovementSM::do_SoldierNPCMovementSM_Event_MOVE_TO(): received event, running: ");
	OutputDebugStringA(pRealEvt->m_running ? "true\n":"false\n");
	if (pRealEvt->m_running)
	{
		m_state = RUNNING_TO_TARGET;

		// make sure the animations are playing

		PE::Handle h("SoldierNPCAnimSM_Event_RUN", sizeof(SoldierNPCAnimSM_Event_RUN));
		Events::SoldierNPCAnimSM_Event_RUN* pOutEvt = new(h) SoldierNPCAnimSM_Event_RUN();

		//SoldierNPCAnimSM_Event_RUN event sent to SceneNode
		SoldierNPC* pSol = getFirstParentByTypePtr<SoldierNPC>();
		pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

		// release memory now that event is processed
		h.release();

	}
	else
	{
		m_state = WALKING_TO_TARGET;

		// make sure the animations are playing

		PE::Handle h("SoldierNPCAnimSM_Event_WALK", sizeof(SoldierNPCAnimSM_Event_WALK));
		Events::SoldierNPCAnimSM_Event_WALK* pOutEvt = new(h) SoldierNPCAnimSM_Event_WALK();

		//SoldierNPCAnimSM_Event_WALK event sent to SceneNode
		SoldierNPC* pSol = getFirstParentByTypePtr<SoldierNPC>();
		pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

		// release memory now that event is processed
		h.release();
	}
}

void SoldierNPCMovementSM::do_SoldierNPCMovementSM_Event_STOP(PE::Events::Event *pEvt)
{
	Events::SoldierNPCAnimSM_Event_STOP Evt;

	//SoldierNPCAnimSM_Event_STOP event sent to SceneNode
	SoldierNPC *pSol = getFirstParentByTypePtr<SoldierNPC>();
	pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(&Evt);
}



void SoldierNPCMovementSM::do_SoldierNPCMovementSM_Event_ROTATE(PE::Events::Event* pEvt)
{
	OutputDebugStringA("ROTATE EVENT HAPPENED\n");
	// 将传入的事件转换为 SoldierNPCMovementSM_Event_ROTATE 类型
	Events::SoldierNPCMovementSM_Event_ROTATE* pRealEvent = (Events::SoldierNPCMovementSM_Event_ROTATE*)(pEvt);

	// 获取敌人的位置和我的(士兵)位置
	Vector3 enemyPosition = pRealEvent->m_enemyPosition;
	Vector3 myPosition = pRealEvent->m_myPosition;


	// 获取 SoldierNPC 对象以检查 npcType
	SoldierNPC* pSol = getFirstParentByTypePtr<SoldierNPC>();
	if (pSol)
	{
		// 检查 npcType 是否为 "Guard"
		if (StringOps::strcmp(pSol->m_npcType, "Guard") == 0)
		{
			// 如果 npcType 是 Guard，则更新状态为 STAND_SHOOTING
			m_state = STAND_SHOOTING;
		}
	}

	// 计算方向向量：从我的位置指向敌人的位置
	Vector3 direction = enemyPosition - myPosition;
	direction.normalize();  // 归一化方向向量

	// 获取 SceneNode 组件，让士兵朝向敌人方向旋转
	PE::Components::SceneNode* pSN = getParentsSceneNode();
	if (pSN)
	{
		// 使用 SceneNode 的 turnInDirection 函数根据方向旋转士兵
		pSN->m_base.turnInDirection(direction, 3.1415f);  // 让士兵面向敌人
	}
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
				float speed = (m_state == WALKING_TO_TARGET) ? 1.4f : 3.0f;
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

						// 1. 获取 SoldierNPC 对象
						SoldierNPC* pSol = getFirstParentByTypePtr<SoldierNPC>();

						if (pSol)
						{
							// 2. 将 NPC 的类型修改为 "Guard"
							StringOps::writeToString("Guard", pSol->m_npcType, 64);  // 将 "Guard" 写入 m_npcType，最大长度为 64
						}

						// send animation event to screennode
						Events::SoldierNPCAnimSM_Event_STAND_SHOOT evt;
						//SoldierNPC *pSol = getFirstParentByTypePtr<SoldierNPC>();
						pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(&evt);
					}
				}
			}
		}
	}

	else if (m_state == STAND_SHOOTING)
	{
		PE::Handle h("SoldierNPCAnimSM_Event_STAND_SHOOT", sizeof(SoldierNPCAnimSM_Event_STAND_SHOOT));
		Events::SoldierNPCAnimSM_Event_STAND_SHOOT* pOutEvt = new(h) SoldierNPCAnimSM_Event_STAND_SHOOT();

		SoldierNPC* pSol = getFirstParentByTypePtr<SoldierNPC>();
		pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

		h.release();
	}
}
}}




