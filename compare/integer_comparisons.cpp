#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <utility>

using namespace std::literals;

TEST_CASE("safe comparing integral numbers")
{
    int x = -42;
    unsigned int y = 665;

    SECTION("std::cmp_*")
    {
        // CHECK(x < y);
        CHECK(std::cmp_less(x, y));

        auto cmp_by_less = [](auto a, auto b) {                        
            if constexpr(std::integral<decltype(a)> && std::integral<decltype(b)>)
                return std::cmp_less(a, b);
            else
                return a < b;
        };

        auto cmp_by_less_ints = [](std::integral auto a, std::integral auto b) {
            return std::cmp_less(a, b);
        };

        CHECK(cmp_by_less(x, y));
        CHECK(cmp_by_less("abc"s, "def"s));
    }

    SECTION("std::in_range<T>")
    {
        CHECK(std::in_range<size_t>(665));
        CHECK(std::in_range<size_t>(-1) == false);
        static_assert(std::in_range<uint8_t>(257) == false);
    }
}