#include <array>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <ranges>
#include <numeric>

using namespace std::literals;

int runtime_func(int x)
{
    return x * x;
}

constexpr int constexpr_func(int x)
{
    return x * x;
}

consteval int consteval_func(int x)
{
    return x * x;
}

void compile_time_error() // runtime function
{ }

consteval int next_two_digit_value(int value)
{
    if (value < 9 || value >= 99)
    {
        compile_time_error();
    }

    return ++value;
}

TEST_CASE("consteval")
{
    int x = 10;
    const int y = 20;
    constexpr int z = 30;

    int tab1[consteval_func(z)] = {consteval_func(2), consteval_func(3)};

    STATIC_CHECK(next_two_digit_value(42) == 43);
    // STATIC_CHECK(next_two_digit_value(99) == 100);
}

TEST_CASE("consteval lambda")
{
    auto square = [](int n) consteval {
        return n * n;
    };

    constexpr std::array arr = {square(1), square(2), square(3)};
}

constexpr int len(const char* s)
{
    if (std::is_constant_evaluated())
    // if consteval // C++23
    {
        // compile-time friendly code
        int idx = 0;

        while (s[idx] != '\0')
            ++idx;
        return idx;
    }
    else
    {
        return std::strlen(s); // function called at runtime
    }
}

TEST_CASE("constexpr extensions")
{
    static_assert(len("abc") == 3);
    CHECK(len("abc") == 3);
}

template <size_t N>
constexpr auto create_powers()
{
    std::array<uint32_t, N> powers{};

    // std::iota(powers.begin(), powers.end(), 1); // std algorithms are constexpr
    // std::ranges::transform(powers, powers.begin(), [](int x) { return x * x; }); // ranges algorithms are constexpr

    auto squares = std::views::iota(1)
        | std::views::transform([](int x) { return x * x; })
        | std::views::take(N);

    std::ranges::copy(squares, powers.begin());

    return powers;
}

template <std::ranges::input_range... TRng_>
constexpr auto avg_for_unique(const TRng_&... rng)
{
    using TElement = std::common_type_t<std::ranges::range_value_t<TRng_>...>;

    std::vector<TElement> vec;                            // empty vector
    vec.reserve((rng.size() + ...));                      // reserve a buffer - fold expression C++17
    (vec.insert(vec.end(), rng.begin(), rng.end()), ...); // fold expression C++17

    // sort items
    std::ranges::sort(vec); // std::sort(vec.begin(), vec.end());

    // create span of unique_items
    auto new_end = std::unique(vec.begin(), vec.end());
    std::span unique_items{vec.begin(), new_end};

    // calculate sum of unique items
    auto sum = std::accumulate(unique_items.begin(), unique_items.end(), TElement{});

    return sum / static_cast<double>(unique_items.size());
}


TEST_CASE("constexpr - lookup table")
{
    constexpr auto powers = create_powers<100>();

    constexpr std::array lst1 = {1, 2, 3, 4, 5};
    constexpr std::array lst2 = {5, 6, 7, 8, 9};

    constexpr auto avg = avg_for_unique(lst1, lst2);

    std::cout << "AVG: " << avg << "\n";
}

consteval void memleak_free()
{
    int* tab = new int[1024];

    tab[1023] = 8;

    delete[] tab;
}

TEST_CASE("memleak")
{
    memleak_free();
}

// a.cpp
constinit int global_x = constexpr_func(10);

// b.cpp
int global_y = global_x;
