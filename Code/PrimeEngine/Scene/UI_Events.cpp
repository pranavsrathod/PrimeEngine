#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Lua/EventGlue/EventDataCreators.h"


#include "UI_Events.h"


using namespace PE;
using namespace PE::Components;
using namespace PE::Events;

namespace PE {
	namespace Events {

		PE_IMPLEMENT_CLASS1(Event_CREATE_BUTTON, PE::Events::Event);

		void Event_CREATE_BUTTON::SetLuaFunctions(PE::Components::LuaEnvironment* pLuaEnv, lua_State* luaVM)
		{
			static const struct luaL_Reg l_Event_CREATE_BUTTON[] = {
				{"Construct", l_Construct},
				{NULL, NULL} // sentinel
			};

			// register the functions in current lua table which is the table for Event_CreateSoldierNPC
			luaL_register(luaVM, 0, l_Event_CREATE_BUTTON);
		}

		int Event_CREATE_BUTTON::l_Construct(lua_State* luaVM)
		{
			PE::Handle h("CREATE_BUTTON_EVENT", sizeof(Event_CREATE_BUTTON));
	

			// get arguments from stack
			int numArgs, numArgsConst;
			numArgs = numArgsConst = 12;

			PE::GameContext* pContext = (PE::GameContext*)(lua_touserdata(luaVM, -numArgs--));

			Event_CREATE_BUTTON* pEvt = new(h) Event_CREATE_BUTTON(pContext->m_gameThreadThreadOwnershipMask);

			const char* name = lua_tostring(luaVM, -numArgs--);
			float width = (float)lua_tonumber(luaVM, -numArgs--);
			float height = (float)lua_tonumber(luaVM, -numArgs--);
			const char* text = lua_tostring(luaVM, -numArgs--);
			const char* bgFile = lua_tostring(luaVM, -numArgs--);
			const char* drawType = lua_tostring(luaVM, -numArgs--);
			const char* funcName = lua_tostring(luaVM, -numArgs--);

			Vector3 buttonPos;
			buttonPos.m_x = (float)lua_tonumber(luaVM, -numArgs--);
			buttonPos.m_y = (float)lua_tonumber(luaVM, -numArgs--);
			buttonPos.m_z = (float)lua_tonumber(luaVM, -numArgs--);

			pEvt->m_peuuid = LuaGlue::readPEUUID(luaVM, -numArgs--);
			StringOps::writeToString(name, pEvt->m_name, 255);
			StringOps::writeToString(drawType, pEvt->m_drawType,255);
			pEvt->m_width = width;
			pEvt->m_height = height;
			StringOps::writeToString(text, pEvt->m_text, 255);
			StringOps::writeToString(bgFile, pEvt->m_background, 255);
			StringOps::writeToString(funcName, pEvt->m_funcName, 255);

			pEvt->m_pos = buttonPos;

			lua_pop(luaVM, numArgsConst);

			//lua_pop(luaVM, numArgsConst); //Second arg is a count of how many to pop

			/*pEvt->m_base.loadIdentity();
			pEvt->m_base.setPos(pos);
			pEvt->m_base.setU(u);
			pEvt->m_base.setV(v);
			pEvt->m_base.setN(n);*/

			LuaGlue::pushTableBuiltFromHandle(luaVM, h);

			return 1;
		}
	};
}; // namespace PE
