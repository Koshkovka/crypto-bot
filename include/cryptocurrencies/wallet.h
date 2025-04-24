#pragma once 

#include "services/cache/redis.h"
#include "utils/forCurl.h"

double getBitcoinBalance(sw::redis::Redis& redis, const std::string& address);
double getEthereumBalance(sw::redis::Redis& redis, const std::string& address);
double getToncoinBalance(sw::redis::Redis& redis, const std::string& address);


