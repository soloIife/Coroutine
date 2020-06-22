#include <iostream>
#include <windows.h>
#include <list>
#include "coroutine.h"
//Define the size of the coroutine stack
#define COROUTINE_STACK_SIZE 1024*1024*1 // 1M stack
#define MAX_COROUTINE 1000 // max coroutine num

//Coroutine status flag
enum FLAGS
{
    COROUTINE_CREATE = 0x1,
    COROUTINE_READY = 0x2,
    COROUTINE_EXIT = 0x3,
};

std::list<MCoroutine*> GlobalCoroutineList; // Global Coroutine List
static int CoroutineRunningCount = 0; // the Running count of Coroutine
static MCoroutine cor_main = { 0,COROUTINE_EXIT }; //main thread coroutine
static MCoroutine* CurrentCoroutine = &cor_main; //the current running coroutine

//Assign a coroutine
MCoroutine* alloc_co() {
    if (GlobalCoroutineList.size() > MAX_COROUTINE)
    {
        return nullptr;
    }
    auto* co = new MCoroutine();
    GlobalCoroutineList.push_back(co);
    return co;
}

// append a coroutine to Global Coroutine List
void append_co(MCoroutine* co) {
    GlobalCoroutineList.push_back(co);
}

// pop a coroutine from Global Coroutine List
MCoroutine* pop_co() {
    if (GlobalCoroutineList.empty()) {
        return nullptr;
    }
    MCoroutine* co;
    co = GlobalCoroutineList.front();
    GlobalCoroutineList.pop_front();
    return co;
}

//This function will be run for the first time, and the task will be called by this function
static void coroutine_startup(MCoroutine* coroutine) {
    CoroutineRunningCount++;
    coroutine->task(coroutine->parameters);
    coroutine->flag = COROUTINE_EXIT;
    CoroutineRunningCount--;
    Schedule();
}

static void push_stack(unsigned int** stack_top_p_p, unsigned int val) {
    *stack_top_p_p -= 1;
    **stack_top_p_p = val;
}

static bool init_stack(MCoroutine* coroutine) {
    unsigned char* stack_pages;
    unsigned int* stack_top_p;
    stack_pages = (unsigned char*)VirtualAlloc(nullptr, COROUTINE_STACK_SIZE, MEM_COMMIT, PAGE_READWRITE);
    if (stack_pages == nullptr) return false;
    coroutine->stack_start = stack_pages + COROUTINE_STACK_SIZE;
    coroutine->stack_end = stack_pages;
    stack_top_p = (unsigned int*)coroutine->stack_start;
    push_stack(&stack_top_p, (unsigned int)coroutine);//
    push_stack(&stack_top_p, 0);//填充对齐
    push_stack(&stack_top_p, (unsigned int) coroutine_startup);//
    push_stack(&stack_top_p, 1);//ebp
    push_stack(&stack_top_p, 2);
    push_stack(&stack_top_p, 3);
    push_stack(&stack_top_p, 4);
    push_stack(&stack_top_p, 5);
    push_stack(&stack_top_p, 6);
    push_stack(&stack_top_p, 8);//eax
    coroutine->stack_top_p = stack_top_p;
    coroutine->flag = COROUTINE_READY;
    return true;
}

static unsigned int ID_NUM=1;
static unsigned int get_id(){
    return ID_NUM++;
}

bool CreateCoroutine(void (*task)(void*), void* args) {
    MCoroutine* coroutine = alloc_co();
    if (coroutine==nullptr) return false;
    coroutine->flag = COROUTINE_CREATE;
    coroutine->id = get_id();
    coroutine->task = task;
    coroutine->parameters = args;
    return init_stack(coroutine);
}

__declspec(naked) void switch_context(MCoroutine* cur_coroutine, MCoroutine* dst_coroutine)
{
    __asm {
    push ebp
    mov ebp, esp
    push edi
    push esi
    push ebx
    push ecx
    push edx
    push eax

    mov esi, cur_coroutine
    mov edi, dst_coroutine
    mov[esi + MCoroutine.stack_top_p], esp
    /// Classic thread switch, another coroutine resurrection
    mov esp, [edi + MCoroutine.stack_top_p]

    pop eax
    pop edx
    pop ecx
    pop ebx
    pop esi
    pop edi
    pop ebp
    ret
    }
}

void Schedule() {
    MCoroutine* src_coroutine;
    MCoroutine* dst_coroutine;
    dst_coroutine = pop_co();
    if (dst_coroutine == nullptr) {
        dst_coroutine = &cor_main;
    }
    if (CurrentCoroutine->flag != COROUTINE_EXIT) {
        append_co(CurrentCoroutine);
    }
    src_coroutine = CurrentCoroutine;
    CurrentCoroutine = dst_coroutine;
    switch_context(src_coroutine, dst_coroutine);
    //print("switch_context over: src_coroutine:%d,dst_coroutine:%d", src_coroutine->id, dst_coroutine->id);
}

void RunTask() {
    do {
        Schedule();
        //print("current coroutine num: %d", CoroutineRunningCount);
    } while (CoroutineRunningCount);
}
