#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
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

/////////////////////////////////////////////////////
// NTTP

template <double Factor, typename T>
auto scale(T x)
{
    return x * Factor;
}

TEST_CASE("NTTP")
{
    CHECK(scale<2.0>(8) == 16);
    CHECK(scale<3.14>(10) == Catch::Approx(31.4));
}

/////////////////////////////////////////////

struct Tax
{
    double value;

    constexpr Tax(double v) : value{v}
    {}
};

template <Tax Vat_v>
constexpr auto calc_gross_price(double net_price)
{
    return net_price + net_price * Vat_v.value;
}

TEST_CASE("struct as NTTP")
{
    constexpr Tax vat_pl{0.23};
    constexpr Tax vat_ger{0.19};

    CHECK(calc_gross_price<vat_pl>(100.0) == 123.0);
    CHECK(calc_gross_price<vat_ger>(100.0) == 119.0);    
}

template <size_t N>
struct Str
{
    char text[N];

    constexpr Str(const char(&str)[N])
    {
        std::copy(str, str+N, text);
    }

    auto operator<=>(const Str&) const = default;

    friend std::ostream& operator<<(std::ostream& out, const Str& str)
    {
        out << str.text;

        return out;
    }
};


template <Str Name_v>
class Logger
{
public:
    void log(std::string_view msg)
    {
        std::cout << Name_v << " logger: " << msg << "\n";
    }
};

TEST_CASE("string as NTTP")
{
    Logger<"main"> logger1;
    Logger<"backup"> logger2;

    logger1.log("Start");
    logger2.log("Start");
}

//////////////////////////////////////////////////////////
// lambda as NTTP

template <std::invocable auto GetVat>
constexpr double calculate_gross_price(double price)
{
    return price + price * GetVat();
}

TEST_CASE("NTTP + lambda")
{
    CHECK(calculate_gross_price<[]{ return 0.23; }>(100.0) == 123.0);

    constexpr static auto vat_ger = []{ return 0.19; };
    CHECK(calculate_gross_price<vat_ger>(100.0) == 119.0);
}