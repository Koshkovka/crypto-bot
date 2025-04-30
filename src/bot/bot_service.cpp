#include "bot/bot_service.h"

void register_user(const TgBot::Bot& bot, pqxx::connection& pg_conn, int64_t user_tg_id) {
    try {
        pqxx::work txn(pg_conn);
        pqxx::result result = txn.exec_params("SELECT user_id FROM users WHERE telegram_id = $1", user_tg_id);
        if (result.empty()) {
            txn.exec_params("INSERT INTO users (telegram_id) VALUES ($1)", user_tg_id);
            txn.commit();
        }
		//Chat keyboard
        TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
        std::vector<TgBot::InlineKeyboardButton::Ptr> row1;
        std::vector<TgBot::InlineKeyboardButton::Ptr> row2;

        auto linkWalletButton = std::make_shared<TgBot::InlineKeyboardButton>();
        linkWalletButton->text = "🔗 Link New Wallet";
        linkWalletButton->callbackData = "/link_new_wallet";

    	auto unlink_walletButton = std::make_shared<TgBot::InlineKeyboardButton>();
    	unlink_walletButton->text = "🚮 Delete Wallet";
    	unlink_walletButton->callbackData = "/unlink_wallet";

        auto viewWalletsButton = std::make_shared<TgBot::InlineKeyboardButton>();
        viewWalletsButton->text = "👛 View Wallets";
        viewWalletsButton->callbackData = "/view_wallets";

        auto walletInfoButton = std::make_shared<TgBot::InlineKeyboardButton>();
        walletInfoButton->text = "ℹ️ Wallet Info";
        walletInfoButton->callbackData = "/wallet_info";

        auto targetCoinsButton = std::make_shared<TgBot::InlineKeyboardButton>();
        targetCoinsButton->text = "🎯 Target Coins";
        targetCoinsButton->callbackData = "/target_coins";

        row1.push_back(linkWalletButton);
    	row1.push_back(unlink_walletButton);
        row1.push_back(viewWalletsButton);
        row2.push_back(walletInfoButton);
        row2.push_back(targetCoinsButton);

        keyboard->inlineKeyboard.push_back(row1);
        keyboard->inlineKeyboard.push_back(row2);

        bot.getApi().sendMessage(user_tg_id, "Welcome to Crypto Wallet Helper! 🚀\nChoose an action:", false, 0, keyboard);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
void link_new_wallet(const TgBot::Bot& bot, pqxx::connection& pg_conn, int64_t user_tg_id, const std::string& walletAddress) {
    try {
        pqxx::work txn(pg_conn);
        pqxx::result result = txn.exec_params("SELECT wallet_id FROM wallet WHERE wallet_address = $1", walletAddress);

        if (result.empty()) {
            txn.exec_params("INSERT INTO wallet (wallet_address, user_id) VALUES($1, (SELECT user_id FROM users WHERE telegram_id = $2))", walletAddress, user_tg_id);
            txn.commit();

            bot.getApi().sendMessage(user_tg_id, "✅ Wallet added successfully!");
        }
        else {
            bot.getApi().sendMessage(user_tg_id, "❌ This wallet was already added");
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        bot.getApi().sendMessage(user_tg_id, "❌ Error adding wallet, try again");
    }
}
void unlink_wallet(const TgBot::Bot& bot, pqxx::connection& pg_conn, sw::redis::Redis& redis, const std::string& walletAddress, int64_t user_tg_id) {
	try {
		pqxx::work txn(pg_conn);
		// Verify wallet belongs to user before deletion
		pqxx::result check = txn.exec_params(
			"SELECT wallet_id FROM wallet WHERE wallet_address = $1 AND user_id = (SELECT user_id FROM users WHERE telegram_id = $2)",
			walletAddress, user_tg_id
		);

		if (check.empty()) {
			bot.getApi().sendMessage(user_tg_id, "❌ There is no such wallet"); // Wallet not found or doesn't belong to user
		}
		// Delete wallet from database
		txn.exec_params("DELETE FROM wallet WHERE wallet_address = $1", walletAddress);
		txn.commit();
		// Clear Redis cache entries for this wallet
		std::vector<std::string> cacheKeys = {
			"wallet_balance_btc:" + walletAddress,
			"wallet_balance_eth:" + walletAddress,
			"wallet_balance_ton:" + walletAddress
		};
		for (const std::string& key : cacheKeys) {
			redis.del(key);
		}
		bot.getApi().sendMessage(user_tg_id, "✅ Wallet successfully unlinked");
	}
	catch (const std::exception& e) {
		std::cerr << "Error deleting wallet: " << e.what() << std::endl;
		bot.getApi().sendMessage(user_tg_id, "Something went wrong:(\nTry again😇");
	}
}
void view_wallets(const TgBot::Bot& bot, pqxx::connection& pg_conn, int64_t user_tg_id) {
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

void wallet_info(const TgBot::Bot& bot, pqxx::connection& pg_conn, sw::redis::Redis& redis, int64_t user_tg_id, const std::string& walletAddress) {
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

void target_coins(const TgBot::Bot& bot, pqxx::connection& pg_conn, sw::redis::Redis& redis_conn, int64_t user_tg_id, std::string& coinId) {
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