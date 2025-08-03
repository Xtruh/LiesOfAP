#include "UnrealConsole.hpp"

#include "Client.hpp"

#include "DynamicOutput/DynamicOutput.hpp"

#include "StringOps.hpp"

namespace UnrealConsole
{
	void ProcessInput(RC::Unreal::FText input)
	{
		std::wstring command = input.ToString();
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
		else if (tokens[0] == STR("deathlink"))
		{
			Client::ToggleDeathLink();
		}
	}
}