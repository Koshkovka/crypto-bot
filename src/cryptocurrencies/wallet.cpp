#include "cryptocurrencies/wallet.h"


double getBitcoinBalance(sw::redis::Redis& redis, const std::string& address) {
	//Check Redis first
	auto balanceStr = redis.get("wallet_balance_btc:" + address);
	if (balanceStr) {
		return std::stod(*balanceStr);
	}

	//Check Blockcypher API
	std::string url = "https://api.blockcypher.com/v1/btc/main/addrs/" + address + "/balance";
	json response = performCurlRequestJson(url);
	if (!response.is_null()) {
		double balance = response["final_balance"].get<double>() / 100000000; // satoshi -> BTC
		redis.set("wallet_balance_btc:" + address, std::to_string(balance));
		redis.expire("wallet_balance_btc:" + address, 86400); // TTL = 1 day
		return balance;
	}
	return -1;
}

double getEthereumBalance(sw::redis::Redis& redis, const std::string& address) {
	//Check Redis first
	auto balanceStr = redis.get("wallet_balance_eth:" + address);
	if (balanceStr) {
		return std::stod(*balanceStr);
	}
	//Check Etherscan API
	std::string token;
	if (!getConfigValue("ETHERSCAN_TOKEN", token)) {
		std::cerr << "Error: could not find ETHERSCAN_TOKEN in config.json" << std::endl;
	}
	std::string url = "https://api.etherscan.io/api?module=account&action=balance&address=" + address + "&tag=latest&apikey=" + token;
	json response = performCurlRequestJson(url);
	if (!response.is_null()) {
		double balance = response["result"].get<double>() / 1000000000000000000; // Wei -> ETH
		redis.set("wallet_balance_eth:" + address, std::to_string(balance));
		redis.expire("wallet_balance_eth:" + address, 86400); // TTL = 1 day
		return balance;
	}
	return -1;
}

double getToncoinBalance(sw::redis::Redis& redis, const std::string& address) {
	//Check Redis first
	auto balanceStr = redis.get("wallet_balance_ton:" + address);
	if (balanceStr) {
		return std::stod(*balanceStr);
	}

	//Check Ton API
	std::string url = "https://tonapi.io/v1/account/getInfo?account=" + address;
	json response = performCurlRequestJson(url);
	if (!response.is_null()) {
		double balance = response["balance"].get<double>() / 1000000000; // nanoTon -> TON
		redis.set("wallet_balance_ton:" + address, std::to_string(balance));
		redis.expire("wallet_balance_ton:" + address, 86400); // TTL = 1 day
		return balance;
	}
	return -1;
}

