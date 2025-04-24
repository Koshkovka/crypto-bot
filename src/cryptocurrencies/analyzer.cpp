#include "cryptocurrencies/analyzer.h"

//Simple Moving Average
double calculateSMA(const std::vector<double>& prices, int period) {
	if (prices.size() < period) return 0.0;
	double sum = std::accumulate(prices.end() - period, prices.end(), 0.0);
	return sum / period;
}

//Relative Strength Index
double calculateRSI(const std::vector<double>& prices, int period) {
	if (prices.size() < period) return 50.0;

	std::vector<double> gains, losses;
	for (size_t i = 1; i < prices.size(); ++i) {
		double change = prices[i] - prices[i - 1];
		if (change > 0) gains.push_back(change);
		else losses.push_back(-change);
	}

	double avgGain = gains.empty() ? 0 : std::accumulate(gains.begin(), gains.end(), 0.0) / period;
	double avgLoss = losses.empty() ? 0 : std::accumulate(losses.begin(), losses.end(), 0.0) / period;

	if (avgLoss == 0) return 100.0;
	double rs = avgGain / avgLoss;
	return 100 - (100 / (1 + rs));
}

std::string getRecommenation(double rsi, double sma, double price) {
	if (rsi < 30 && price > sma) {
		return "Buy";
	}
	else if (rsi > 70 && price < sma) {
		return "Sell";
	}
	else {
		return "Hold";
	}
}

std::string evaluateCoin(sw::redis::Redis& redis, const std::string& coinId, int periodDays) {
	// Get historical prices
	std::vector<double> historicalPrices = getHistoricalPrices(redis, coinId, periodDays);

	// Remove missing data points
	historicalPrices.erase(std::remove(historicalPrices.begin(), historicalPrices.end(), -1), historicalPrices.end());

	if (historicalPrices.size() < periodDays) {
		return "Not enough data to evaluate";
	}

	// Calculate SMA and RSI
	double sma = calculateSMA(historicalPrices, periodDays);
	double rsi = calculateRSI(historicalPrices, periodDays);
	double currentPrice = getCoinPrice(redis, coinId);

	return getRecommenation(rsi, sma, currentPrice);
}

