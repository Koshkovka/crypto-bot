#include "main.h"

std::unordered_map<int64_t, std::string> userStates;



int main() {

	auto pg_conn = create_connection_to_postgres();
	auto redis_conn = create_connection_to_redis();

	std::string token;
	if (!getConfigValue("BOT_TOKEN", token)) return -1;

	TgBot::Bot bot(token);

	bot.getEvents().onCommand("start", [&bot, &pg_conn, &redis_conn](TgBot::Message::Ptr message) {
		int64_t user_tg_id = message->chat->id;
		bot.getApi().sendMessage(user_tg_id, "Hi, I'm your crypto wallet helper!");
		register_user(bot, pg_conn, user_tg_id);
		});

	bot.getEvents().onCommand("link_new_wallet", [&bot, &pg_conn](TgBot::Message::Ptr message) {
		int64_t user_tg_id = message->chat->id;
		userStates[user_tg_id] = "awaiting_address_link";
		bot.getApi().sendMessage(user_tg_id, "Enter your wallet address:");
		});

	bot.getEvents().onCommand("view_wallets", [&bot, &pg_conn](TgBot::Message::Ptr message) {
		view_wallets(bot, pg_conn, message->chat->id);
		});

	bot.getEvents().onCommand("wallet_info", [&bot, &pg_conn, &redis_conn](TgBot::Message::Ptr message) {
		int64_t user_tg_id = message->chat->id;
		userStates[user_tg_id] = "awaiting_address_info";
		bot.getApi().sendMessage(user_tg_id, "Enter the address of the wallet you want to receive information about:");
		});

	bot.getEvents().onCommand("target_coins", [&bot, &pg_conn, &redis_conn](TgBot::Message::Ptr message) {
		int64_t user_tg_id = message->chat->id;
		userStates[user_tg_id] = "awaiting_coin_id";
		bot.getApi().sendMessage(user_tg_id, "Enter the coin name (bitcoin, the-open-network (TON), etc...):");
		});

	bot.getEvents().onAnyMessage([&bot, &pg_conn, &redis_conn](TgBot::Message::Ptr message) {
		int64_t user_tg_id = message->chat->id;
		auto& msg = message->text;
		if (userStates.find(user_tg_id) != userStates.end() && msg[0] != '/') {
			const std::string state = userStates[user_tg_id];
			if (state == "awaiting_address_link") {
				link_new_wallet(bot, pg_conn, user_tg_id, msg);
				userStates.erase(user_tg_id);
			}
			else if (state == "awaiting_address_info") {
				wallet_info(bot, pg_conn, redis_conn, user_tg_id, msg);
				userStates.erase(user_tg_id);
			}
			else if (state == "awaiting_coin_id") {
				target_coins(bot, pg_conn, redis_conn, user_tg_id, msg);
				userStates.erase(user_tg_id);
			}
		}
		});

	signal(SIGINT, [](int) { exit(0); }); // Ctrl+C

	std::cout << "Bot is running..." << std::endl;

	bot.getApi().deleteWebhook();

	TgBot::TgLongPoll longPoll(bot);
	while (true) {
		longPoll.start();
	}

	return 0;
}
