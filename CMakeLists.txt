cmake_minimum_required(VERSION 3.10.2)
project(bot)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(Boost_USE_MULTITHREADED ON)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(CURL)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)
find_package(redis++ CONFIG REQUIRED)


include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${OPENSSL_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
)

if (CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    add_definitions(-DHAVE_CURL)
endif()


add_executable(bot 
    "include/main.h" "src/main.cpp" 
    "include/bot/bot_service.h" "src/bot/bot_service.cpp"
    "include/cryptocurrencies/analyzer.h" "src/cryptocurrencies/analyzer.cpp"
    "include/cryptocurrencies/coins.h" "src/cryptocurrencies/coins.cpp" 
    "include/cryptocurrencies/wallet.h" "src/cryptocurrencies/wallet.cpp"
    "include/services/base/postgres.h" "src/services/base/postgres.cpp"
    "include/services/cache/redis.h" "src/services/cache/redis.cpp"
    "include/utils/parseJSON.h" "src/utils/parseJSON.cpp"
    "include/utils/forCurl.h" "src/utils/forCurl.cpp"
    "include/utils/thread_pool.h" "include/utils/connection_pool.h" 
    )

# Линкуем зависимости
target_link_libraries(
    bot
    C:/vcpkg/installed/x64-windows/lib/TgBot.lib
    ${CMAKE_THREAD_LIBS_INIT}
    ${OPENSSL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${CURL_LIBRARIES}
    nlohmann_json::nlohmann_json
    libpqxx::pqxx
    redis++::redis++
)