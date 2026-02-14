#include "GameData.hpp"

#include "Mod/CppUserModBase.hpp"
#include "Unreal/UObjectGlobals.hpp"
#include "Unreal/UFunction.hpp"
#include "Unreal/UScriptStruct.hpp"
#include "Unreal/Property/FStructProperty.hpp"
#include "Unreal/FText.hpp"
#include "Unreal/World.hpp"
#include "Unreal/UClass.hpp"

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

	if (!characterSaveProperty)
	{
		Output::send(STR("Couldn't get CharacterSaveData Prop!\n"));
		return nullptr;
	}

	auto characterSaveData = characterSaveProperty->GetStruct();

	if (!characterSaveData)
	{
		Output::send(STR("Couldn't get CharacterSaveData!\n"));
		return nullptr;
	}

	auto characterSave = characterSaveProperty->ContainerPtrToValuePtr<void>(saveGame);

	if (!characterSave)
	{
		Output::send(STR("Couldn't get Character Save Instance!\n"));
		return nullptr;
	}

	auto saveNameProperty = characterSaveData->GetPropertyByNameInChain(STR("CharacterName"));

	if (!saveNameProperty)
	{
		Output::send(STR("Couldn't get SaveName Prop!\n"));
		return nullptr;
	}

	auto saveName = saveNameProperty->ContainerPtrToValuePtr<FString>(characterSave);

	if (!saveName)
	{
		Output::send(STR("Couldn't get SaveName!\n"));
		return nullptr;
	}

	return saveName;
}

RC::Unreal::int32* RetriveErgoAmountPointer()
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

	if (!characterSaveProperty)
	{
		Output::send(STR("Couldn't get CharacterSaveData Prop!\n"));
		return nullptr;
	}

	auto characterSaveData = characterSaveProperty->GetStruct();

	if (!characterSaveData)
	{
		Output::send(STR("Couldn't get CharacterSaveData!\n"));
		return nullptr;
	}

	auto characterSave = characterSaveProperty->ContainerPtrToValuePtr<void>(saveGame);

	if (!characterSave)
	{
		Output::send(STR("Couldn't get Character Save Instance!\n"));
		return nullptr;
	}

	auto ergoAmountProperty = characterSaveData->GetPropertyByNameInChain(STR("AcquisitionSoul"));

	if (!ergoAmountProperty)
	{
		Output::send(STR("Couldn't get Ergo Amount Prop!\n"));
		return nullptr;
	}

	auto ergoAmount = ergoAmountProperty->ContainerPtrToValuePtr<int32>(characterSave);

	if (!ergoAmount)
	{
		Output::send(STR("Couldn't get Ergo Amount!\n"));
		return nullptr;
	}

	return ergoAmount;
}

bool GameData::IsLoaded()
{
	using namespace RC::Unreal;

	auto loadingHandler = UObjectGlobals::FindFirstOf(STR("LSoundSystem"));

	if (!loadingHandler)
	{
		Output::send(STR("Couldn't get SoundSystem Instance!\n"));
		return false;
	}

	UFunction* loadingFunction = loadingHandler->GetFunctionByNameInChain(L"GetIsAsncLoadingMapCompleted");
	if (!loadingFunction)
	{
		Output::send(STR("Failed to find function GetIsAsncLoadingMapCompleted\n"));
		return false;
	}

	bool load = false;

	loadingHandler->ProcessEvent(loadingFunction, &load);

	return load;

}

std::wstring GameData::GetCurrentMap(RC::Unreal::UObject* object)
{
	std::wstring current_map = object->GetWorld()->GetName();
	return current_map;
}

