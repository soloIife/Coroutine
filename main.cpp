#include "stdafx.h"
#include "coroutine.h"

void task(void* num) {
    print("Coroutine%d start...", (int)num);
    for (int i = 0; i < 1000; ++i) {
        print("Coroutine%d_%d", (int)num, i);
        co_yield
    }
}

int main() {
    print("start...");
    for (int i = 0; i < 10; i++)
    {
        CreateCoroutine(task,(void*)i);
    }
    RunTask();
    return 0;
}
