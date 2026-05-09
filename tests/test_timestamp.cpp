#include <catch2/catch_test_macros.hpp>
#include <regex>
#include <string>
#include "../src/sbom_builder.h"

// Regex matching ISO 8601 UTC timestamp: YYYY-MM-DDTHH:mm:ssZ
static const std::regex iso8601Regex(
    "^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}Z$");

TEST_CASE("makeTimestamp returns valid ISO 8601 UTC format", "[timestamp]") {
    std::string ts = makeTimestamp();
    REQUIRE(std::regex_match(ts, iso8601Regex));
}

TEST_CASE("makeTimestamp ends with Z suffix", "[timestamp]") {
    std::string ts = makeTimestamp();
    REQUIRE(ts.back() == 'Z');
}

TEST_CASE("makeTimestamp year is reasonable", "[timestamp]") {
    std::string ts = makeTimestamp();
    // Year should be in range 2020-2030 for the foreseeable future
    int year = std::stoi(ts.substr(0, 4));
    REQUIRE(year >= 2020);
    REQUIRE(year <= 2030);
}

TEST_CASE("makeTimestamp returns stable value on repeated calls within same second", "[timestamp]") {
    std::string ts1 = makeTimestamp();
    std::string ts2 = makeTimestamp();
    std::string ts3 = makeTimestamp();
    // Within the same second, all three should be equal.
    // (If this test fails due to second boundary, it's a rare edge case.)
    REQUIRE(ts1 == ts2);
    REQUIRE(ts2 == ts3);
}

TEST_CASE("makeTimestamp produces valid month, day, hour, minute, second", "[timestamp]") {
    std::string ts = makeTimestamp();
    int month = std::stoi(ts.substr(5, 2));
    int day   = std::stoi(ts.substr(8, 2));
    int hour  = std::stoi(ts.substr(11, 2));
    int min   = std::stoi(ts.substr(14, 2));
    int sec   = std::stoi(ts.substr(17, 2));

    REQUIRE(month >= 1);  REQUIRE(month <= 12);
    REQUIRE(day >= 1);    REQUIRE(day <= 31);
    REQUIRE(hour >= 0);   REQUIRE(hour <= 23);
    REQUIRE(min >= 0);    REQUIRE(min <= 59);
    REQUIRE(sec >= 0);    REQUIRE(sec <= 59);
}
