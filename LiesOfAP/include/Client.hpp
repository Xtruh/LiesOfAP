#pragma once

#include <string>

namespace Client {
	void Connect(const std::string uri, const std::string slotname, const std::string password);
	bool Connected();
	void SendCheck(int64_t id);
	void SendGoal();
	void SendDeath(bool dead);
	void ToggleDeathLink();
	void PollServer();
	void Disconnect();
}