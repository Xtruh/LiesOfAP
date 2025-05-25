#pragma comment (lib, "crypt32")

#define ASIO_STANDALONE

#include "Client.hpp"

#include "apclient.hpp"
#include "apuuid.hpp"

APClient* ap = nullptr;
std::string uuid(ap_get_uuid(""));
const std::string game_name("Clique"); //TODO Change to "Lies Of P" when apworld is working
const std::string cert_store("Mods/LiesOfAP/dlls/cacert.pem");

void Client::Connect(const std::string uri, const std::string slotname, const std::string password)
{
	// Get rid of any exisiting client to account for uri changes
	if (ap) {
		delete ap;
	}

	ap = new APClient(uuid, game_name, uri);
	ap->set_room_info_handler([slotname, password]()
		{
			int items_handling = 0b111;
			APClient::Version version{ 0, 6, 2 };
			ap->ConnectSlot(slotname, password, items_handling, {}, version);
		}
	);
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