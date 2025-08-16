#pragma once

#include <string>

#include "nlohmann/json.hpp"

namespace Client 
{
	void Connect(const std::string uri, const std::string slotname, const std::string password);
	bool Connected();
	void SendCheck(int64_t id);
	void SendGoal();
	void SendDeath(bool dead);
	void SendRingLink();
	void ReciveRingLink(const nlohmann::json& data);
	void ToggleDeathLink();
	void ToggleRingLink();
	void ToggleHardRingLink();
	void SetRingRatio(int ratio);
	void UpdateTags();
	void EchoRatio();
	void Say(const std::string message);
	void PollServer();
	void Disconnect();
}