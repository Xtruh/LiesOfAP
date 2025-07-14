#pragma once

#include <string>
namespace GameData
{
	std::wstring GetSaveName();
	void SetSaveName(const std::wstring& name);
	void CheckItemSpots();
	void CheckEnemySpots();
	void CheckQuests();
	bool CheckDeath();
	bool ReceiveItem(int64_t id);
	bool GiveItem(const std::wstring& codename);
	bool GiveWeapon(int64_t id);
	void ReceiveDeath();
}