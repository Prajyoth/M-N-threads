#include<stdio.h>
#include<stdlib.h>
#include <atomic_ops.h>
#define current_thread (get_current_thread())
extern void * safe_mem(int, void*);
extern void * safe_printf(const char*,...);
#define malloc(arg) safe_mem(0, ((void*)(arg)))
#define free(arg) safe_mem(1, arg)
/*#define printf(format, args...) {
  static AO_TS_t spinlock = AO_TS_INITIALIZER; \
  spinlock_lock(&spinlock); \
  const char *A[] = {format}; \
  if(sizeof(A) > 0) {\
    printf(*A,##args); \
  } else {\
    printf("\n"); \
  }\
  spinlock_unlock(&spinlock); \
} */


typedef unsigned char byte;

typedef enum {
RUNNING,
READY,
BLOCKED,
DONE
} state_t;

struct thread{

        byte* stack_pointer;
        void (*initial_function)(void*);
        void* initial_argument;
	state_t state;
	struct mutex *m;
	struct condition *c;
}T;

struct mutex {
	int held;
	struct queue *waiting_threads;
	AO_TS_t s;
};

struct condition {
	struct queue *waiting_threads;
	AO_TS_t s;
};


//mutex operations
void mutex_init(struct mutex *);
void mutex_lock(struct mutex *);
void mutex_unlock(struct mutex *);

//condtion variables operations
void condition_init(struct condition *);
void condition_wait(struct condition *, struct mutex *);
void condition_signal(struct condition *);
void condition_broadcast(struct condition *);

void scheduler_begin();
struct thread* thread_fork(void(*target)(void*), void * arg);
void yield();
void scheduler_end();
void spinlock_lock(AO_TS_t *);
void spinlock_unlock(AO_TS_t *);

extern struct thread * current_thread;
