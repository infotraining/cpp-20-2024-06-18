#include <catch2/catch_test_macros.hpp>
#include <concepts>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std::literals;

template <typename TContainer>
void print(TContainer&& c, std::string_view prefix = "items")
{
    std::cout << prefix << ": [ ";
    for (const auto& item : c)
        std::cout << item << " ";
    std::cout << "]\n";
}

namespace Ver_1
{

    template <typename T>
    T max_value(T a, T b)
    {
        return a < b ? b : a;
    }

    template <typename T>
        requires std::is_pointer_v<T>
    auto max_value(T a, T b)
    {
        assert(a != nullptr);
        assert(b != nullptr);
        return *a < *b ? *b : *a;
    }

    namespace Alt1
    {
        auto max_value(auto a, auto b)
            requires(std::is_pointer_v<decltype(a)> && std::is_same_v<decltype(a), decltype(b)>)
        {
            assert(a != nullptr);
            assert(b != nullptr);
            return *a < *b ? *b : *a;
        }
    } // namespace Alt1
} // namespace Ver_1

namespace Ver_2
{
    template <typename T>
    concept Pointer = std::is_pointer_v<T>;

    static_assert(Pointer<int*>);
    static_assert(Pointer<const int*>);
    static_assert(!Pointer<int>);

    template <typename T>
    T max_value(T a, T b)
    {
        return a < b ? b : a;
    }

    template <Pointer T>
    // requires Pointer<T>
    auto max_value(T a, T b)
    {
        assert(a != nullptr);
        assert(b != nullptr);
        return *a < *b ? *b : *a;
    }

    namespace Alt
    {
        auto max_value(Pointer auto a, Pointer auto b)
            requires std::same_as<decltype(a), decltype(b)>
        {
            assert(a != nullptr);
            assert(b != nullptr);
            return *a < *b ? *b : *a;
        }
    } // namespace Alt
} // namespace Ver_2

namespace Ver_3
{
    template <typename T>
    concept Pointer = requires(T ptr) {
        *ptr;
        ptr == nullptr;
        //ptr != nullptr;
    };

    static_assert(Pointer<int*>);
    static_assert(Pointer<const int*>);
    static_assert(!Pointer<int>);

    template <typename T>
    T max_value(T a, T b)
    {
        return a < b ? b : a;
    }

    template <Pointer T>
    auto max_value(T a, T b)
    {
        assert(a != nullptr);
        assert(b != nullptr);
        return *a < *b ? *b : *a;
    }
} // namespace Ver_3

TEST_CASE("constraints")
{
    using namespace Ver_3;

    int x = 10;
    int y = 20;

    CHECK(max_value(x, y) == 20);

    CHECK(max_value(&x, &y) == 20);

    auto sp1 = std::make_shared<int>(20);
    auto sp2 = std::make_shared<int>(30);
    CHECK(max_value(sp1, sp2) == 30);
}

template <typename T>
concept Range = requires(T&& arg) {
    std::ranges::begin(arg);
    std::ranges::end(arg);
};

template <typename T>
concept Printable = requires(T&& arg, std::ostream& out) {
    out << arg;
};

template <typename T>
concept PrintableRange = std::ranges::range<T> && Printable<std::ranges::range_value_t<T>>;

template <PrintableRange T>
void print_all(T&& rng, std::string_view desc)
{
    std::cout << desc << ": [ ";
    for(const auto& item : rng)
        std::cout << item << " ";
    std::cout << "]\n";
}

TEST_CASE("concepts")
{
    std::vector vec = {1, 2, 3};
    print_all(vec, "vec");
    REQUIRE(true);
}

template <typename T>    
struct DataHolder
{
    T value;

    void print() const 
    {
        std::cout << "value: " << value << "\n";
    }

    void print() const requires PrintableRange<T>
    {
        print_all(value, "values");
    }
};

// template <std::integral T>
// struct DataHolder<T>
// {
//     T value;

//     std::int64_t to_long() const
//     {
//         return static_cast<std::int64_t>(value);
//     }
// };

TEST_CASE("concepts + class templates")
{
    DataHolder<int> i{42};
    CHECK(i.value == 42);
    //CHECK(i.to_long() == 42ll);
    i.print();

    DataHolder<std::string> s{"text"};
    CHECK(s.value == "text");
    s.print();
}