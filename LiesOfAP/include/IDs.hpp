#pragma once

#include <map>
#include <string>
#include <vector>
#include <set>

namespace ID
{

	extern std::map<int, std::wstring> ITEMID_TO_CODENAME;

	extern std::map<std::wstring, std::vector<int>> LOCCODENAME_TO_ID;

	extern std::vector<std::wstring> QUESTS;

	void Init();
}