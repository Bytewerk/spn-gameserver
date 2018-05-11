/*
 * Schlangenprogrammiernacht: A programming game for GPN18.
 * Copyright (C) 2018  bytewerk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <memory>

#include <TcpServer/TcpServer.h>

#include "UpdateTracker.h"
#include "Field.h"
#include "Database.h"

class Game
{
	private:
		static constexpr const int DB_QUERY_INTERVAL = 60;
		static constexpr const int STREAM_STATS_UPDATE_INTERVAL = 60;
		static constexpr const int DB_STATS_UPDATE_INTERVAL = 600;

		TcpServer server;
		std::unique_ptr<Field> m_field;
		std::unique_ptr<db::IDatabase> m_database;
		int m_dbQueryCounter = 0;
		int m_streamStatsUpdateCounter = 0;

		bool connectDB();
		void queryDB();
		void createBot(int bot_id);

	public:
		Game();

		bool OnConnectionEstablished(TcpSocket &socket);
		bool OnConnectionClosed(TcpSocket &socket);
		bool OnDataAvailable(TcpSocket &socket);
		bool OnTimerInterval();

		int Main();
};
