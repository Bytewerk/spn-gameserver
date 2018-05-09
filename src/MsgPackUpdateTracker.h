#pragma once

#include <sstream>

#include <msgpack.hpp>

#include "MsgPackProtocol.h"

#include "types.h"
#include "UpdateTracker.h"

/*!
 * \brief Implementation of UpdateTracker which serializes the events using
 * MsgPack.
 */
class MsgPackUpdateTracker : public UpdateTracker
{
	private:
		// messages that need to be filled over a frame
		MsgPackProtocol::FoodConsumeMessage m_foodConsumeMessage;
		MsgPackProtocol::FoodSpawnMessage m_foodSpawnMessage;
		MsgPackProtocol::FoodDecayMessage m_foodDecayMessage;
		MsgPackProtocol::BotMoveMessage m_botMoveMessage;
		MsgPackProtocol::BotMoveHeadMessage m_botMoveHeadMessage;
		MsgPackProtocol::BotStatsMessage m_botStatsMessage;
		MsgPackProtocol::BotLogMessage m_botLogMessage;

		std::ostringstream m_stream;

		void appendMessage(const msgpack::sbuffer &buf);

	public:
		MsgPackUpdateTracker();

		/* Implemented functions */
		void foodConsumed(
				const Food &food,
				const Bot &by_bot) override;

		void foodDecayed(const Food &food) override;

		void foodSpawned(const Food &food) override;

		void botSpawned(const Bot &bot) override;

		void botKilled(const Bot &victim, const Bot *killer) override;

		void botMoved(const Bot& bot, std::size_t steps) override;

		void botLogMessage(uint64_t viewerKey, const std::string &message) override;

		void gameInfo(void);

		void worldState(Field &field) override;

		void tick(uint64_t frame_id) override;

		void botStats(const Bot &bot) override;

		std::string serialize(void) override;

		void reset(void) override;
};
