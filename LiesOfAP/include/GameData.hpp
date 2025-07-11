#pragma once

#include <string>
namespace GameData
{
	void CheckItemSpots();
	void CheckEnemySpots();
	void CheckQuests();
	void ReceiveItem(int64_t id);
	bool GiveItem(const std::wstring& codename, int quantity);
}