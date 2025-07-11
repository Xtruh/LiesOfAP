#pragma comment (lib, "crypt32")

#define ASIO_STANDALONE

#include "Client.hpp"

#include "apclient.hpp"
#include "apuuid.hpp"

#include "GameData.hpp"

APClient* ap = nullptr;
std::string uuid(ap_get_uuid(""));
const std::string game_name("Lies Of P"); //TODO Change to "Lies Of P" when apworld is working
const std::string cert_store("ue4ss/Mods/LiesOfAP/dlls/cacert.pem");
int lastReceivedIndex = -1;
std::set<int64_t> sent_ids;

void Client::Connect(const std::string uri, const std::string slotname, const std::string password)
{
	// Get rid of any exisiting client to account for uri changes
	if (ap) {
		delete ap;
	}

	ap = new APClient(uuid, game_name, uri, cert_store);
	ap->set_room_info_handler([slotname, password]()
		{
			int items_handling = 0b111;
			APClient::Version version{ 0, 6, 2 };
			ap->ConnectSlot(slotname, password, items_handling, {}, version);
		});
	ap->set_slot_connected_handler([](const nlohmann::json& slot_data)
		{
			std::list<std::string> tags{"DeathLink", "RingLink"};
			ap->ConnectUpdate(false, 0, true, tags);
		});
	ap->set_items_received_handler([](const std::list<APClient::NetworkItem>& items)
		{
			for (const auto& item : items)
			{
				if (item.index <= lastReceivedIndex)
					continue;
				lastReceivedIndex = item.index;
				GameData::ReceiveItem(item.item);
			}
		});
}

bool Client::Connected()
{
	return ap;
}

void Client::SendCheck(int64_t id)
{
	if (!ap) {
		return;
	}

	if (sent_ids.contains(id))
		return;

	sent_ids.insert(id);

	std::list<int64_t> id_list{ id };
	ap->LocationChecks(id_list);
}

void Client::SendGoal()
{
	if (!ap) {
		return;
	}

	ap->StatusUpdate(APClient::ClientStatus::GOAL);
}

void Client::PollServer()
{
	if (!ap) {
		return;
	}
	ap->poll();
}

void Client::Disconnect()
{
	if (!ap) {
		return;
	}
	delete ap;
}