#include <coroutine>
#include <exception>
#include <iostream>

struct Task {
    struct promise_type {
        std::exception_ptr eptr;

        Task get_return_object() {
            return Task{ std::coroutine_handle<promise_type>::from_promise(*this) };
        }

        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        void return_void() {}

        void unhandled_exception() noexcept {
            std::cout << "promise_type::unhandled_exception() called\n";
            eptr = std::current_exception();
        }
    };

    std::coroutine_handle<promise_type> h;

    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<>) const noexcept {}
    void await_resume() {
        if (h.promise().eptr)
            std::rethrow_exception(h.promise().eptr);
    }
};

Task foo() {
    std::cout << "foo(): to throw eh\n";
    throw std::runtime_error("eh in coroutine");
    co_return;
}

int main() {
    try {
        co_await foo();
    } catch (std::exception const& e) {
        std::cerr << "main(): caught eh from coroutine: " << e.what() << "\n";
    }
}
