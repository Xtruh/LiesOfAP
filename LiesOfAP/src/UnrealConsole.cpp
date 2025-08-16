#include "UnrealConsole.hpp"

#include "Client.hpp"
#include "GameData.hpp"

#include "DynamicOutput/DynamicOutput.hpp"

#include "StringOps.hpp"

namespace UnrealConsole
{
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
		//this takes user input and splits it up by spaces
		std::wstring input(command);
		std::wstringstream stream(input);
		std::wstring segment;
		std::vector<std::wstring> tokens;

		while (stream >> segment)
		{
			tokens.push_back(segment);
		}

		if (tokens.empty())
		{
			return;
		}

		for (auto& token : tokens)
		{
			RC::Output::send<RC::LogLevel::Verbose>(token);
		}

		if (tokens[0] == STR("connect"))
		{
			if (tokens.size() > 4 || tokens.size() <= 2)
			{
				return;
			}

			if (tokens.size() > 3)
			{
				Client::Connect(StringOps::ws2s(tokens[1]), StringOps::ws2s(tokens[2]), StringOps::ws2s(tokens[3]));
			}
			else
			{
				Client::Connect(StringOps::ws2s(tokens[1]), StringOps::ws2s(tokens[2]), "");
			}
		}
		else if (tokens[0] == STR("ratio"))
		{
			if (tokens.size() > 2)
			{
				return;
			}
			else if (tokens.size() == 1)
			{
				Client::EchoRatio();
			}

			try
			{
				int ratio = std::stoi(tokens[1]);
				ratio = std::clamp(ratio, 1, 100);
				Client::SetRingRatio(ratio);
			}
			catch (const std::exception&)
			{
				return;
			}
		}
		else if (tokens[0] == STR("deathlink"))
		{
			Client::ToggleDeathLink();
		}
		else if (tokens[0] == STR("ringlink"))
		{
			Client::ToggleRingLink();
		}
		else if (tokens[0] == STR("hardringlink"))
		{
			Client::ToggleHardRingLink();
		}
		else if (tokens[0] == STR("disconnect"))
		{
			Client::Disconnect();
		}
	}
}