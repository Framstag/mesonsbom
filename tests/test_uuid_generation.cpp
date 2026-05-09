#include <catch2/catch_test_macros.hpp>
#include <random>
#include <regex>
#include <string>
#include <unordered_set>
#include "../src/sbom_builder.h"

// Regex matching a valid UUIDv4: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
// where y is 8, 9, a, or b (variant 10xx).
static const std::regex uuidv4Regex(
    "^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$");

TEST_CASE("UUIDv4 format matches RFC 4122 pattern", "[uuid]") {
    std::string uuid = generateUuidV4();
    REQUIRE(std::regex_match(uuid, uuidv4Regex));
}

TEST_CASE("UUIDv4 version nibble is always 4", "[uuid]") {
    std::string uuid = generateUuidV4();
    // The version nibble is at position 14 (0-indexed: after 8-4-4 pattern,
    // the character after the second dash).
    REQUIRE(uuid[14] == '4');
}

TEST_CASE("UUIDv4 variant nibble is 8, 9, a, or b", "[uuid]") {
    std::string uuid = generateUuidV4();
    char variantNibble = uuid[19]; // position of first hex char after 3rd dash
    bool validVariant = (variantNibble == '8' || variantNibble == '9' ||
                         variantNibble == 'a' || variantNibble == 'b');
    REQUIRE(validVariant);
}

TEST_CASE("Multiple generateUuidV4 calls produce different values", "[uuid]") {
    std::unordered_set<std::string> seen;
    for (int i = 0; i < 100; ++i) {
        std::string uuid = generateUuidV4();
        REQUIRE(seen.count(uuid) == 0);
        seen.insert(uuid);
    }
}

TEST_CASE("Caller-provided RNG overload produces valid UUIDs", "[uuid]") {
    std::mt19937 rng(42);
    for (int i = 0; i < 50; ++i) {
        std::string uuid = generateUuidV4(rng);
        REQUIRE(std::regex_match(uuid, uuidv4Regex));
    }
}

TEST_CASE("Different seeds produce different first UUIDs", "[uuid]") {
    // Simulate application starts with different seeds.
    // Using the RNG overload for deterministic control.
    std::mt19937 start1(1001);
    std::mt19937 start2(2001);
    // Adjacent seeds (same second with broken random_device).
    std::mt19937 start3(1002);

    std::string uuid1 = generateUuidV4(start1);
    std::string uuid2 = generateUuidV4(start2);
    std::string uuid3 = generateUuidV4(start3);

    REQUIRE(uuid1 != uuid2); // different seeds → different UUIDs
    REQUIRE(uuid1 != uuid3); // adjacent seeds → different UUIDs
}
