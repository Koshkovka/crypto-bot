#include "main.h"

std::unordered_map<int64_t, std::string> userStates;


int main() {

	auto pg_conn = create_connection_to_postgres();
	auto redis_conn = create_connection_to_redis();

	std::string token;
	if (!getConfigValue("BOT_TOKEN", token)) return -1;

	TgBot::Bot bot(token);
	bot.getEvents().onAnyMessage([&bot, &pg_conn, &redis_conn](const TgBot::Message::Ptr &message) {
		int64_t user_tg_id = message->chat->id;
		auto& msg = message->text;
		if (msg == "/start") {
			register_user(bot, pg_conn, user_tg_id);
			return;
		}
		// Проверяем, зарегистрирован ли пользователь
		try {
			pqxx::work txn(pg_conn);
			pqxx::result result = txn.exec_params("SELECT user_id FROM users WHERE telegram_id = $1", user_tg_id);
			if (result.empty()) {
				bot.getApi().sendMessage(user_tg_id, "Please use /start command to begin interaction with the bot.");
				return;
			}
		} catch (const std::exception& e) {
			std::cerr << "Error checking user registration: " << e.what() << std::endl;
			return;
		}
		// Обрабатываем остальные сообщения только для зарегистрированных пользователей
		if (userStates.contains(user_tg_id) && msg[0] != '/') {
			if (const std::string state = userStates[user_tg_id]; state == "awaiting_address_link") {
				link_new_wallet(bot, pg_conn, user_tg_id, msg);
				userStates.erase(user_tg_id);
			} else if (state == "awaiting_address_to_unlink" ) {
				unlink_wallet(bot, pg_conn, redis_conn, msg, user_tg_id);
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
	bot.getEvents().onCommand("link_new_wallet", [&bot](const TgBot::Message::Ptr &message) {
		int64_t user_tg_id = message->chat->id;
		userStates[user_tg_id] = "awaiting_address_link";
		bot.getApi().sendMessage(user_tg_id, "Enter your wallet address:");
		});
	bot.getEvents().onCommand("unlink_wallet", [&bot](const TgBot::Message::Ptr &message) {
		int64_t user_tg_id = message->chat->id;
		userStates[user_tg_id] = "awaiting_address_to_unlink";
		bot.getApi().sendMessage(user_tg_id, "Enter your wallet address:");
		});

	bot.getEvents().onCommand("view_wallets", [&bot, &pg_conn](const TgBot::Message::Ptr &message) {
		view_wallets(bot, pg_conn, message->chat->id);
		});

	bot.getEvents().onCommand("wallet_info", [&bot](const TgBot::Message::Ptr &message) {
		int64_t user_tg_id = message->chat->id;
		userStates[user_tg_id] = "awaiting_address_info";
		bot.getApi().sendMessage(user_tg_id, "Enter the address of the wallet you want to receive information about:");
		});

	bot.getEvents().onCommand("target_coins", [&bot](const TgBot::Message::Ptr &message) {
		int64_t user_tg_id = message->chat->id;
		userStates[user_tg_id] = "awaiting_coin_id";
		bot.getApi().sendMessage(user_tg_id, "Enter the coin name (bitcoin, the-open-network (TON), etc...):");
		});
	//Обрабатываем кнопки
	bot.getEvents().onCallbackQuery([&bot, &pg_conn](const TgBot::CallbackQuery::Ptr &query) {
		int64_t user_tg_id = query->message->chat->id;
		std::string data = query->data;
		if (data == "/link_new_wallet") {
			userStates[user_tg_id] = "awaiting_address_link";
			bot.getApi().sendMessage(user_tg_id, "Enter your wallet address:");
		}
		else if (data == "/unlink_wallet") {
			userStates[user_tg_id] = "awaiting_address_to_unlink";
			bot.getApi().sendMessage(user_tg_id, "Enter your wallet address:");
		}
		else if (data == "/view_wallets") {
			view_wallets(bot, pg_conn, user_tg_id);
		}
		else if (data == "/wallet_info") {
			userStates[user_tg_id] = "awaiting_address_info";
			bot.getApi().sendMessage(user_tg_id, "Enter the address of the wallet you want to receive information about:");
		}
		else if (data == "/target_coins") {
			userStates[user_tg_id] = "awaiting_coin_id";
			bot.getApi().sendMessage(user_tg_id, "Enter the coin name (bitcoin, the-open-network (TON), etc...):");
		}
		bot.getApi().answerCallbackQuery(query->id);
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
