#include "services/base/postgres.h"


pqxx::connection create_connection_to_postgres()
{
	int attempts = 3;
	while (attempts > 0) {
		try {
			std::string host, port, database, username, password;
			getConfigValue("postgres.host", host);
			getConfigValue("postgres.port", port);
			getConfigValue("postgres.database", database);
			getConfigValue("postgres.username", username);
			getConfigValue("postgres.password", password);
			pqxx::connection conn("host=" + host + " port=" + port + " dbname=" + database + " user=" + username + " password=" + password);

			if (!conn.is_open()) {
				std::cerr << "Error: could not connect to postgres. Attempts left: " << --attempts << std::endl;
				std::this_thread::sleep_for(std::chrono::seconds(5));
				continue;
			}
			std::cout << "Connected to Postgres DB: " << database << std::endl;
			return conn;
		}
		catch (const std::exception& e) {
			std::cerr << "Error: " << e.what() << ". Attempts left: " << --attempts << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(5));
		}
	}

	std::cerr << "Error: could not connect to postgres after all attempts. Bye!" << std::endl;
	std::exit(1);
}