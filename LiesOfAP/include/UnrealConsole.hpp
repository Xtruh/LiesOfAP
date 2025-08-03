#pragma once
#include "Unreal/FText.hpp"

namespace UnrealConsole
{
	void ProcessInput(RC::Unreal::FText input);
	void ProcessCommand(std::wstring command);
}