#include "bot/bot_service.h"

void register_user(TgBot::Bot& bot, pqxx::connection& pg_conn, int64_t user_tg_id) {
	try {
		pqxx::work txn(pg_conn);

		pqxx::result result = txn.exec_params("SELECT user_id FROM users WHERE telegram_id = $1", user_tg_id);
		if (result.empty()) {
			txn.exec_params("INSERT INTO users (telegram_id) VALUES ($1)", user_tg_id);
			txn.commit();
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
}

void link_new_wallet(TgBot::Bot& bot, pqxx::connection& pg_conn, int64_t user_tg_id, const std::string& walletAddress) {
	register_user(bot, pg_conn, user_tg_id);
	try {
		pqxx::work txn(pg_conn);

		pqxx::result result = txn.exec_params("SELECT wallet_id FROM wallet WHERE wallet_address = $1", walletAddress);
		if (result.empty()) {
			txn.exec_params("INSERT INTO wallet (wallet_address, user_id) VALUES($1, (SELECT user_id FROM users WHERE telegram_id = $2)); ", walletAddress, user_tg_id);
			txn.commit();
			bot.getApi().sendMessage(user_tg_id, "Wallet added successfully!");
		}
		else bot.getApi().sendMessage(user_tg_id, "This wallet was already added");
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		bot.getApi().sendMessage(user_tg_id, "Error adding wallet, try again");
	}
}

void view_wallets(TgBot::Bot& bot, pqxx::connection& pg_conn, int64_t user_tg_id) {
	register_user(bot, pg_conn, user_tg_id);
	try {
		pqxx::work txn(pg_conn);

		pqxx::result result = txn.exec_params(
			"SELECT wallet_address FROM wallet WHERE user_id = (SELECT user_id FROM users WHERE telegram_id = $1)",
			user_tg_id
		);

		if (result.empty()) {
			bot.getApi().sendMessage(user_tg_id, "You have no wallets added.");
		}
		else {
			std::string message = "Your wallets:\n";
			for (const auto& row : result) {
				message += "- " + row["wallet_address"].as<std::string>() + "\n";
			}
			bot.getApi().sendMessage(user_tg_id, message);
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		bot.getApi().sendMessage(user_tg_id, "Error retrieving wallets, try again");
	}
}

void wallet_info(TgBot::Bot& bot, pqxx::connection& pg_conn, sw::redis::Redis& redis, int64_t user_tg_id, const std::string& walletAddress) {
	try {
		pqxx::work txn(pg_conn);
		pqxx::result result = txn.exec_params(
			"SELECT wallet_id FROM wallet WHERE wallet_address = $1 AND user_id = (SELECT user_id FROM users WHERE telegram_id = $2)",
			walletAddress, user_tg_id
		);

		if (result.empty()) {
			bot.getApi().sendMessage(user_tg_id, "This wallet is not associated with your account.");
		}
		else {
			double btcBalance = getBitcoinBalance(redis, walletAddress);
			double ethBalance = getEthereumBalance(redis, walletAddress);
			double tonBalance = getToncoinBalance(redis, walletAddress);

			std::string message = "Wallet info for " + walletAddress + ":\n";
			if (btcBalance >= 0)
				message += "Bitcoin balance: " + std::to_string(btcBalance) + " BTC\n";
			else
				message += "Bitcoin balance: Error retrieving data\n";

			if (ethBalance >= 0)
				message += "Ethereum balance: " + std::to_string(ethBalance) + " ETH\n";
			else
				message += "Ethereum balance: Error retrieving data\n";

			if (tonBalance >= 0)
				message += "TON balance: " + std::to_string(tonBalance) + " TON\n";
			else
				message += "TON balance: Error retrieving data\n";

			bot.getApi().sendMessage(user_tg_id, message);
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		bot.getApi().sendMessage(user_tg_id, "Error retrieving wallet info, try again");
	}
}

void target_coins(TgBot::Bot& bot, pqxx::connection& pg_conn, sw::redis::Redis& redis_conn, int64_t user_tg_id, std::string& coinId) {
	try {
		//Bitcoin -> bitcoin
		for (char& c : coinId)
			c = std::tolower(c);

		const double price = getCoinPrice(redis_conn, coinId);
		if (price == -1)
			bot.getApi().sendMessage(user_tg_id, "Can't get price for this coin id :(");
		else
			bot.getApi().sendMessage(user_tg_id, "Target coin info for " + coinId + ":\nPrice: " + std::to_string(price) + " USD");
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		bot.getApi().sendMessage(user_tg_id, "Error retrieving coin info, try again");
	}
}