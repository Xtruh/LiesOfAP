#include "Mod/CppUserModBase.hpp"
#include "Unreal/UObjectGlobals.hpp"
#include "Unreal/AActor.hpp"
#include "Unreal/Hooks.hpp"

#include "Client.hpp"
#include "GameData.hpp"

static void EmptyFunction(RC::Unreal::UnrealScriptFunctionCallableContext& context, void* customdata) {
	// Empty function to provide to RegisterHook.
}

std::string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

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
		using namespace RC::Unreal;

		if (Client::Connected())
		{
			Client::PollServer();
			GameData::CheckItemSpots();
			GameData::CheckEnemySpots();
			GameData::CheckQuests();
			Client::SendDeath(GameData::CheckDeath());
			Client::SendRingLink();
		}

		//int ergo = GameData::GetErgoAmount();
		//Output::send<LogLevel::Verbose>(STR("Ergo: {}\n"), ergo);
	}

	auto on_unreal_init() -> void override
	{
		using namespace RC::Unreal;
		Output::send<LogLevel::Verbose>(STR("Lies Of AP Init\n"));

		// Client::Connect("localhost:38281", "Player1", "");

		Hook::RegisterProcessConsoleExecCallback([&](UObject* object, const TCHAR* command, FOutputDevice& Ar, UObject* executor) -> bool
			{
				//this takes user input and splits it up by spaces
				std::wstring input(command);
				std::wstringstream stream(input);
				std::wstring segment;
				std::vector<std::wstring> tokens;

				while (stream >> segment)
				{
					tokens.push_back(segment);
				}

				if (tokens.empty())
				{
					return false;
				}

				for (auto& token : tokens)
				{
					Output::send<LogLevel::Verbose>(token);
				}

				if (!tokens[0].starts_with(STR("/")))
				{
					return false;
				}

				if (tokens[0] == STR("/connect"))
				{
					if (tokens.size() > 4 || tokens.size() <= 2)
					{
						return false;
					}

					if (tokens.size() > 3)
					{
						Client::Connect(ws2s(tokens[1]), ws2s(tokens[2]), ws2s(tokens[3]));
					}
					else
					{
						Client::Connect(ws2s(tokens[1]), ws2s(tokens[2]), "");
					}
				}
				else if (tokens[0] == STR("/deathlink"))
				{
					Client::ToggleDeathLink();
				}


				return true;
			});
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