#pragma once

#include <string>
namespace GameData
{
	bool IsLoaded();
	std::wstring GetSaveName();
	void SetSaveName(const std::wstring& name);
	int GetErgoAmount();
	void SetErgoAmount(int amount);
	void CheckItemSpots();
	void CheckEnemySpots();
	void CheckQuests();
	bool CheckDeath();
	bool ReceiveItem(int64_t id);
	bool GiveItem(const std::wstring& codename);
	bool GiveWeapon(int64_t id);
	void ReceiveDeath();
	void PrintToConsole(const std::wstring& markdown_text, const std::wstring& plain_text);
}