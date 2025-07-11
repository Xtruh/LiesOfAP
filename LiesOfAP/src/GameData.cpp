#include "GameData.hpp"

#include "Mod/CppUserModBase.hpp"
#include "Unreal/UObjectGlobals.hpp"
#include "Unreal/UFunction.hpp"

#include "IDs.hpp"
#include "Client.hpp"

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

void GameData::ReceiveItem(int64_t id)
{
    auto codename = ID::ITEMID_TO_CODENAME[id];

	GiveItem(codename, 1);
}

bool GameData::GiveItem(const std::wstring& codename, int quantity)
{
    using namespace RC::Unreal;
    //Get player inventory
    auto PlayerInventory = UObjectGlobals::FindFirstOf(STR("LPlayerInventory"));

    if (!PlayerInventory)
    {
        Output::send(STR("Couldn't get PlayerInventory Instance!\n"));
        return false;
    }

    //Get Item system
    auto ItemSystem = UObjectGlobals::FindFirstOf(STR("LItemSystem"));

    if (!ItemSystem)
    {
        Output::send(STR("Couldn't get ItemSystem Instance!\n"));
        return false;
    }

    // Try to find the item in the player inventory
    UFunction* finditem = PlayerInventory->GetFunctionByName(L"FindItemByCodeName");
    if (!finditem)
    {
        Output::send(STR("Failed to find function FindItemByCodeName\n"));
        return false;
    }

    struct FFindItemParams
    {
        FName CodeName;

        UObject* InventoryItem;
    }FindItemParams{ FName(codename), nullptr };

    PlayerInventory->ProcessEvent(finditem, &FindItemParams);

    UObject* InventoryItem = FindItemParams.InventoryItem;

    if (!InventoryItem)
    {
        // create a new item if one isn't in the invetory
        UFunction* createitem = ItemSystem->GetFunctionByName(L"CreateItem");
        if (!createitem)
        {
            Output::send(STR("Failed to find function CreateItem\n"));
            return false;
        }

        struct FCreateItemParams
        {
            FName CodeName;

            UObject* Item;
        };
        FCreateItemParams CreateItemParams;

        CreateItemParams.CodeName = FName(codename);

        ItemSystem->ProcessEvent(createitem, &CreateItemParams);

        UObject* Item = CreateItemParams.Item;
        if (!Item) {

            Output::send(STR("Item could not be create Item Codename likely wrong: {}"), codename);
            return false;
        }
        Output::send(STR("Item created"));
        UFunction* additem = PlayerInventory->GetFunctionByName(L"AddItem");
        if (!additem) {

            Output::send(STR("Couldn't find add item function"));
            return false;
        }
        struct FAddItemParams {
            UObject* Item;
            bool ItemAdded;
        };

        FAddItemParams AddItemParams;
        AddItemParams.Item = Item;

        PlayerInventory->ProcessEvent(additem, &AddItemParams);

        quantity = quantity - 1;
        InventoryItem = Item;
    }
    else {

        Output::send(STR("Item found in inventory"));
    }

    //try to add item to inventory if item is already in the inventory
    int32* ItemProperty = InventoryItem->GetValuePtrByPropertyName<int32>(STR("ItemCount"));
    if (!ItemProperty) {

        Output::send(STR("Couldn't find item property"));
        return false;

    }

    *ItemProperty += quantity;
    return true;
}