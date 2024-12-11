

#include <windows.h>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <atomic>
#include <iomanip>

// Количество философов
const int PHILOSOPHERS_COUNT = 5;

// Время на действия
const DWORD SIMULATION_TIME_MS = 10000; // Время симуляции в миллисекундах

// Массив критических секций для вилок
CRITICAL_SECTION forks[PHILOSOPHERS_COUNT];

// Статистика
struct PhilosopherStats {
    std::atomic<int> eat_count{ 0 };
    std::atomic<DWORD> wait_time_ms{ 0 };
    std::atomic<DWORD> eat_time_ms{ 0 };
};

PhilosopherStats stats[PHILOSOPHERS_COUNT];

// Функция для моделирования действий философа
DWORD WINAPI philosopher(LPVOID param) {
    int id = reinterpret_cast<int>(param);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(500, 1500);

    DWORD start_time = GetTickCount64();

    while (true) {
        // Проверяем, не завершилась ли симуляция
        DWORD elapsed_time = GetTickCount64() - start_time;
        if (elapsed_time > SIMULATION_TIME_MS) {
            break;
        }

        // Философ думает
        Sleep(dis(gen));

        // Время ожидания начинается
        DWORD wait_start = GetTickCount64();

        // Берем вилки (левая и правая)
        int leftFork = id;
        int rightFork = (id + 1) % PHILOSOPHERS_COUNT;

        // Решение deadlock: философы с четными ID берут вилки в другом порядке
        if (id % 2 == 0) {
            std::swap(leftFork, rightFork);
        }

        EnterCriticalSection(&forks[leftFork]);
        EnterCriticalSection(&forks[rightFork]);

        // Время ожидания заканчивается
        DWORD wait_end = GetTickCount64();
        stats[id].wait_time_ms += (wait_end - wait_start);

        // Философ ест
        DWORD eat_start = GetTickCount64();
        Sleep(dis(gen));
        DWORD eat_end = GetTickCount64();

        stats[id].eat_count++;
        stats[id].eat_time_ms += (eat_end - eat_start);

        // Освобождаем вилки
        LeaveCriticalSection(&forks[rightFork]);
        LeaveCriticalSection(&forks[leftFork]);
    }

    return 0;
}

void printStats() {
    std::cout << "\nStats:\n";
    std::cout << std::setw(12) << "Phil"
        << std::setw(12) << "Times eated"
        << std::setw(18) << "Time to eat (ms)"
        << std::setw(20) << "Wait time (ms)\n";

    for (int i = 0; i < PHILOSOPHERS_COUNT; ++i) {
        std::cout << std::setw(12) << i
            << std::setw(12) << stats[i].eat_count
            << std::setw(18) << stats[i].eat_time_ms.load()
            << std::setw(20) << stats[i].wait_time_ms.load() << "\n";
    }
}

int main() {
    // Инициализация критических секций
    for (int i = 0; i < PHILOSOPHERS_COUNT; ++i) {
        InitializeCriticalSection(&forks[i]);
    }

    // Создание потоков философов
    HANDLE threads[PHILOSOPHERS_COUNT];
    for (int i = 0; i < PHILOSOPHERS_COUNT; ++i) {
        threads[i] = CreateThread(nullptr, 0, philosopher, reinterpret_cast<LPVOID>(i), 0, nullptr);
    }

    // Ожидание завершения потоков
    WaitForMultipleObjects(PHILOSOPHERS_COUNT, threads, TRUE, INFINITE);

    // Закрытие дескрипторов потоков
    for (int i = 0; i < PHILOSOPHERS_COUNT; ++i) {
        CloseHandle(threads[i]);
    }

    // Уничтожение критических секций
    for (int i = 0; i < PHILOSOPHERS_COUNT; ++i) {
        DeleteCriticalSection(&forks[i]);
    }

    // Печать статистики
    printStats();

    return 0;
}
