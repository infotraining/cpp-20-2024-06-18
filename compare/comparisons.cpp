#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <list>

using namespace std::literals;

struct Point
{
    int x;
    int y;

    constexpr Point(int x, int y)
        : x{x}
        , y{y}
    { }

    friend std::ostream& operator<<(std::ostream& out, const Point& p)
    {
        return out << std::format("Point({},{})", p.x, p.y);
    }

    constexpr bool operator==(const Point& other) const noexcept = default;

    bool operator==(const std::pair<int, int>& other) const
    {
        return x == other.first && y == other.second;
    }

    // bool operator!=(const Point& other) const
    // {
    //     return !(*this == other);
    // }
};

struct Point3D : Point
{
    int z;

    constexpr Point3D(int x, int y, int z)
        : Point{x, y}
        , z{z}
    { }

    bool operator==(const Point3D& other) const = default;
};

TEST_CASE("Point - operator ==")
{
    SECTION("Point")
    {
        constexpr Point p1{1, 2};
        constexpr Point p2{1, 2};
        Point p3{2, 1};

        CHECK(p1 == p2);
        static_assert(p1 == p2);
        CHECK(p1 != p3); // !(p1 == p3)

        CHECK(p1 == std::pair{1, 2}); // p1.operator==(std::pair{1, 2})
        CHECK(std::pair{1, 2} == p1); // p1 == std::pair{1, 2}
    }

    SECTION("Point3D")
    {
        Point3D p1{1, 2, 3};
        Point3D p2{1, 2, 3};
        Point3D p3{1, 2, 4};

        CHECK(p1 == p2);
        CHECK(p1 != p3);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Comparisons
{

    struct Money
    {
        int dollars;
        int cents;

        constexpr Money(int dollars, int cents)
            : dollars(dollars)
            , cents(cents)
        {
            if (cents < 0 || cents > 99)
            {
                throw std::invalid_argument("cents must be between 0 and 99");
            }
        }

        constexpr Money(double amount)
            : dollars(static_cast<int>(amount))
            , cents(static_cast<int>(amount * 100) % 100)
        { }

        friend std::ostream& operator<<(std::ostream& out, const Money& m)
        {
            return out << std::format("${}.{}", m.dollars, m.cents);
        }

        auto operator<=>(const Money&) const = default;
        // bool operator==(const Money&) const = default; // implicitly declared
    };

    namespace Literals
    {
        // clang-format off
        constexpr Money operator""_USD(long double amount)
        {
            return Money(amount);
        }
        // clang-format on
    } // namespace Literals
} // namespace Comparisons

TEST_CASE("Money - operator <=>")
{
    using Comparisons::Money;
    using namespace Comparisons::Literals;

    Money m1{42, 50};
    Money m2{42, 50};

    SECTION("comparison operators are synthetized")
    {
        CHECK(m1 == m2);
        CHECK(m1 == Money(42.50));
        CHECK(m1 == 42.50_USD);
        CHECK(m1 != 42.51_USD);
        CHECK(m1 < 42.51_USD);
        CHECK(m1 <= 42.51_USD);
        CHECK(m1 > 0.99_USD);
        CHECK(m1 >= 0.99_USD);

        static_assert(Money{42, 50} == 42.50_USD);
    }

    SECTION("sorting")
    {
        std::vector<Money> wallet{42.50_USD, 13.37_USD, 0.99_USD, 100.00_USD, 0.01_USD};
        std::ranges::sort(wallet);
        CHECK(std::ranges::is_sorted(wallet));
    }
}

TEST_CASE("operator <=>")
{
    SECTION("primitive types")
    {
        SECTION("integral types - strong ordering")
        {
            int x = 42;
            int y = 65;

            auto result = x <=> y;
            static_assert(std::is_same_v<decltype(result), std::strong_ordering>);
            CHECK((result < 0));
            CHECK((result == std::strong_ordering::less));
        }

        SECTION("floating points - partial ordering")
        {
            double dx = 0.01;
            double dy = 0.1;

            auto result = dx <=> dy; // std::partial_ordering
            CHECK((result < 0));
            CHECK((result == std::partial_ordering::less));

            result = dx <=> std::numeric_limits<double>::quiet_NaN();
            CHECK((result == std::partial_ordering::unordered));
        }
    }

    SECTION("custom types")
    {
        SECTION("result is a comparison category")
        {
            using namespace Comparisons;
            using namespace Comparisons::Literals;

            Money m{100, 99};
            auto result = m <=> 101.99_USD; // std::strong_ordering
            CHECK((result < 0));
        }

        SECTION("operators <, >, <=, >= are synthetized")
        {
            using namespace Comparisons;
            using namespace Comparisons::Literals;

            Money m{100, 99};
            CHECK(m < 102.99_USD);
        }
    }
}

struct Temperature
{
    double value;

    std::strong_ordering operator<=>(const Temperature& other) const 
    {
        return std::strong_order(value, other.value);
    }

    bool operator==(const Temperature& other) const = default;
};

TEST_CASE("Temperature - sorting")
{
    std::vector<Temperature> temperatures{ Temperature{23.0}, Temperature{std::numeric_limits<double>::quiet_NaN()}, Temperature{62.8} };

    //std::sort(temperatures.begin(), temperatures.end());
    std::ranges::sort(temperatures);
    CHECK(std::ranges::is_sorted(temperatures));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PreCpp20
{
    int value;

    bool operator==(const PreCpp20& other) const
    {
        return value == other.value;
    }

    bool operator<(const PreCpp20& other) const
    {
        return value < other.value;
    }
};

struct PostCpp20
{
    int x;
    PreCpp20 y;

    std::strong_ordering operator<=>(const PostCpp20& other) const = default;
};

TEST_CASE("Pre & Post C++20")
{
    PostCpp20 p1{1, PreCpp20{2}};
    PostCpp20 p2{1, PreCpp20{3}};

    CHECK((p1 <=> p2 < 0));
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Comparisons
{
    class Data
    {
        int* buffer_;
        size_t size_;

    public:
        Data(std::initializer_list<int> values)
            : buffer_(new int[values.size()])
            , size_(values.size())
        {
            std::copy(values.begin(), values.end(), buffer_);
        }

        ~Data()
        {
            delete[] buffer_;
        }

        std::strong_ordering operator<=>(const Data& other) const
        {
            return std::lexicographical_compare_three_way(buffer_, buffer_ + size_, other.buffer_, other.buffer_ + other.size_);
        }

        bool operator==(const Data& other) const
        {
            return std::equal(buffer_, buffer_ + size_, other.buffer_);
        }
    };
} // namespace Comparisons

TEST_CASE("lexicographical_compare_three_way")
{
    using Comparisons::Data;

    Data data1{1, 2, 3};
    Data data2{1, 2, 3};
    Data data3{1, 2, 4};

    CHECK((data1 <=> data2 == 0));
    CHECK((data1 <=> data3 < 0));
}