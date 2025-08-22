#pragma comment (lib, "crypt32")

#define ASIO_STANDALONE

#include <format>
#include <ctime>

#include "Mod/CppUserModBase.hpp"

#include "Client.hpp"

#include "apclient.hpp"
#include "apuuid.hpp"

#include "GameData.hpp"
#include "StringOps.hpp"

APClient* ap = nullptr;
std::string uuid(ap_get_uuid(""));
int source = static_cast<int>(std::time(nullptr));
const std::string game_name("Lies Of P");
const std::string cert_store("ue4ss/Mods/LiesOfAP/dlls/cacert.pem");
bool init = false;
bool dc = false;
int lastReceivedIndex = -1;
bool deathLink = false;
bool ringLink = false;
bool hardRingLink = false;
bool isDead = false;
int ringRatio = 10;
int prevErgo = -1;
int partialRings = 0;
int toAdd = 0;
int goal_id = 740; // Simon Manus Goal
std::set<int64_t> sent_ids;
std::set<int64_t> toResend;


std::wstring EncodeSaveName()
{
	// Format: {Seed},{slot},{lastReceivedIndex}
	std::wstringstream stream;

	if (!ap)
		return L"";

	stream << StringOps::s2ws(ap->get_seed()) << L',' << ap->get_player_number() << L',' << lastReceivedIndex;

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

std::string ProcessMessageText(const APClient::PrintJSONArgs& args)
{
	std::string console_text;

	// This loop is basically the logic of APClient::render_json(), adapted to use RichTextBlock markdown.
	for (const auto& node : args.data)
	{
		if (node.type == "player_id")
		{
			int id = std::stoi(node.text);
			std::string player_name = ap->get_player_alias(id);
			if (id == ap->get_player_number())
				console_text += "<Self>" + player_name + "</>";
			else
				console_text += "<Player>" + player_name + "</>";
		}
		else if (node.type == "item_id")
		{
			int64_t id = std::stoll(node.text);
			std::string item_name = ap->get_item_name(id, ap->get_player_game(node.player));
			if (node.flags & APClient::FLAG_ADVANCEMENT)
				console_text += "<Progression";
			else if (node.flags & APClient::FLAG_NEVER_EXCLUDE)
				console_text += "<Useful";
			else if (node.flags & APClient::FLAG_TRAP)
				console_text += "<Trap";
			else
				console_text += "<Filler";
			console_text += "Item>" + item_name + "</>";
		}
		else if (node.type == "location_id")
		{
			int64_t id = std::stoll(node.text);
			std::string location_name = ap->get_location_name(id, ap->get_player_game(node.player));
			console_text += "<Location>" + location_name + "</>";
		}
		else
			console_text += node.text;
	}

	return console_text;
}

void Client::Connect(const std::string uri, const std::string slotname, const std::string password)
{
	using json = nlohmann::json;
	// Get rid of any exisiting client to account for uri changes
	if (ap) {
		delete ap;
	}
	lastReceivedIndex = -1;
	init = false;
	dc = true;
	deathLink = false;
	ringLink = false;
	hardRingLink = false;
	ringRatio = 10;
	prevErgo = -1;
	partialRings = 0;
	toAdd = 0;
	goal_id = 740; // Simon Manus Goal

	ap = new APClient(uuid, game_name, uri, cert_store);
	ap->set_room_info_handler([slotname, password]()
		{
			int items_handling = 0b111;
			APClient::Version version{ 0, 6, 3 };

			ap->ConnectSlot(slotname, password, items_handling, {}, version);
		});
	ap->set_slot_connected_handler([](const json& slot_data)
		{
			auto saveData = SplitSaveName(GameData::GetSaveName());
			if (saveData.size() == 3)
			{
				if ((StringOps::ws2s(saveData[0]) != ap->get_seed()) || std::stoi(saveData[1]) != ap->get_player_number())
				{
					Output::send(STR("Slot mismatch load correct save"));
					init = true;
					return;
				}

				lastReceivedIndex = std::stoi(saveData[2]);
			}

			dc = false;
			init = true;

			Output::send(STR("index: {}"), lastReceivedIndex);
			GameData::SetSaveName(EncodeSaveName());

			Output::send(STR("{}"), StringOps::s2ws(slot_data.dump()));

			std::list<std::string> tags{};

			for (auto& [key, value] : slot_data.items())
			{
				if (key == "death_link" && value == 1)
				{
					tags.push_back("DeathLink");
					deathLink = true;
				}
				else if (key == "ring_link" && value > 0)
				{
					tags.push_back("RingLink");
					ringLink = true;

					if (value == 2)
					{
						tags.push_back("HardRingLink");
						hardRingLink = true;
					}
				}
				else if (key == "ring_link_ratio")
				{
					ringRatio = value;
				}
				else if (key == "goal")
				{
					if (value == 0) // King of Puppets Goal
					{
						goal_id = 346;
					}
					else if (value == 2)
					{
						goal_id = 742; // Nameless Puppet
					}
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
	ap->set_print_json_handler([](const APClient::PrintJSONArgs& args)
		{
			std::string plain_text = ap->render_json(args.data);
			std::string markdown_text = ProcessMessageText(args);

			Output::send(STR("p: {}\n"), StringOps::s2ws(plain_text));
			Output::send(STR("m: {}\n"), StringOps::s2ws(markdown_text));

			GameData::PrintToConsole(StringOps::s2ws(markdown_text), StringOps::s2ws(plain_text));
		});
	ap->set_bounced_handler([](const json& data)
		{
			Output::send(STR("receiving bounce: {}"), StringOps::s2ws(data.dump()));

			auto tags = data.find("tags"); // This will either be data.end() or an array of tags.
			if (tags == data.end())
			{
				return; // Just ignore non-deathlink bounces.
			}

			bool is_deathlink = std::find(tags->begin(), tags->end(), "DeathLink") != tags->end();
			bool is_ringLink = std::find(tags->begin(), tags->end(), "RingLink") != tags->end();
			bool is_hardRingLink = std::find(tags->begin(), tags->end(), "HardRingLink") != tags->end();

			if (is_deathlink && GameData::IsLoaded())
			{
				isDead = true;
				std::wstring source = StringOps::s2ws(data["data"]["cause"]);
				GameData::PrintToConsole(source, source);
				GameData::ReceiveDeath();
			}
			else if ((is_ringLink || is_hardRingLink) && GameData::IsLoaded())
			{
				Client::ReciveRingLink(data);
			}

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

	if (id == goal_id)
		SendGoal();
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
		Output::send(STR("sending deathlink: {}"), StringOps::s2ws(data.dump()));
	}
}

void Client::SendRingLink()
{
	using json = nlohmann::json;

	if (!ap || !ringLink) {
		return;
	}

	int currentErgo = GameData::GetErgoAmount();

	if (currentErgo == -1)
		return;

	if (prevErgo == -1)
	{
		prevErgo = currentErgo;
		return;
	}

	int change = currentErgo - prevErgo;

	if (change == 0)
		return;

	prevErgo = currentErgo;

	change += partialRings;
	int rings = change / ringRatio;
	partialRings = change % ringRatio;

	if (rings == 0)
		return;

	json data{
			{"time", ap->get_server_time()},
			{"amount", rings},
			{"source", source}
	};

	Output::send(STR("sending ringlink: {}"), StringOps::s2ws(data.dump()));
	ap->Bounce(data, {}, {}, { "RingLink" });
}

void Client::ReciveRingLink(const nlohmann::json& data)
{
	using json = nlohmann::json;

	if (!ap)
		return;

	json details = data["data"];

	int packetSource = details["source"];
	if (packetSource == source)
		return;

	int amount = details["amount"];
	amount *= ringRatio;

	toAdd += amount;

	Output::send(STR("amount: {}\n total: {}"), amount, toAdd);
}

void Client::ToggleDeathLink()
{
	if (!ap) {
		return;
	}

	deathLink = !deathLink;

	UpdateTags();
}

void Client::ToggleRingLink()
{
	if (!ap) {
		return;
	}

	ringLink = !ringLink;

	UpdateTags();
}

void Client::ToggleHardRingLink()
{
	if (!ap) {
		return;
	}

	hardRingLink = !hardRingLink;

	UpdateTags();
}

void Client::SetRingRatio(int ratio)
{
	if (!ap) {
		return;
	}

	ringRatio = ratio;
	partialRings = 0;
	toAdd = 0;

	auto plain = std::format(L"Ring Link ratio set to {}", ringRatio);

	GameData::PrintToConsole(plain);
}

void Client::UpdateTags()
{
	std::list<std::string> tags{};
	if (deathLink)
		tags.push_back("DeathLink");
	if (ringLink)
	{
		tags.push_back("RingLink");
		if (hardRingLink)
			tags.push_back("HardRingLink");
	}

	ap->ConnectUpdate(false, 0, true, tags);
}

void Client::EchoRatio()
{
	auto plain = std::format(L"The current Ring Link Ratio is {}", ringRatio);

	GameData::PrintToConsole(plain);
}

void Client::Say(const std::string message)
{
	if (!ap)
	{
		return;
	}
	ap->Say(message);
}

void Client::PollServer()
{
	if (init && dc)
		Disconnect();

	if (!ap)
	{
		return;
	}

	ap->poll();

	if (toAdd != 0 && GameData::IsLoaded())
	{
		prevErgo = std::max(0, prevErgo + toAdd);
		GameData::SetErgoAmount(toAdd);
		toAdd = 0;
	}

}

void Client::Disconnect()
{
	Output::send(STR("Disconecting"));

	if (!ap) {
		return;
	}
	delete ap;
	ap = nullptr;
}