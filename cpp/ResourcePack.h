
#pragma once

namespace Windows::UI::DynamicWindow
{
    class ResourcePack
    {
    public:
        class NinegridData 
        { public:
            int standard;
            int popup;
            int secondary;
        } ninegrid;

        class ToggleSwitchData 
        { public:
            int on;
            int off;
        } switches;

        int mainIcon;

        ResourcePack()
        {
            
        }
        ResourcePack(NinegridData nd, ToggleSwitchData td)
        {
            ninegrid = nd;
            switches = td;
        }
    public:
        static void SetStandardIcon(int iconID)
        {
            Standard_Light->mainIcon = iconID;
            Standard_Dark->mainIcon = iconID;
        }
        static ResourcePack* Standard_Light;
        static ResourcePack* Standard_Dark;
    };
}