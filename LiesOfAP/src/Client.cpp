#pragma comment (lib, "crypt32")

#define ASIO_STANDALONE

#include <format>

#include "Mod/CppUserModBase.hpp"

#include "Client.hpp"

#include "apclient.hpp"
#include "apuuid.hpp"

#include "GameData.hpp"

APClient* ap = nullptr;
std::string uuid(ap_get_uuid(""));
const std::string game_name("Lies Of P");
const std::string cert_store("ue4ss/Mods/LiesOfAP/dlls/cacert.pem");
bool dc = false;
int lastReceivedIndex = -1;
bool deathLink = false;
bool isDead = false;
std::set<int64_t> sent_ids;
std::set<int64_t> toResend;

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
	using json = nlohmann::json;
	// Get rid of any exisiting client to account for uri changes
	if (ap) {
		delete ap;
	}
	lastReceivedIndex = -1;
	dc = true;
	deathLink = false;

	ap = new APClient(uuid, game_name, uri, cert_store);
	ap->set_room_info_handler([slotname, password]()
		{
			int items_handling = 0b111;
			APClient::Version version{ 0, 6, 2 };

			ap->ConnectSlot(slotname, password, items_handling, {}, version);
		});
	ap->set_slot_connected_handler([](const json& slot_data)
		{
			auto saveData = SplitSaveName(GameData::GetSaveName());
			if (saveData.size() == 3)
			{
				if ((Utils::ws2s(saveData[0]) != ap->get_seed()) || std::stoi(saveData[1]) != ap->get_player_number())
				{
					Output::send(STR("Slot mismatch load correct save"));
					return;
				}

				lastReceivedIndex = std::stoi(saveData[2]);
			}

			dc = false;

			Output::send(STR("index: {}"), lastReceivedIndex);
			GameData::SetSaveName(EncodeSaveName());

			Output::send(STR("{}"), Utils::s2ws(slot_data.dump()));

			std::list<std::string> tags{};

			for (auto& [key, value] : slot_data.items())
			{
				if (key == "death_link" && value == 1)
				{
					tags.push_back("DeathLink");
					deathLink = true;
				}
			}

			ap->ConnectUpdate(false, 0, true, tags);
		});
	ap->set_items_received_handler([](const std::list<APClient::NetworkItem>& items)
		{
			if (dc)
				return;

			bool indexChanged = false;
			for (const auto& item : items)
			{
				if (item.index <= lastReceivedIndex)
					continue;
				bool recived = GameData::ReceiveItem(item.item);
				if (recived)
				{
					lastReceivedIndex = item.index;
					indexChanged = true;
				}
				else
				{
					toResend.insert(item.index);
				}
				
			}

			if (indexChanged)
				GameData::SetSaveName(EncodeSaveName());
		});
	ap->set_bounced_handler([](const json& data)
		{
			Output::send(STR("receiving bounce: {}"), Utils::s2ws(data.dump()));

			auto tags = data.find("tags"); // This will either be data.end() or an array of tags.
			if (tags == data.end()) 
			{
				return; // Just ignore non-deathlink bounces.
			}

			bool is_deathlink = std::find(tags->begin(), tags->end(), "DeathLink") != tags->end();
			if (is_deathlink)
				GameData::ReceiveDeath();

		});
}

bool Client::Connected()
{
	return ap;
}

void Client::SendCheck(int64_t id)
{
	if (!ap || dc) {
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
	if (!ap || dc) {
		return;
	}

	ap->StatusUpdate(APClient::ClientStatus::GOAL);
}

void Client::SendDeath(bool dead)
{
	using json = nlohmann::json;

	if (!ap || !deathLink) {
		return;
	}

	if (isDead)
	{
		if (!dead)
			isDead = false;

		return;
	}

	if (dead)
	{
		isDead = true;
		std::string message = std::vformat("{} didn't lie", std::make_format_args(ap->get_slot()));

		json data{
			{"time", ap->get_server_time()},
			{"cause", message},
			{"source", ap->get_slot()}
		};

		ap->Bounce(data, {}, {}, { "DeathLink" });
		Output::send(STR("sending deathlink: {}"), Utils::s2ws(data.dump()));
	}
}

void Client::ToggleDeathLink()
{
	if (!ap) {
		return;
	}

	std::list<std::string> tags{};
	deathLink = !deathLink;

	if (deathLink)
		tags.push_back("DeathLink");

	ap->ConnectUpdate(false, 0, true, tags);
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