std::wstring GameData::GetSaveName()
{
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

void GameData::SaveGame()
{
	using namespace RC::Unreal;
	//Get GameDataSystem
	auto dataSystem = UObjectGlobals::FindFirstOf(STR("LGameDataSystem"));

	if (!dataSystem)
	{
		Output::send(STR("Couldn't get GameDataSystem Instance!\n"));
		return;
	}

	UFunction* saveGame = dataSystem->GetFunctionByNameInChain(L"SaveGameDataFromUI");
	if (!saveGame)
	{
		Output::send(STR("Failed to find function SaveGameDataFromUI\n"));
		return;
	}

	struct SaveGameParams
	{
		bool async;
		bool normal;
	}saveGameParams{ true, true };

	dataSystem->ProcessEvent(saveGame, &saveGameParams);
}

int GameData::GetErgoAmount()
{
	auto ergoAmount = RetriveErgoAmountPointer();
	if (!ergoAmount)
		return -1;

	return *ergoAmount;
}

void GameData::SetErgoAmount(int amount)
{
	using namespace RC::Unreal;
	//Get player
	std::vector<UObject*> players;
	UObjectGlobals::FindAllOf(STR("BP_CH_PC_Pino_C"), players);
	UObject* player = nullptr;

	for (auto p : players)
	{
		if (p->GetName().starts_with(STR("BP_CH_PC_Pino_C")))
		{
			player = p;
			break;
		}
	}

	if (!player)
	{
		Output::send(STR("Couldn't get Player Instance!\n"));
		return;
	}

	UFunction* gainErgo = nullptr; // = player->GetFunctionByNameInChain(L"OnGainExp");

	std::vector<UFunction*> functions;

	for (UFunction* Function : player->GetClassPrivate()->ForEachFunctionInChain())
	{
		if (Function->GetNamePrivate() == FName(L"OnGainExp"))
		{
			Output::send(L"OnGainExp");
			functions.push_back(Function);
		}
	}

	if (functions.size() == 0)
	{
		Output::send(STR("Failed to find function OnGainExp\n"));
		return;
	}

	gainErgo = functions[1];

	if (amount > 0)
	{
		Unreal::int32 inExp = amount;
		player->ProcessEvent(gainErgo, &inExp);
	}
	else if (amount < 0)
	{
		auto ergoAmount = RetriveErgoAmountPointer();
		if (!ergoAmount)
			return;

		int total = *ergoAmount + amount;

		total = std::max(-1, total - 1);

		*ergoAmount = total;
		Unreal::int32 one = 1;
		player->ProcessEvent(gainErgo, &one);
	}
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
			auto statePtr = ItemSpot->GetValuePtrByPropertyNameInChain<int32>(STR("PropState"));
			if (!statePtr)
				return;
			auto state = *statePtr;

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
			auto deadPtr = NPCSpot->GetValuePtrByPropertyNameInChain<bool>(STR("IsDeadState"));
			if (!deadPtr)
				return;
			auto dead = *deadPtr;

			if (!dead)
				continue;

			auto spotCodename = NPCSpot->GetValuePtrByPropertyNameInChain<FString>(STR("SpotUniqueID"));
			if (!spotCodename)
			{
				Output::send<LogLevel::Error>(STR("No Code name found?\n"));
			}

			//if (spotCodename->ToString().starts_with(L"CH08_Puppet_Tomorrow_Electronic_Named_00"))
			//{
			//	std::vector<int> locationIds;
			//	auto id = NPCSpot->GetValuePtrByPropertyNameInChain<int>(STR("InstanceId"));
			//	if (!id)
			//		continue;
			//	if (*id == 33) // First Puppet of the Future
			//		locationIds = ID::LOCCODENAME_TO_ID[L"CH08_Puppet_Tomorrow_Electronic_Named_00_1"];
			//	else // Second POTF
			//		locationIds = ID::LOCCODENAME_TO_ID[L"CH08_Puppet_Tomorrow_Electronic_Named_00_2"];

			//	for (int id : locationIds)
			//		Client::SendCheck(id);
			//}

			if (!ID::LOCCODENAME_TO_ID.contains(spotCodename->GetCharArray()))
				continue;

			//Output::send<LogLevel::Verbose>(STR("{}: ISDEAD||CODENAME: {}\n"), NPCSpot->GetName(), spotCodename->ToString());

			std::vector<int> locationIds = ID::LOCCODENAME_TO_ID[spotCodename->GetCharArray()];
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

bool GameData::CheckDeath()
{
	using namespace RC::Unreal;
	//Get player
	std::vector<UObject*> players;
	UObjectGlobals::FindAllOf(STR("BP_CH_PC_Pino_C"), players);
	UObject* player = nullptr;

	for (auto p : players)
	{
		if (p->GetName().starts_with(STR("BP_CH_PC_Pino_C")))
		{
			player = p;
			break;
		}
	}

	if (!player)
	{
		Output::send(STR("Couldn't get Player Instance!\n"));
		return false;
	}

	UFunction* isDeadFunction = player->GetFunctionByNameInChain(L"IsDead");
	if (!isDeadFunction)
	{
		Output::send(STR("Failed to find function IsDead\n"));
		return false;
	}

	bool isDead = false;

	player->ProcessEvent(isDeadFunction, &isDead);

	return isDead;
}

bool GameData::CheckDLC()
{
	//Any failure will assume the dlc is enabled if this breaks things my bad

	using namespace RC::Unreal;

	//Get SaveGame
	auto saveGame = UObjectGlobals::FindFirstOf(STR("LAccountSaveGame"));

	if (!saveGame)
	{
		Output::send(STR("Couldn't get SaveGame Instance!\n"));
		return true;
	}

	auto dlcSaveProperty = static_cast<FStructProperty*>(saveGame->GetPropertyByNameInChain(STR("DLCSaveData_Account")));

	if (!dlcSaveProperty)
	{
		Output::send(STR("Couldn't get DLCSaveData Prop!\n"));
		return true;
	}

	auto dlcSaveData = dlcSaveProperty->GetStruct();

	if (!dlcSaveData)
	{
		Output::send(STR("Couldn't get DLCSaveData!\n"));
		return true;
	}

	auto dlcSave = dlcSaveProperty->ContainerPtrToValuePtr<void>(saveGame);

	if (!dlcSave)
	{
		Output::send(STR("Couldn't get DLC Save Instance!\n"));
		return true;
	}

	auto appliedDLCsProperty = dlcSaveData->GetPropertyByNameInChain(STR("AppliedDLCs"));

	if (!appliedDLCsProperty)
	{
		Output::send(STR("Couldn't get AppliedDLCs Prop!\n"));
		return true;
	}

	auto appliedDLCs = appliedDLCsProperty->ContainerPtrToValuePtr<TMap<FString, FName>>(dlcSave);

	if (!appliedDLCs)
	{
		Output::send(STR("Couldn't get AppliedDLCs!\n"));
		return true;
	}

	return appliedDLCs->Contains(FString(L"2848330"));
}

bool GameData::ReceiveItem(int64_t id)
{
	if ((id > 600 && id < 700) || (id > 2100 && id < 2200))
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
	std::vector<UObject*> players;
	UObjectGlobals::FindAllOf(STR("BP_CH_PC_Pino_C"), players);
	UObject* player = nullptr;

	for (auto p : players)
	{
		if (p->GetName().starts_with(STR("BP_CH_PC_Pino_C")))
		{
			player = p;
			break;
		}
	}

	if (!player)
	{
		Output::send(STR("Couldn't get Player Instance!\n"));
		return false;
	}

	Output::send(STR("Player Name: {}"), player->GetName());

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
	std::vector<UObject*> players;
	UObjectGlobals::FindAllOf(STR("BP_CH_PC_Pino_C"), players);
	UObject* player = nullptr;

	for (auto p : players)
	{
		if (p->GetName().starts_with(STR("BP_CH_PC_Pino_C")))
		{
			player = p;
			break;
		}
	}

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

void GameData::ReceiveDeath()
{
	using namespace RC::Unreal;

	//Get player
	std::vector<UObject*> players;
	UObjectGlobals::FindAllOf(STR("BP_CH_PC_Pino_C"), players);
	UObject* player = nullptr;

	for (auto p : players)
	{
		if (p->GetName().starts_with(STR("BP_CH_PC_Pino_C")))
		{
			player = p;
			break;
		}
	}

	if (!player)
	{
		Output::send(STR("Couldn't get Player Instance!\n"));
		return;
	}

	UFunction* receiveDamage = player->GetFunctionByNameInChain(L"ReceiveDamage");
	if (!receiveDamage)
	{
		Output::send(STR("Failed to find function OnGainWeapon\n"));
		return;
	}

	struct ReceiveDamageParams
	{
		struct Input
		{
			int64 HitProcContext;
			int32 Damage;
			int64 Attacker;
			FName SkillHitCodeName;
			int64 HitComponent;
			TMap<uint8, float> PhysicalDamages;
			TMap<uint8, float> ElementDamages;
			uint8 bIsGuarding;
			uint8 bSkipUIUpdate;
			uint8 bIsFallingDamage;
			uint8 bRegainBlock;
		}Input;

		struct Output
		{
			int32 OutDamage;
			FName HitPartsName;
			bool bHitPartsDestoryed;
			int32 DamageCurrentHP;
		}Output;

	}receiveDamageParams{ {0,5000,0,FName(),0,TMap<uint8,float>(),TMap<uint8,float>(),0,0,0,0},{0,FName(),false,0} };

	player->ProcessEvent(receiveDamage, &receiveDamageParams);
}

void GameData::PrintToConsole(const std::wstring& markdown_text, const std::wstring& plain_text)
{
	using namespace RC::Unreal;

	std::vector<UObject*> consoles;
	UObjectGlobals::FindAllOf(STR("AP_Console_C"), consoles);

	for (auto c : consoles)
	{
		if (!c->GetName().starts_with(STR("AP_Console_C_")))
		{
			continue;
		}

		UFunction* addMessageFunction = c->GetFunctionByNameInChain(L"AP_PrintToConsole");
		if (!addMessageFunction)
		{
			Output::send(STR("Failed to find function AP_PrintToConsole\n"));
			return;
		}

		struct MessageParams
		{
			FText markdown;
			FText plain;
		}messageParams{ FText(markdown_text), FText(plain_text) };

		c->ProcessEvent(addMessageFunction, &messageParams);
	}
}

void GameData::PrintToConsole(const std::wstring& text)
{
	auto markdown = std::format(L"<System>{}</>", text);

	PrintToConsole(markdown, text);
}
