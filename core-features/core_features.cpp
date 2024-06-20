#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <set>
#include <source_location>
#include <string>
#include <vector>

using namespace std::literals;

struct Person
{
    int id;
    std::string name;
    double salary;

    bool operator==(const Person&) const = default;
};

template <typename T1, typename T2>
struct Pair
{
    T1 fst;
    T2 snd;
};

// template <typename T1, typename T2> // since C++20 implicitly declared
// Pair(T1, T2) -> Pair<T1, T2>;

TEST_CASE("aggregates")
{
    static_assert(std::is_aggregate_v<Person>);

    Person p1{1, "Jan", 9'000.99};
    Person p2{2};

    Person p3{.id = 42, .salary = 11'000.99}; // designated initializers
    CHECK(p3 == Person{42, "", 11'000.99});

    // Person p4{.salary=7000.99, .id=888};

    auto ptr = std::make_unique<Person>(777, "Adam", 7999.99);

    Person p4(884, "Ewa", 8783.99); // ( ) == { }

    Pair pair_1{10, 4.44};

    auto sl = std::source_location::current();

    std::cout << "File: " << sl.file_name() << "\n"
              << "Function: " << sl.function_name() << "\n"
              << "Line: " << sl.line() << "\n";
}

//////////////////////////////////////////////////////////
// lambda expressions C++20

TEST_CASE("lambdas")
{
    SECTION("<typename T>")
    {
        std::vector<std::string> words;

        auto add_to_vec_1 = [&words](auto&& item) {
            words.emplace_back(std::forward<decltype(item)>(item));
        };

        auto add_to_vec_2 = [&words]<typename T>(T&& item) {
            words.emplace_back(std::forward<T>(item));
        };

        auto pusher = []<typename T>(std::vector<T>& vec) {
            vec.push_back(665);
        };
    }

    SECTION("default init")
    {
        SECTION("before C++20")
        {
            auto cmp_by_value = [](auto a, auto b) {
                return *a < *b;
            };

            std::set<std::shared_ptr<int>, decltype(cmp_by_value)> my_set(cmp_by_value);

            my_set.insert(std::make_shared<int>(42));
            my_set.insert(std::make_shared<int>(1));
            my_set.insert(std::make_shared<int>(665));
            my_set.insert(std::make_shared<int>(65));

            for (const auto& ptr : my_set)
            {
                std::cout << *ptr << " ";
            }
            std::cout << "\n";
        }
    }

    SECTION("since C++20")
    {
        auto cmp_by_value = [](auto a, auto b) {
            return *a < *b;
        };

        static_assert(std::is_default_constructible_v<decltype(cmp_by_value)>); // since C++20

        std::set<std::shared_ptr<int>, decltype(cmp_by_value)> my_set;

        my_set.insert(std::make_shared<int>(42));
        my_set.insert(std::make_shared<int>(1));
        my_set.insert(std::make_shared<int>(665));
        my_set.insert(std::make_shared<int>(65));

        for (const auto& ptr : my_set)
        {
            std::cout << *ptr << " ";
        }
        std::cout << "\n";

        auto file_closer = [](FILE* f) {
            fclose(f);
        };

        std::unique_ptr<FILE, decltype(file_closer)> my_file(fopen("text.txt", "+w"));
        fprintf(my_file.get(), "text");
    }
}

auto create_caller(auto f, auto... args)
{
    return [f, ... args = std::move(args)]() -> decltype(auto) {
        return f(args...);
    };
}

TEST_CASE("capturing parameter pack")
{
    auto calculate = create_caller(std::plus{}, 3, 4);

    CHECK(calculate() == 7);
}