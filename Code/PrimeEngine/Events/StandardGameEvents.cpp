
#include "StandardGameEvents.h"

#include "../Lua/LuaEnvironment.h"

namespace PE {
namespace Events {
	
	PE_IMPLEMENT_CLASS1(Event_RESUEME_GAME, Event);
	PE_IMPLEMENT_CLASS1(Event_FLY_CAMERA, Event);
	PE_IMPLEMENT_CLASS1(Event_PAUSE_GAME, Event);
	PE_IMPLEMENT_CLASS1(Event_DEBUG_TEXT, Event);
	PE_IMPLEMENT_CLASS1(Event_ROTATE_CAMERA, Event);
};
};
