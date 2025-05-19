#pragma comment (lib, "crypt32")

#define ASIO_STANDALONE

#include "Mod/CppUserModBase.hpp"

#include "apclient.hpp"
#include "apuuid.hpp"

class LiesOFAP : public RC::CppUserModBase
{
public:
    APClient* ap;
    std::string uuid = ap_get_uuid("");

    LiesOFAP() : CppUserModBase()
    {
        ModName = STR("LiesOFAP");
        ModVersion = STR("1.0");
        ModDescription = STR("A Lies Of P Randomizer made for the Archipelago multiworld randomizer");
        ModAuthors = STR("Ninjakakes, Xtruh");
        // Do not change this unless you want to target a UE4SS version
        // other than the one you're currently building with somehow.
        //ModIntendedSDKVersion = STR("2.6");
        
    }

    ~LiesOFAP() override
    {
        if (ap)
            delete ap;
    }

    auto on_update() -> void override
    {
        if (ap)
            ap->poll();
    }

    auto on_unreal_init() -> void override
    {
        ap = new APClient(uuid, "Clique");
        ap->set_room_info_handler([&]() 
            {
            int items_handling = 0b111;
            APClient::Version version{ 0, 6, 2 };
            ap->ConnectSlot("Player1", "", items_handling, {}, version);
            }
        );
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