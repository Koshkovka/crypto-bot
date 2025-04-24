#pragma once

#include "utils/parseJSON.h"
#include <chrono> 
#include <cstdlib>
#include <iostream>
#include <sw/redis++/redis++.h>
#include <thread> 


sw::redis::Redis create_connection_to_redis();