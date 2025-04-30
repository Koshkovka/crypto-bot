#pragma once

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <pqxx/pqxx>
#include "utils/parseJSON.h"

pqxx::connection create_connection_to_postgres();