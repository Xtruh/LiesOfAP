#pragma once

#include <string>

#include "Unreal/UObject.hpp"
namespace GameData
{
	bool IsLoaded();
	std::wstring GetCurrentMap(RC::Unreal::UObject* object);
	std::wstring GetSaveName();
	void SetSaveName(const std::wstring& name);
	void SaveGame();
	int GetErgoAmount();
	void SetErgoAmount(int amount);
	void CheckItemSpots();
	void CheckEnemySpots();
	void CheckQuests();
	bool CheckDeath();
	bool CheckDLC();
	bool ReceiveItem(int64_t id);
	bool GiveItem(const std::wstring& codename);
	bool GiveWeapon(int64_t id);
	void ReceiveDeath();
	void PrintToConsole(const std::wstring& markdown_text, const std::wstring& plain_text);
	void PrintToConsole(const std::wstring& text);
}