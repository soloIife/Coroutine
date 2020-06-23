#include "coroutine.h"
#include <iostream>

void task(void *num) {
    print("Coroutine%d start...", (int) num);
    for (int i = 0; i < 100; ++i) {
        print("Coroutine%d_%d", (int) num, i);
        co_yield
    }
}
void task01(void *){
    for (int i = 0; i < 1000; i++) {
        CreateCoroutine(task, (void *) i);
        co_yield
    }
}

int main() {
    print("start...");
    CreateCoroutine(task01, nullptr);
    RunTask();
    print("end...");
    return 0;
}
