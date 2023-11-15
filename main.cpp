#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <functional>
#include <vector>
#include <mutex>
#include <future>

std::mutex calls_mutex;

int universal_function(int n, int function_number, std::vector<std::function<int(int)>> &functions, int* calls);

void log_and_sleep(int function_number, int* calls) {
    {
        std::lock_guard<std::mutex> guard(calls_mutex);
        calls[function_number - 1]++;
        std::cout << "I'm function " << function_number << ", currently working" << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 100 + 10));
}

int universal_function(int n, int function_number, std::vector<std::function<int(int)>> &functions, int* calls) {
    log_and_sleep(function_number, calls);
    if (n <= 0) return 1;

    int random_index1 = rand() % 4;
    int random_index2 = rand() % 4;

    std::promise<int> promise1;
    std::promise<int> promise2;
    std::future<int> future1 = promise1.get_future();
    std::future<int> future2 = promise2.get_future();

    std::thread thread1([&functions, &promise1, n, random_index1]() {
        promise1.set_value(functions[random_index1](n - 1));
    });

    std::thread thread2([&functions, &promise2, n, random_index2]() {
        promise2.set_value(functions[random_index2](n / 3));
    });

    thread1.join();
    thread2.join();

    return future1.get() + future2.get();
}

int main() {
    srand(time(NULL));
    int n;
    std::cout << "Enter value n: ";
    if (!(std::cin >> n)) {
        std::cerr << "Invalid input for n" << std::endl;
        return 1;
    }

    int calls[4] = {0, 0, 0, 0};

    std::vector<std::function<int(int)>> functions = {
            std::bind(universal_function, std::placeholders::_1, 1, std::ref(functions), calls),
            std::bind(universal_function, std::placeholders::_1, 2, std::ref(functions), calls),
            std::bind(universal_function, std::placeholders::_1, 3, std::ref(functions), calls),
            std::bind(universal_function, std::placeholders::_1, 4, std::ref(functions), calls)
    };

    functions[0](n);

    for (int i = 0; i < 4; i++) {
        std::cout << "Function " << (i + 1) << " was called " << calls[i] << " times." << std::endl;
    }

    return 0;
}