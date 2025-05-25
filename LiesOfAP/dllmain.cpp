#pragma comment (lib, "crypt32")

#define ASIO_STANDALONE

#include "Mod/CppUserModBase.hpp"

#include "Client.hpp"

class LiesOfAP : public RC::CppUserModBase
{
public:

    LiesOfAP() : CppUserModBase()
    {
        ModName = STR("LiesOfAP");
        ModVersion = STR("1.0");
        ModDescription = STR("A Lies Of P Randomizer made for the Archipelago multiworld randomizer");
        ModAuthors = STR("Ninjakakes, Xtruh");
        // Do not change this unless you want to target a UE4SS version
        // other than the one you're currently building with somehow.
        //ModIntendedSDKVersion = STR("2.6");
        
    }

    ~LiesOfAP() override
    {
        Client::Disconnect();
    }

    auto on_update() -> void override
    {
        Client::PollServer();
    }

    auto on_unreal_init() -> void override
    {
        Client::Connect("localhost:38281", "Player1", "");
    }
};

extern "C"
{
    __declspec(dllexport) RC::CppUserModBase* start_mod()
    {
        return new LiesOfAP();
    }

    __declspec(dllexport) void uninstall_mod(RC::CppUserModBase* mod)
    {
        delete mod;
    }
}