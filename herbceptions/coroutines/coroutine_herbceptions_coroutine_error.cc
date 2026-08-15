#include <coroutine>
#include <exception>
#include <iostream>
#include "../error.h"
#include "coroutine_error.h"

struct Task {
    struct promise_type {
        ::std::coroutine_error err;

        Task get_return_object() {
            return Task{ std::coroutine_handle<promise_type>::from_promise(*this) };
        }

        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        void return_void() {}

        void unhandled_herbception(::std::coroutine_error e) noexcept {
            std::cout << "promise_type::unhandled_exception() called\n";
            err = ::std::move(e);
        }
    };

    std::coroutine_handle<promise_type> h;

    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<>) const noexcept {}
    void await_resume() throws {
        if (h.promise().err)
            throw throws h.promise().err; //only allow in task type??
    }
};

inline Task foo() throws  //mark throws on coroutine means noexcept and will throws herbceptions. Promise_type will have unhandled_herbception not unhandled_exception.
{
    std::cout << "foo(): to throw eh\n";
    throw throws ::std::errc::file_not_found;
    co_return;
}

int main() {
    try {
        co_await foo();
    } catch throws(::std::error e) {
//        std::cerr << "main(): caught eh from coroutine: " << e.what() << "\n";
    }
}
