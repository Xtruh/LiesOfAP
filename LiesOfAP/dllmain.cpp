#include <stdio.h>
#include <Mod/CppUserModBase.hpp>

class LiesOFAP : public RC::CppUserModBase
{
public:
    LiesOFAP() : CppUserModBase()
    {
        ModName = STR("LiesOFAP");
        ModVersion = STR("1.0");
        ModDescription = STR("A Lies Of P Randomizer made for the Archipelago multiworld randomizer");
        ModAuthors = STR("Ninjakakes, Xtruh");
        // Do not change this unless you want to target a UE4SS version
        // other than the one you're currently building with somehow.
        //ModIntendedSDKVersion = STR("2.6");
        
        printf("LiesOFAP says hello\n");
    }

    ~LiesOFAP() override
    {
    }

    auto on_update() -> void override
    {
    }
};

extern "C"
{
    __declspec(dllexport) RC::CppUserModBase* start_mod()
    {
        return new LiesOFAP();
    }

    __declspec(dllexport) void uninstall_mod(RC::CppUserModBase* mod)
    {
        delete mod;
    }
}