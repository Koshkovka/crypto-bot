#include "services/cache/redis.h"


sw::redis::Redis create_connection_to_redis() {
	int attempts = 3;

	while (attempts > 0) {
		try {

			std::string host, port;
			getConfigValue("redis.host", host);
			getConfigValue("redis.port", port);

			std::string address = "tcp://" + host + ":" + port;

			auto redis = sw::redis::Redis(address);

			redis.ping();

			std::cout << "Connected to Redis at " << address << std::endl;
			return redis;
		}
		catch (const sw::redis::Error& e) {
			std::cerr << "Error: " << e.what() << ". Attempts left: " << --attempts << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(5));
		}
	}

	std::cerr << "Error: could not connect to Redis after all attempts. Exiting." << std::endl;
	std::exit(1);
}


