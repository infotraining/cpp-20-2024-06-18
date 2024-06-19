#include <catch2/catch_test_macros.hpp>
#include <concepts>
#include <iostream>
#include <map>
#include <set>
#include <span>
#include <string>
#include <vector>
#include <numeric>

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
        ptr != nullptr;
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
struct ContainerValueType
{
    using value_type = typename T::value_type;
};

template <typename T, size_t N>
struct ContainerValueType<T[N]>
{
    using value_type = T;
};

template <typename T>
using ContainerValueType_t = typename ContainerValueType<T>::value_type;

template <typename T>
concept Range = requires(T&& arg) {
    std::ranges::begin(arg);
    std::ranges::end(arg);
};

static_assert(Range<std::vector<int>>);
static_assert(Range<int[10]>);

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
    for (const auto& item : rng)
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

    void print() const
        requires PrintableRange<T>
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
    // CHECK(i.to_long() == 42ll);
    i.print();

    DataHolder<std::string> s{"text"};
    CHECK(s.value == "text");
    s.print();
}

std::unsigned_integral auto get_id()
{
    static uint64_t id_ = 0;
    return ++id_;
}

TEST_CASE("auto + concepts")
{
    std::unsigned_integral auto id = get_id();          // compiler checks std::unsigned_integral<decltype(id)>
    std::convertible_to<uint64_t> auto id_2 = get_id(); // compiler checks: std::convertible_to<decltype(id_2), uint64_t>
}

void insert_into_container(auto& container, auto&& item)
{
    if constexpr (requires { container.push_back(std::forward<decltype(item)>(item)); }) // requires expression
    {
        container.push_back(std::forward<decltype(item)>(item));
    }
    else
    {
        container.insert(std::forward<decltype(item)>(item));
    }
}

template <typename T>
concept LeanPointer = requires(T ptr) {
    *ptr;
    ptr == nullptr;
    ptr != nullptr;
    requires sizeof(T) == sizeof(int*); // now it is evaluated to true or false
};

static_assert(!LeanPointer<std::shared_ptr<int>>);
static_assert(LeanPointer<std::unique_ptr<int>>);

template <typename T>
concept Indexable = requires(T obj, size_t index) {
    { obj[index] } -> std::same_as<int>;
    { obj.size() } noexcept -> std::convertible_to<size_t>;
};

TEST_CASE("requires expression")
{
    std::vector<int> vec;
    insert_into_container(vec, 42);
    CHECK(vec.front() == 42);

    std::set<int> my_set;
    insert_into_container(my_set, 42);
}

//////////////////////////////////////////////////////////////

namespace TooComplex
{

    template <Range TRange>
        requires requires(std::ranges::range_value_t<TRange> a) { a + a; }
    && requires { std::ranges::range_value_t<TRange>{}; }

    auto sum(const TRange& data)
    {
        return std::accumulate(std::ranges::begin(data), std::ranges::end(data),
            std::ranges::range_value_t<TRange>{});
    }

} // namespace TooComplex

template <typename T>
concept Addable = requires(T a, T b) { a + b; };

template <Range TRange>
    requires Addable<std::ranges::range_value_t<TRange>> 
             && std::default_initializable<std::ranges::range_value_t<TRange>>
auto sum(const TRange& data)
{
    return std::accumulate(std::ranges::begin(data), std::ranges::end(data),
        std::ranges::range_value_t<TRange>{});
}