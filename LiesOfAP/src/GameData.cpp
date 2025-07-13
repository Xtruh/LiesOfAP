#include "GameData.hpp"

#include "Mod/CppUserModBase.hpp"
#include "Unreal/UObjectGlobals.hpp"
#include "Unreal/UFunction.hpp"
#include "Unreal/UScriptStruct.hpp"
#include "Unreal/Property/FStructProperty.hpp"

#include "IDs.hpp"
#include "Client.hpp"

RC::Unreal::FString* RetriveSaveNamePointer()
{
	using namespace RC::Unreal;
	//Get SaveGame
	auto saveGame = UObjectGlobals::FindFirstOf(STR("LCharacterSaveGame"));

	if (!saveGame)
	{
		Output::send(STR("Couldn't get SaveGame Instance!\n"));
		return nullptr;
	}

	auto characterSaveProperty = static_cast<FStructProperty*>(saveGame->GetPropertyByNameInChain(STR("CharacterSaveData")));

	auto characterSavaData = characterSaveProperty->GetStruct();

	auto characterSave = characterSaveProperty->ContainerPtrToValuePtr<void>(saveGame);

	auto saveNameProperty = characterSavaData->GetPropertyByNameInChain(STR("CharacterName"));

	auto saveName = saveNameProperty->ContainerPtrToValuePtr<FString>(characterSave);
	return saveName;
}

std::wstring GameData::GetSaveName()
{
	using namespace RC::Unreal;

	auto saveName = RetriveSaveNamePointer();
	if (!saveName)
		return L"";

	return saveName->GetCharArray();
}

void GameData::SetSaveName(const std::wstring& name)
{
	using namespace RC::Unreal;

	auto saveName = RetriveSaveNamePointer();
	if (!saveName)
		return;

	*saveName = FString(name.c_str());
}

void GameData::CheckItemSpots()
{
	using namespace RC::Unreal;

	std::vector<UObject*> ItemSpots;
	UObjectGlobals::FindAllOf(STR("LPropItemSpot"), ItemSpots);

	if (!ItemSpots.empty())
	{
		// Output::send<LogLevel::Verbose>(STR("Found {} Item Spots\n"), ItemSpots.size());
		for (UObject* ItemSpot : ItemSpots)
		{
			auto state = *ItemSpot->GetValuePtrByPropertyNameInChain<int32>(STR("PropState"));

			if (ItemSpot->GetName().starts_with(STR("LDynamicPropItemSpot")))
				continue;

			//Output::send<LogLevel::Verbose>(STR("{}: {} \n"), ItemSpot->GetName(), state);
			if (state != 2)
				continue;

			auto spotCodename = ItemSpot->GetValuePtrByPropertyNameInChain<FName>(STR("ItemPackageCodeName"));
			if (!spotCodename)
			{
				Output::send<LogLevel::Error>(STR("No Code name found?\n"));
			}

			std::vector<int> locationIds = ID::LOCCODENAME_TO_ID[spotCodename->ToString()];
			for (int id : locationIds)
			{
				Client::SendCheck(id);
				if (id == 29)
				{
					//Client::SendGoal();
				}
			}
		}
	}
}

void GameData::CheckEnemySpots()
{
	using namespace RC::Unreal;

	std::vector<UObject*> NPCSpots;
	UObjectGlobals::FindAllOf(STR("LNPCSpot"), NPCSpots);
	if (!NPCSpots.empty())
	{
		for (UObject* NPCSpot : NPCSpots)
		{
			auto important = *NPCSpot->GetValuePtrByPropertyNameInChain<bool>(STR("bImportantNPC"));
			auto dead = *NPCSpot->GetValuePtrByPropertyNameInChain<bool>(STR("IsDeadState"));

			if (!important || !dead)
				continue;
			// Output::send<LogLevel::Verbose>(STR("{}: IS IMPORTANT AND DEAD \n"), NPCSpot->GetName());

			auto spotCodename = NPCSpot->GetValuePtrByPropertyNameInChain<FName>(STR("SpotCodeName"));
			if (!spotCodename)
			{
				Output::send<LogLevel::Error>(STR("No Code name found?\n"));
			}

			std::vector<int> locationIds = ID::LOCCODENAME_TO_ID[spotCodename->ToString()];
			for (int id : locationIds)
				Client::SendCheck(id);
		}
	}
}

void GameData::CheckQuests()
{
	using namespace RC::Unreal;

	auto QuestSystem = UObjectGlobals::FindFirstOf(STR("LQuestSystem"));
	if (!QuestSystem)
		return;

	auto questCompleteFunction = QuestSystem->GetFunctionByNameInChain(STR("IsCompleteQuest"));
	if (!questCompleteFunction)
		return;

	struct FQuestCompleteParams
	{
		FName CodeName;

		bool Result;
	};

	for (std::wstring questName : ID::QUESTS)
	{
		FQuestCompleteParams params{ FName(questName), false };
		QuestSystem->ProcessEvent(questCompleteFunction, &params);

		if (params.Result)
		{
			//Output::send<LogLevel::Verbose>(STR("{}: Quest Completed \n"), questName);
			std::vector<int> locationIds = ID::LOCCODENAME_TO_ID[questName];
			for (int id : locationIds)
				Client::SendCheck(id);
		}
	}

}

bool GameData::ReceiveItem(int64_t id)
{
	if (id > 600 && id < 700)
	{
		return GiveWeapon(id);
	}

	auto codename = ID::ITEMID_TO_CODENAME[id];

	return GiveItem(codename);
}

bool GameData::GiveItem(const std::wstring& codename)
{
	using namespace RC::Unreal;
	//Get player
	auto player = UObjectGlobals::FindFirstOf(STR("BP_CH_PC_Pino_C"));

	if (!player)
	{
		Output::send(STR("Couldn't get Player Instance!\n"));
		return false;
	}

	UFunction* gainItem = player->GetFunctionByNameInChain(L"OnGainItem");
	if (!gainItem)
	{
		Output::send(STR("Failed to find function OnGainItem\n"));
		return false;
	}

	struct GainItemParams
	{
		FName codename;
		int32 quantity;
	}gainItemParams{ FName(codename), 1 };

	player->ProcessEvent(gainItem, &gainItemParams);
	return true;
}

bool GameData::GiveWeapon(int64_t id)
{
	using namespace RC::Unreal;

	auto handle_codename = ID::ITEMID_TO_CODENAME[id + 100];
	auto blade_codename = ID::ITEMID_TO_CODENAME[id + 200];

	//Get player
	auto player = UObjectGlobals::FindFirstOf(STR("BP_CH_PC_Pino_C"));

	if (!player)
	{
		Output::send(STR("Couldn't get Player Instance!\n"));
		return false;
	}

	UFunction* gainWeapon = player->GetFunctionByNameInChain(L"OnGainWeapon");
	if (!gainWeapon)
	{
		Output::send(STR("Failed to find function OnGainWeapon\n"));
		return false;
	}

	struct GainWeaponParams
	{
		FName handleCodename;
		FName bladeCodename;

		int32 level;
	}gainWeaponParams{ FName(handle_codename), FName(blade_codename), 0 };

	player->ProcessEvent(gainWeapon, &gainWeaponParams);
	return true;
}
