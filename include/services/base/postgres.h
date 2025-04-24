#pragma once

#include "utils/parseJSON.h"
#include <pqxx/pqxx>
#include <iostream>
#include <cstdlib>
#include <thread> 
#include <chrono> 

pqxx::connection create_connection_to_postgres();