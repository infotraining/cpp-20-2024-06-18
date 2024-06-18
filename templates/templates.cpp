#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>
#include <vector>



using namespace std::literals;

template <typename T>
void foo(T arg)
{
    std::cout << "foo<T>(" << arg << ")\n";
}

auto bar(auto arg1, auto arg2)
{
    std::cout << "bar<T>(" << arg1 << ", " << arg2 << ")\n";
}

namespace IsInterpretedAs
{
    template <typename T1, typename T2>
    void bar(T1 arg1, T2 arg2)
    {
        std::cout << "bar<T>(" << arg1 << ", " << arg2 << ")\n";
    }

    template <typename T>
    void fun(T arg1, T arg2)
    {
        std::cout << "bar<T>(" << arg1 << ", " << arg2 << ")\n";
    }

    namespace Alternative
    {
        void fun(auto arg1, decltype(arg1) arg2)
        {
            std::cout << "bar<T>(" << arg1 << ", " << arg2 << ")\n";
        }
    } // namespace Alternative
} // namespace IsInterpretedAs

template <typename T>
std::unique_ptr<T> make_object_on_heap(auto&&... arg)
{
    return std::make_unique<T>(std::forward<decltype(arg)>(arg)...);
}

void cmp_by_less_func(const auto& a, const auto& b)
{
    return a < b;
}

TEST_CASE("template functions + auto")
{
    foo(42);
    foo(std::string("abc"));

    bar(42, 665u);
    bar(std::string("abc"), "text");
    bar<double>(3.14, 3.14f);

    auto obj = make_object_on_heap<std::string>("text");
    std::cout << "on heap: " << *obj << "\n";

    SECTION("generic lambda")
    {
        auto cmp_by_less = [](const auto& a, const auto& b) {
            return a < b;
        };

        auto cmp_by_greater = []<typename T1, typename T2>(const T1& a, const T2& b) {
            return a > b;
        };
    }
}

struct Data
{
    std::vector<int> data;

    Data(size_t size)
        : data(size)
    { }

    void fill(auto value)
    {
        std::ranges::fill(data, value);
    }
};

TEST_CASE("member function with auto")
{
    Data data_1(10);
    data_1.fill(3);

    CHECK(data_1.data[4] == 3);
}

TEST_CASE("templates & lambda expressions")
{
}