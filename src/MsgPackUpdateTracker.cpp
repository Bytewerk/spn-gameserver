#include <arpa/inet.h>

#include "Bot.h"
#include "Food.h"

#include "config.h"

#include "MsgPackUpdateTracker.h"

/* Private methods */

void MsgPackUpdateTracker::appendMessage(const msgpack::sbuffer &buf)
{
	uint32_t length = htonl(static_cast<uint32_t>(buf.size()));

	m_stream.write(reinterpret_cast<char*>(&length), sizeof(length));
	m_stream.write(buf.data(), buf.size());
}

/* Public methods */

MsgPackUpdateTracker::MsgPackUpdateTracker()
{
	reset();
}

void MsgPackUpdateTracker::foodConsumed(
		const std::shared_ptr<Food> &food,
		const std::shared_ptr<Bot> &by_bot)
{
	MsgPackProtocol::FoodConsumeItem item;

	item.bot_id = by_bot->getGUID();
	item.food_id = food->getGUID();

	m_foodConsumeMessage->items.push_back(item);
}

void MsgPackUpdateTracker::foodDecayed(const std::shared_ptr<Food> &food)
{
	m_foodDecayMessage->food_ids.push_back(food->getGUID());
}

void MsgPackUpdateTracker::foodSpawned(const std::shared_ptr<Food> &food)
{
	m_foodSpawnMessage->new_food.push_back(food);
}

void MsgPackUpdateTracker::botSpawned(const std::shared_ptr<Bot> &bot)
{
	MsgPackProtocol::BotSpawnMessage msg;
	msg.bot = bot;

	msgpack::sbuffer buf;
	msgpack::pack(buf, msg);
	appendMessage(buf);
}

void MsgPackUpdateTracker::botKilled(
		const std::shared_ptr<Bot> &killer,
		const std::shared_ptr<Bot> &victim)
{
	MsgPackProtocol::BotKillMessage msg;
	msg.killer_id = killer->getGUID();
	msg.victim_id = victim->getGUID();

	msgpack::sbuffer buf;
	msgpack::pack(buf, msg);
	appendMessage(buf);
}

void MsgPackUpdateTracker::botMoved(const std::shared_ptr<Bot> &bot, std::size_t steps)
{
	MsgPackProtocol::BotMoveItem item;

	const Snake::SegmentList &segments = bot->getSnake()->getSegments();

	item.bot_id = bot->getGUID();
	item.new_segments.assign(segments.begin(), segments.begin() + steps);
	item.current_segment_radius = bot->getSnake()->getSegmentRadius();
	item.current_length = segments.size();

	m_botMoveMessage->items.push_back(item);
}

void MsgPackUpdateTracker::gameInfo(void)
{
	MsgPackProtocol::GameInfoMessage msg;

	msg.world_size_x = config::FIELD_SIZE_X;
	msg.world_size_y = config::FIELD_SIZE_Y;
	msg.food_decay_per_frame = config::FOOD_DECAY_STEP;

	msgpack::sbuffer buf;
	msgpack::pack(buf, msg);
	appendMessage(buf);
}

void MsgPackUpdateTracker::worldState(const std::shared_ptr<Field> &field)
{
	MsgPackProtocol::WorldUpdateMessage msg
	{
		field->getBots(),
		field->getFoodInfoMap()
	};

	msgpack::sbuffer buf;
	msgpack::pack(buf, msg);
	appendMessage(buf);
}

std::string MsgPackUpdateTracker::serialize(void)
{
	// decayed food
	if(!m_foodDecayMessage->food_ids.empty()) {
		msgpack::sbuffer buf;
		msgpack::pack(buf, m_foodDecayMessage);
		appendMessage(buf);
	}

	// spawned food
	if(!m_foodSpawnMessage->new_food.empty()) {
		msgpack::sbuffer buf;
		msgpack::pack(buf, m_foodSpawnMessage);
		appendMessage(buf);
	}

	// consumed food
	if(!m_foodConsumeMessage->items.empty()) {
		msgpack::sbuffer buf;
		msgpack::pack(buf, m_foodConsumeMessage);
		appendMessage(buf);
	}

	// moved bots
	if(!m_botMoveMessage->items.empty()) {
		msgpack::sbuffer buf;
		msgpack::pack(buf, m_botMoveMessage);
		appendMessage(buf);
	}

	std::string result = m_stream.str();
	reset();
	return result;
}

void MsgPackUpdateTracker::reset(void)
{
	m_foodConsumeMessage = std::make_unique<MsgPackProtocol::FoodConsumeMessage>();
	m_foodSpawnMessage = std::make_unique<MsgPackProtocol::FoodSpawnMessage>();
	m_foodDecayMessage = std::make_unique<MsgPackProtocol::FoodDecayMessage>();
	m_botMoveMessage = std::make_unique<MsgPackProtocol::BotMoveMessage>();

	m_stream.str("");
}

