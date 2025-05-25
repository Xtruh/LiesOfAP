#pragma once

#include <string>

namespace Client {
	void Connect(const std::string uri, const std::string slotname, const std::string password);
	void PollServer();
	void Disconnect();
}