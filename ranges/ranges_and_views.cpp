#include <catch2/catch_test_macros.hpp>
#include <helpers.hpp>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>
#include <map>

using namespace std::literals;

TEST_CASE("ranges", "[ranges]")
{
    auto data = helpers::create_numeric_dataset<20>(42);
    helpers::print(data, "data");

    std::vector words = {"one"s, "two"s, "three"s, "four"s, "five"s, "six"s, "seven"s, "eight"s, "nine"s, "ten"s,
        "eleven"s, "twelve"s, "thirteen"s, "fourteen"s, "fifteen"s, "sixteen"s, "seventeen"s, "eighteen"s, "nineteen"s, "twenty"s};
    helpers::print(words, "words");

    SECTION("algorithms")
    {
        std::sort(words.begin(), words.end());
        std::ranges::sort(words, std::greater{});
        CHECK(std::ranges::is_sorted(words, std::greater{}));

        std::vector<int> negative_numbers;
        std::ranges::copy_if(data, std::back_inserter(negative_numbers), 
            [](int n) { return n < 0; });
        helpers::print(negative_numbers, "negative_numbers");
    }

    SECTION("projections")
    {
        //std::ranges::sort(words, [](const auto& a, const auto& b) { return a.size() < b.size(); });
        //std::ranges::sort(words, std::greater{}, /*projection*/ [](const auto& s) { return s.size(); });
        std::ranges::sort(words, std::greater{}, /*projection*/ std::ranges::size);
        helpers::print(words, "words by length");
    }//

    SECTION("concepts & tools")
    {
        std::vector<int> vec;

        using T = std::ranges::range_value_t<decltype(vec)>;
        static_assert(std::same_as<T, int>);
    }
}

template <auto Value>
struct EndValue
{
    bool operator==(auto it) const
    {
        return *it == Value;
    }
};

TEST_CASE("sentinels", "[ranges]")
{
    std::vector data = {2, 3, 4, 1, 5, 42, 6, 7, 8, 9, 10};

    auto pos = std::ranges::find(data.begin(), std::unreachable_sentinel, 42);
    CHECK(*pos == 42);

    // TODO - sort range [begin; 42) in descending order
    auto sentinel = EndValue<42>{};
    std::ranges::sort(data.begin(), sentinel); 

    EndValue<'\0'> null_term;
    auto& txt = "acbgdef\0ajdhfgajsdhfgkasdjhfg"; // const char(&txt)[30]
    std::string str;
    std::ranges::copy(std::ranges::begin(txt), null_term, std::back_inserter(str));

    helpers::print(data, "data");
}

TEST_CASE("counted_iterator")
{
    std::vector data = {2, 3, 4, 1, 5, 42, 6, 7, 8, 9, 10};

    int target[5];

    std::ranges::copy(std::counted_iterator{data.begin(), 5}, std::default_sentinel, std::ranges::begin(target));
    std::ranges::copy(std::views::counted(data.begin(), 5), std::ranges::begin(target));
}

TEST_CASE("common view")
{
    std::vector data = {2, 3, 4, 1, 5, 42, 6, 7, 8, 9, 10};

    std::ranges::subrange head(std::counted_iterator{data.begin(), 5}, std::default_sentinel);

    for(auto it = head.begin(); it != head.end(); ++it)
    {
        std::cout << *it << " ";
    }
    std::cout << "\n";
    
    auto harmonized_head = std::ranges::common_view(head);
    std::vector<int> target(harmonized_head.begin(), harmonized_head.end());
}

TEST_CASE("views")
{
    std::vector data = {2, 3, 4, 1, 5, 42, 6, 7, 8, 9, 10};

    SECTION("all")
    {
        auto all_items = std::views::all(data);
        helpers::print(all_items, "all_items");
    }

    SECTION("subrange - iterator & sentinel as a view")
    {
        auto& txt = "acbgdef\0ajdhfgajsdhfgkasdjhfg"; // const char(&txt)[30]
        auto token = std::ranges::subrange(std::ranges::begin(txt), EndValue<'\0'>{});
        helpers::print(token, "token");
    }

    SECTION("counted")
    {
        auto head_3 = std::views::counted(data.begin(), 3);
        helpers::print(head_3, "head_3");
    }

    SECTION("iota")
    {
        auto gen = std::views::iota(1);

        auto it = gen.begin();

        std::cout << *it << "\n";
        ++it;
        std::cout << *it << "\n";        
    }

    SECTION("pipes |")
    {
        auto items = std::views::iota(1) 
            | std::views::take(10) 
            | std::views::transform([](int x) { return x * x; })
            | std::views::filter([](int x) { return x % 2 == 0; })
            | std::views::reverse
            | std::views::common;

        for(const auto& item : items)
        {
            std::cout << item << " ";
        }    
        std::cout << "\n";

        std::vector<int> target(items.begin(), items.end()); // copy view to vector
    }

    SECTION("keys - values")
    {
        std::map<int, std::string> dict = { {1, "one"}, {2, "two" } };

        helpers::print(std::views::keys(dict), "keys");
        helpers::print(std::views::values(dict), "values");
        helpers::print(std::views::elements<1>(dict), "values");
    }
}

void print_all(std::ranges::view auto coll)
{
    for(const auto& item : coll)
    {
        std::cout << item << " ";
    }
    std::cout << "\n";
}

TEST_CASE("views - reference semantics")
{
    std::vector data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto evens_view = data | std::views::filter([](int i) { return i % 2 == 0; });
    print_all(evens_view);
    print_all(std::views::all(data));

    helpers::print(data, "data");
}

std::vector<std::string_view> tokenize(std::string_view text, auto separator = ' ')
{
    auto tokens = text 
        | std::views::split(separator) 
        | std::views::transform([](auto token) { return std::string_view(token.begin(), token.end()); });
 
    std::vector<std::string_view> tokens_sv(tokens.begin(), tokens.end());

    return tokens_sv;
}

template <typename T>
std::vector<std::span<T>> tokenize(std::span<T> text, auto separator)
{
    using Token = std::span<T>;

    std::vector<Token> tokens;

    for (auto&& rng : text | std::views::split(separator))
    {
        tokens.emplace_back(rng);
    }

    return tokens;
}

TEST_CASE("split")
{
    std::string str = "abc,def,ghi";

    auto tokens = tokenize(str, ',');

    helpers::print(tokens, "tokens");

    auto span_tokens = tokenize(std::span{str}, ',');

    span_tokens[1][0] = 'Z';

    std::cout << "str: " << str << "\n";
}