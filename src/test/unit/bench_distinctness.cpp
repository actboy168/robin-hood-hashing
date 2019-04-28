#include "robin_hood.h"

#include "benchmark.h"
#include "doctest.h"
#include "sfc64.h"

TYPE_TO_STRING(robin_hood::unordered_flat_map<int, int>);
TYPE_TO_STRING(robin_hood::unordered_node_map<int, int>);

// benchmark adapted from https://github.com/attractivechaos/udb2
// this implementation should have less overhead, because sfc64 and it's uniform() is extremely
// fast.
TEST_CASE_TEMPLATE("bench distinctness" * doctest::test_suite("bench"), Map,
                   robin_hood::unordered_flat_map<int, int>,
                   robin_hood::unordered_node_map<int, int>) {
    using mt = typename Map::mapped_type;

    static mt const upper = 50'000'000;
    static mt const lower = 10'000'000;
    static mt const num_steps = 5;
    static mt const step_width = (upper - lower) / num_steps;

    mt divisor;
    char const* title;
    int required_checksum;

    SUBCASE("5%") {
        divisor = 20;
        title = "  5% distinct";
        required_checksum = 1979978125;
    }
    SUBCASE("25%") {
        divisor = 4;
        title = " 25% distinct";
        required_checksum = 539977283;
    }
    SUBCASE("50%") {
        divisor = 2;
        title = " 50% distinct";
        required_checksum = 359988809;
    }
    SUBCASE("100%") {
        divisor = 0;
        title = "100% distinct";
        required_checksum = 181516798;
    }

    sfc64 rng(123);
    int checksum = 0;
    BENCHMARK(title, 1, "op") {
        for (mt n = lower; n <= upper; n += step_width) {
            mt const max_rng = divisor == 0 ? (std::numeric_limits<mt>::max)() : n / divisor;
            Map map;
            for (mt i = 0; i < n; ++i) {
                checksum += ++map[rng.uniform(max_rng)];
            }
        }
    }
    REQUIRE(checksum == required_checksum);
}
