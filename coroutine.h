#ifndef M_COROUTINE_H
#define M_COROUTINE_H
//线程信息的结构
typedef struct
{
    unsigned int id;
    int flag;
    int sleep_end_time;

    void* stack_start;
    void* stack_end;
    void* stack_top_p;

    void* parameters;
    void (*task)(void*);
}MCoroutine;

MCoroutine* alloc_co();
void append_co(MCoroutine* co);
MCoroutine* pop_co();
bool CreateCoroutine(void (*task)(void*), void* args);
void Schedule();
void RunTask();

#define co_yield Schedule();
#define print(str,...) printf(str##"\n",__VA_ARGS__)

#endif //TEST00_COROUTINE_H
