#pragma once

#include "services/base/postgres.h"
#include "services/cache/redis.h"
#include "cryptocurrencies/analyzer.h"
#include "cryptocurrencies/coins.h"
#include "cryptocurrencies/wallet.h"
#include "utils/parseJSON.h"
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <tgbot/tgbot.h>
#include <iostream>
#include <unordered_map>


void register_user(const TgBot::Bot& bot, pqxx::connection& pg_conn, int64_t user_tg_id);
void link_new_wallet(const TgBot::Bot& bot, pqxx::connection& pg_conn, int64_t user_tg_id, const std::string& walletAddress);
void view_wallets(const TgBot::Bot& bot, pqxx::connection& pg_conn, int64_t user_tg_id);
void wallet_info(const TgBot::Bot& bot, pqxx::connection& pg_conn, sw::redis::Redis& redis_conn, int64_t user_tg_id, const std::string& walletAddress);
void target_coins(const TgBot::Bot& bot, pqxx::connection& pg_conn, sw::redis::Redis& redis_conn, int64_t user_tg_id, std::string& coinId);
void unlink_wallet(const TgBot::Bot& bot, pqxx::connection& pg_conn, sw::redis::Redis& redis, const std::string& walletAddress, int64_t user_tg_id);