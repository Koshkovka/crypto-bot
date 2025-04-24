
#pragma once

#include <curl/curl.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;


std::string performCurlRequest(const std::string& url);
json performCurlRequestJson(const std::string& url);
