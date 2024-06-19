#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <iostream>
#include <numbers>
#include <numeric>
#include <span>
#include <string>
#include <vector>

using namespace std::literals;

void print(std::span<const int> data, std::string_view desc)
{
    std::cout << desc << ": [ ";
    for (const auto& item : data)
        std::cout << item << " ";
    std::cout << "]\n";
}

void zero(std::span<int> data, int zero_value = 0)
{
    for (auto& item : data)
    {
        item = zero_value;
    }
}

TEST_CASE("std::span")
{
    int vec[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    print(vec, "vec");

    std::span<int, 4> slice_1(std::ranges::data(vec), 4); // span - fixed extent
    print(slice_1, "slice of vector");
    slice_1[2] = 665;
    print(slice_1, "slice of vector");

    std::span slice_2 = vec; // std::span<int, 10> - CTAD

    std::vector<int> other_vec = {1, 2, 3, 4};
    std::span<int> slice_3 = other_vec;
    print(slice_3, "all items of other_vec");

    slice_3 = std::span{other_vec.data(), other_vec.size() / 2};
    print(slice_3, "first half");

    for (auto& item : slice_3)
    {
        item = 0;
    }
    print(other_vec, "after for");

    int* buffer_impl = new int[100];
    size_t size_impl = 100;

    std::span<int> buffer{buffer_impl, size_impl};
}

TEST_CASE("std::span - subspan")
{
    std::vector<int> vec(100);
    std::iota(vec.begin(), vec.end(), 0);

    print(vec, "vec");

    const size_t col_size = 10;

    for (size_t row = 0; row < vec.size() / col_size; ++row)
    {
        auto row_data = std::span{vec}.subspan(row * col_size, col_size);

        print(row_data, std::format("Row#{}", row));
    }
}

void print_as_bytes(const float f, const std::span<const std::byte> bytes)
{
#ifdef __cpp_lib_format
    std::cout << std::format("{:+6}", f) << " - { ";

    for (std::byte b : bytes)
    {
        std::cout << std::format("{:02X} ", std::to_integer<int>(b));
    }

    std::cout << "}\n";
#endif
}

TEST_CASE("float as span of bytes")
{
    float data[] = {std::numbers::pi_v<float>};

    std::span<const std::byte> const_bytes = std::as_bytes(std::span{data});
    print_as_bytes(data[0], const_bytes);

    std::span<std::byte> writeable_bytes = std::as_writable_bytes(std::span{data});
    writeable_bytes[3] |= std::byte{0b1000'0000};
    print_as_bytes(data[0], const_bytes);
}

//////////////////////////////////////////////////
// BEWARE

constexpr std::span<int> get_head(std::span<int> items, size_t head_size = 1)
{
    return items.first(head_size);
}

TEST_CASE("beware of dangling pointers")
{
    SECTION("OK")
    {
        std::vector vec = {1, 2, 3, 4, 5};
        auto head = get_head(vec, 3);
        print(head, "head");
    }

    SECTION("dangling pointers")
    {
        std::vector vec = {1, 2, 3, 4, 5};
        auto head = get_head(vec, 3);
        vec.push_back(6);
        print(head, "head");
    }
}