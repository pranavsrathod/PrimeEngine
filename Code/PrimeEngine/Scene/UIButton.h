#ifndef __PYENGINE_2_0_UI_BUTTON__
#define __PYENGINE_2_0_UI_BUTTON__

#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"
#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Math/Vector3.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/MemoryManagement/Handle.h"
#include "ButtonSceneNode.h"
#include "TextSceneNode.h"
#include "UIElement.h"
#include "UI_Events.h"



namespace PE {
    namespace Components {

        struct UIButton : public UIElement {
            PE_DECLARE_CLASS(UIButton);

            UIButton(PE::GameContext& context, PE::MemoryArena arena, Handle hMyself, Events::Event_CREATE_BUTTON* pEvt);
            UIButton(PE::GameContext& context, PE::MemoryArena arena, Handle hMyself,
                Vector2 pos, float width, float height, char* label);

            static void Construct(PE::GameContext& context, PE::MemoryArena arena,
                Vector2 pos, float width, float height, char* label);
            virtual ~UIButton() {}

            virtual void addDefaultComponents();

            PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PRE_GATHER_DRAWCALLS);
            virtual void do_PRE_GATHER_DRAWCALLS(Events::Event* pEvt);

            virtual void setSelfAndMeshAssetEnabled(bool isActive);

            void setPos(Vector3 pos);
            void createTextSceneNode(const char* str, TextSceneNode::DrawType drawType, int& threadOwnershipMask);
            void createSpriteSceneNode(const char* bgFile, ButtonSceneNode::DrawType drawType, int& threadOwnershipMask, float width, float height);

            static void CreateButton(PE::GameContext& context, PE::MemoryArena arena,
                const char* label,
                const char* bgFile,
                Vector2 pos,
                float width, float height,
                float scale = 1.0f);

            PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_TEST_ONCLICK);
            void do_TEST_ONCLICK(Events::Event* pEvt);

            //UIFunction* m_funcMap;
            char* m_buttonText;
            char* m_buttonImageSource;
            char* m_name;
            char* m_label;

            Vector2 m_position;


            PrimitiveTypes::Int32 m_width;
            PrimitiveTypes::Int32 m_height;


            Handle m_hMySpriteSN;
            Handle m_hMyTextSN;

            GameContext* m_context;
        };

    }; // namespace Components
}; // namespace PE

#endif
