#include "UnrealConsole.hpp"

#include "Client.hpp"
#include "GameData.hpp"

#include "DynamicOutput/DynamicOutput.hpp"

#include "StringOps.hpp"

namespace UnrealConsole
{
	const wchar_t DELIM = L' ';

	std::wstring GetNextToken(std::wstring& input)
	{
		// Gets the next space-separated word and removes it from the input string.
		while (input[0] == DELIM)
			input.erase(input.begin());

		std::wstring token;
		if (input[0] == L'"')
		{
			// look for second double quote and take everything between them as one token
			token = input.substr(1, input.find(L'"', 1) - 1);
			input.erase(0, input.find(L'"', 1) + 1);
			return token;
		}
		token = input.substr(0, input.find(DELIM));
		input.erase(0, input.find(DELIM));
		return token;

	}

	void ProcessInput(RC::Unreal::FText input)
	{
		std::wstring command = input.ToString();

		if (!GameData::IsLoaded())
		{
			auto markdown = L"<System>Please load a save file before attemting to use the text client</>";
			auto plain = L"Please load a save file before attempting to use the text client";

			GameData::PrintToConsole(markdown, plain);

			return;
		}

		if (command[0] == *L"/")
		{
			command.erase(0, 1);
			UnrealConsole::ProcessCommand(command);
		}
		else
		{
			Client::Say(StringOps::ws2s(command));
		}
	}

	void ProcessCommand(std::wstring command)
	{
		auto commandText = GetNextToken(command);

		RC::Output::send<RC::LogLevel::Verbose>(commandText);

		if (commandText == STR("connect"))
		{
			auto ip = GetNextToken(command);
			if (ip.empty())
			{
				GameData::PrintToConsole(L"Please provide an ip address, slot name, and (if necessary) password.");
				return;
			}

			auto slotname = GetNextToken(command);
			if (slotname.empty())
			{
				GameData::PrintToConsole(L"Please provide an ip address, slot name, and (if necessary) password.");
				return;
			}

			auto password = GetNextToken(command);
			Client::Connect(StringOps::ws2s(ip), StringOps::ws2s(slotname), StringOps::ws2s(password));
		}
		else if (commandText == STR("ratio"))
		{
			auto ratioText = GetNextToken(command);

			if (ratioText.empty())
			{
				Client::EchoRatio();
				return;
			}

			try
			{
				int ratio = std::stoi(ratioText);
				ratio = std::clamp(ratio, 1, 100);
				Client::SetRingRatio(ratio);
			}
			catch (const std::exception&)
			{
				return;
			}
		}
		else if (commandText == STR("deathlink"))
		{
			Client::ToggleDeathLink();
		}
		else if (commandText == STR("ringlink"))
		{
			Client::ToggleRingLink();
		}
		else if (commandText == STR("hardringlink"))
		{
			Client::ToggleHardRingLink();
		}
		else if (commandText == STR("disconnect"))
		{
			Client::Disconnect();
		}
	}
}