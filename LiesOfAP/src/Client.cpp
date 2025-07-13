#pragma comment (lib, "crypt32")

#define ASIO_STANDALONE

#include "Mod/CppUserModBase.hpp"

#include "Client.hpp"

#include "apclient.hpp"
#include "apuuid.hpp"

#include "GameData.hpp"

APClient* ap = nullptr;
std::string uuid(ap_get_uuid(""));
const std::string game_name("Lies Of P");
const std::string cert_store("ue4ss/Mods/LiesOfAP/dlls/cacert.pem");
int lastReceivedIndex = -1;
std::set<int64_t> sent_ids;

namespace Utils
{

	std::wstring s2ws(const std::string& str)
	{
		using convert_typeX = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_typeX, wchar_t> converterX;

		return converterX.from_bytes(str);
	}

	std::string ws2s(const std::wstring& wstr)
	{
		using convert_typeX = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_typeX, wchar_t> converterX;

		return converterX.to_bytes(wstr);
	}
}

std::wstring EncodeSaveName()
{
	// Format: {Seed},{slot},{lastReceivedIndex}
	std::wstringstream stream;

	if (!ap)
		return L"";

	stream << Utils::s2ws(ap->get_seed()) << L',' << ap->get_player_number() << L',' << lastReceivedIndex;

	return stream.str();
}

std::vector<std::wstring> SplitSaveName(const std::wstring& saveName)
{
	std::wstringstream stream(saveName);
	std::wstring segment;
	std::vector<std::wstring> result;

	while (std::getline(stream, segment, L','))
	{
		result.push_back(segment);
	}

	return result;
}

void Client::Connect(const std::string uri, const std::string slotname, const std::string password)
{
	// Get rid of any exisiting client to account for uri changes
	if (ap) {
		delete ap;
	}
	lastReceivedIndex = -1;

	ap = new APClient(uuid, game_name, uri, cert_store);
	ap->set_room_info_handler([slotname, password]()
		{
			int items_handling = 0b111;
			APClient::Version version{ 0, 6, 2 };

			ap->ConnectSlot(slotname, password, items_handling, {}, version);
		});
	ap->set_slot_connected_handler([](const nlohmann::json& slot_data)
		{
			auto saveData = SplitSaveName(GameData::GetSaveName());
			if (saveData.size() == 3)
			{
				if ((Utils::ws2s(saveData[0]) != ap->get_seed()) || std::stoi(saveData[1]) != ap->get_player_number())
				{
					Disconnect();
					Output::send(STR("Seed or Slot mismatch load correct save"));
					return;
				}

				lastReceivedIndex = std::stoi(saveData[2]);
			}

			Output::send(STR("index: {}"), lastReceivedIndex);
			GameData::SetSaveName(EncodeSaveName());
			std::list<std::string> tags{ "DeathLink", "RingLink" };
			ap->ConnectUpdate(false, 0, true, tags);
		});
	ap->set_items_received_handler([](const std::list<APClient::NetworkItem>& items)
		{
			bool indexChanged = false;
			for (const auto& item : items)
			{
				if (item.index <= lastReceivedIndex)
					continue;
				lastReceivedIndex = item.index;
				indexChanged = true;
				bool recived = GameData::ReceiveItem(item.item);
			}

			if (indexChanged)
				GameData::SetSaveName(EncodeSaveName());
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