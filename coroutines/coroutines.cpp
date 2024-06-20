#include <catch2/catch_test_macros.hpp>
#include <coroutine>
#include <format>
#include <iostream>
#include <queue>
#include <random>
#include <string>
#include <utility>
#include <vector>

using namespace std::literals;

class Task
{
public:
    struct promise_type
    {
        Task get_return_object()
        {
            return Task(std::coroutine_handle<promise_type>::from_promise(*this));
        }

        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() { }
        void unhandled_exception() { std::terminate(); }
    };
    
    Task(std::coroutine_handle<promise_type> coro_handle)
        : coro_handle_{coro_handle}
    { }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    Task(Task&& other)
        : coro_handle_{std::exchange(other.coro_handle_, nullptr)}
    { }

    ~Task()
    {
        if (coro_handle_)
            coro_handle_.destroy();
    }

    auto get_coro_handle() const
    {
        return coro_handle_;
    }

private:
    std::coroutine_handle<promise_type> coro_handle_;
};

class Scheduler
{
public:
    void submit_coro(auto coro_handle)
    {
        suspended_coroutines_.push(coro_handle);
    }

    void submit_task(Task& task)
    {
        suspended_coroutines_.push(task.get_coro_handle());
    }

    auto fetch_data()
    {
        struct ValueAwaiter
        {
            Scheduler& scheduler;

            bool await_ready() { return false; }

            void await_suspend(std::coroutine_handle<> coro_handle)
            {
                scheduler.submit_coro(coro_handle);
            }

            int await_resume()
            {
                std::random_device rd;
                std::mt19937 rnd_gen(rd());
                std::uniform_int_distribution<> distr(1, 100);
                return distr(rnd_gen);
            }
        };

        return ValueAwaiter{*this};
    }

    void run()
    {
        while (!suspended_coroutines_.empty())
        {
            auto coro_handle = suspended_coroutines_.front();
            suspended_coroutines_.pop();

            if (!coro_handle.done())
            {
                coro_handle.resume();
                suspended_coroutines_.push(coro_handle);
            }
        }
    }

private:
    std::queue<std::coroutine_handle<void>> suspended_coroutines_;
};

Task async_coro(int id, Scheduler& scheduler)
{
    std::cout << std::format("async_coro#{} start", id) << std::endl;

    int value1 = co_await scheduler.fetch_data();
    std::cout << std::format("async_coro#{} got value: {}", id, value1) << std::endl;

    std::cout << "async_coro middle" << std::endl;

    int value2 = co_await scheduler.fetch_data();
    std::cout << std::format("async_coro#{} got value: {}", id, value2) << std::endl;

    std::cout << std::format("async_coro#{} end", id) << std::endl;

    co_return;
}

TEST_CASE("first coroutine")
{
    Scheduler scheduler;

    Task task1 = async_coro(1, scheduler);
    Task task2 = async_coro(2, scheduler);
    Task task3 = async_coro(3, scheduler);

    scheduler.submit_task(task1);
    scheduler.submit_task(task2);
    scheduler.submit_task(task3);

    scheduler.run();
}
