#ifndef __PYENGINE_2_0_UI_EVENT__
#define __PYENGINE_2_0_UI_EVENT__

#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Math/Matrix4x4.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "PrimeEngine/Events/StandardKeyboardEvents.h"


namespace PE {
	namespace Events {
		struct Event_CREATE_BUTTON : public PE::Events::Event_CREATE_MESH
		{
			PE_DECLARE_CLASS(Event_CREATE_BUTTON);

			Event_CREATE_BUTTON(int& threadOwnershipMask) : Event_CREATE_MESH(threadOwnershipMask){}
			static void SetLuaFunctions(PE::Components::LuaEnvironment* pLuaEnv, lua_State* luaVM);

			static int l_Construct(lua_State* luaVM);

			char m_name[32];
			int m_width, m_height;
			char m_text[256];
			char m_drawType[32];
			char m_background[32];
			char m_funcName[64];
			Vector3 m_pos;
			bool isActive;

			PEUUID m_peuuid; // unique object id
		};
	}
}; // namespace PE
#endif

#pragma once
