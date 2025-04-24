#pragma once 

#include "services/cache/redis.h"
#include "utils/forCurl.h"
#include <chrono>
#include <iomanip>
#include <sstream>

std::vector<double> getHistoricalPrices(sw::redis::Redis& redis, const std::string& coinId, int days);
double getCoinPrice(sw::redis::Redis& redis, const std::string& coinId);