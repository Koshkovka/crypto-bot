#pragma once 

#include "cryptocurrencies/coins.h"
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>

double calculateSMA(const std::vector<double>& prices, int period);
double calculateRSI(const std::vector<double>& prices, int period);
std::string evaluateCoin(sw::redis::Redis& redis, const std::string& coinId, int periodDays);
std::string getRecommenation(double rsi, double sma, double price);