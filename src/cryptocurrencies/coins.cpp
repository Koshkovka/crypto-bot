#include "cryptocurrencies/coins.h"



std::vector<double> getHistoricalPrices(sw::redis::Redis& redis, const std::string& coinId, int days) {
	std::vector<double> prices;
	auto now = std::chrono::system_clock::now();
	auto start = now - std::chrono::hours(24 * days);

	for (int day = 0; day < days; ++day) {
		auto timestamp = std::chrono::system_clock::to_time_t(start + std::chrono::hours(24 * day));
		std::tm tm = *std::localtime(&timestamp);
		std::ostringstream oss;
		oss << std::put_time(&tm, "%d-%m-%Y");
		std::string date = oss.str();

		// Check Redis first
		auto priceStr = redis.get("coin_price:" + coinId + ":usd:" + date);
		if (priceStr) {
			prices.push_back(std::stod(priceStr.value()));
			continue;
		}

		// Check CoinGecko
		std::string url = "https://api.coingecko.com/api/v3/coins/" + coinId + "/history?date=" + date;
		json response = performCurlRequestJson(url);
		if (!response.is_null() && response.contains("market_data")) {
			double price = response["market_data"]["current_price"]["usd"].get<double>();
			redis.set("coin_price:" + coinId + ":usd:" + date, std::to_string(price));
			redis.expire("coin_price:" + coinId + ":usd:" + date, 86400); // TTL = 1 day
			prices.push_back(price);
		}
		else {
			prices.push_back(-1);
		}
	}

	return prices;
}

double getCoinPrice(sw::redis::Redis& redis, const std::string& coinId) {
	//Prices for 1 day
	std::vector<double> prices = getHistoricalPrices(redis, coinId, 1);

	if (!prices.empty() && prices[0] != -1) {
		return prices[0];
	}
	return -1;
}