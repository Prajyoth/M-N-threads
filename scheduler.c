#define _GNU_SOURCE
#include<sched.h>
#include<stdio.h>
#include<stdlib.h>
#include <atomic_ops.h>
#include "queue.h"
#include "scheduler.h"

#define STACK_SIZE (1024*1024)

#undef malloc
#undef free
#undef printf
void * safe_mem(int op, void * arg) {
      static AO_TS_t spinlock = AO_TS_INITIALIZER;
      void * result = 0;

      spinlock_lock(&spinlock);
      if(op == 0) {
        result = malloc((size_t)arg);
      } else {
        free(arg);
      }
      spinlock_unlock(&spinlock);
      return result;
    }

/* int  safe_printf(const char * format, ...) {
	static AO_TS_t spinlock = AO_TS_INITIALIZER;
	int result = 0;

	spinlock_lock(&spinlock);
	va_list args;
	va_start(args, format);
	result = vprintf(format, args);
	va_end(args);
	spinlock_unlock(&spinlock);
	return result;
} */
#define malloc(arg) safe_mem(0, ((void*)(arg)))
#define free(arg) safe_mem(1, arg)
//#define printf(format, ...) safe_printf((char*)format, ...)

static AO_TS_t ready_list_lock = AO_TS_INITIALIZER;

struct thread * current_thread;
struct queue ready_list;

void yield() {
    spinlock_lock(&ready_list_lock);
    if (current_thread->state != DONE && current_thread->state != BLOCKED)
{
	current_thread->state = READY;
	thread_enqueue(&ready_list,current_thread);
}
    struct thread * dq = thread_dequeue(&ready_list);
    dq->state = RUNNING;
    struct thread * temp = current_thread;
    set_current_thread(dq);
    dq = temp;
    thread_switch(dq, current_thread);
    spinlock_unlock(&ready_list_lock);
}

void block(AO_TS_t * spinlock) {
    spinlock_lock(&ready_list_lock);
    spinlock_unlock(spinlock);
    if (current_thread->state != DONE && current_thread->state != BLOCKED)
{
        current_thread->state = READY;
        thread_enqueue(&ready_list,current_thread);
}
    struct thread * dq = thread_dequeue(&ready_list);
    dq->state = RUNNING;
    struct thread * temp = current_thread;
    set_current_thread(dq);
    dq = temp;
    thread_switch(dq, current_thread);
    spinlock_unlock(&ready_list_lock);
}

void thread_wrap()
 {
  spinlock_unlock(&ready_list_lock);
    current_thread->initial_function(current_thread->initial_argument);
    current_thread->state = DONE;

    condition_signal(current_thread->c);
    yield();
  spinlock_unlock(&ready_list_lock);
  }

void thread_join(struct thread* t)
{
	//printf("address check in thread join - %x , %x, %x\n", current_thread->m,current_thread->c, current_thread);
	//printf("address check in thread join - %x , %x, %x\n", t->m,t->c, t);

	mutex_lock(current_thread->m);  //is this needed? makes no difference
	while(t->state != DONE)
	{
		condition_wait(t->c,current_thread->m); 
	}
	mutex_unlock(current_thread->m);
}

void mutex_init(struct mutex *m)
{
	m->held = 0;
	m->s = AO_TS_INITIALIZER;
	m->waiting_threads =malloc(sizeof(struct queue));
	m->waiting_threads->head = malloc(sizeof(struct queue_node));
	m->waiting_threads->tail = malloc(sizeof(struct queue_node));
	m->waiting_threads->head=NULL;
	m->waiting_threads->tail=NULL;
}

void mutex_lock(struct mutex *m)
{
	spinlock_lock(&m->s);
	if(m->held == 1)
	{
		current_thread->state = BLOCKED;
		thread_enqueue((m->waiting_threads),current_thread);
		block(&m->s);
		spinlock_lock(&ready_list_lock);
		spinlock_unlock(&m->s);
		yield();
	}
	else
	{
		m->held = 1;
	}
}

void mutex_unlock(struct mutex *m)
{
	spinlock_lock(&m->s);
	if(is_empty(m->waiting_threads))
        {
                m->held = 0;
        }
	else
	{
	struct thread *dq = thread_dequeue(m->waiting_threads);
	spinlock_lock(&ready_list_lock);
	dq->state = READY;
	thread_enqueue(&ready_list,dq);
	spinlock_unlock(&ready_list_lock);
	}
	spinlock_unlock(&m->s);
}

void condition_init(struct condition *c)
{
	c->s = AO_TS_INITIALIZER;
	c->waiting_threads = malloc(sizeof(struct queue));
	c->waiting_threads->head=NULL;
	c->waiting_threads->tail=NULL;
}

void condition_wait(struct condition *c, struct mutex *m)
{
	spinlock_lock(&c->s);
	mutex_unlock(m);
	current_thread->state = BLOCKED;
	thread_enqueue(c->waiting_threads, current_thread);
	block(&c->s);
	spinlock_lock(&ready_list_lock);
	spinlock_unlock(&c->s);
	yield();
}

void condition_signal(struct condition *c)
{
	spinlock_lock(&c->s);
	if(!is_empty(c->waiting_threads))
	{
	struct thread *dq = thread_dequeue(c->waiting_threads);
	spinlock_lock(&ready_list_lock);
	dq->state = READY;
	thread_enqueue(&ready_list,dq);
	spinlock_unlock(&ready_list_lock);
	}
	spinlock_unlock(&c->s);
}

void condition_broadcast(struct condition *c)
{
	spinlock_lock(&c->s);
	while (!is_empty(c->waiting_threads))
	{
		struct thread *dq = thread_dequeue(c->waiting_threads);
		spinlock_lock(&ready_list_lock);
		dq->state = READY;
		thread_enqueue(&ready_list,dq);
		spinlock_unlock(&ready_list_lock);
	}
	spinlock_unlock(&c->s);
}


void spinlock_lock(AO_TS_t * lock) {
  while(AO_test_and_set_acquire(lock)!= AO_TS_SET);
}

void spinlock_unlock(AO_TS_t * lock) {
  AO_CLEAR(lock);
}


int kernel_thread_begin(void * arg)
{
	struct thread ttable;
	ttable.state = RUNNING;
	set_current_thread(&ttable);
	while(1)
	{
		yield();
	}
	return 0;
}

void scheduler_begin()
{
	clone(kernel_thread_begin, malloc(STACK_SIZE) + STACK_SIZE, CLONE_THREAD | CLONE_VM | CLONE_SIGHAND | CLONE_FILES | CLONE_FS | CLONE_IO, (void*)"arg"); 
	set_current_thread(malloc(sizeof(struct thread)));
	current_thread->state = RUNNING;
	current_thread->m = malloc(sizeof(struct mutex));
	current_thread->c = malloc(sizeof(struct condition));
	mutex_init(current_thread->m);
	condition_init(current_thread->c);
	ready_list.head = NULL;
	ready_list.tail = NULL;
}

struct thread* thread_fork(void(*target)(void*), void * arg)
{
spinlock_lock(&ready_list_lock);
//printf("%d\n",*(int*)arg);
struct thread * new_thread;
new_thread = malloc(sizeof(struct thread));
new_thread->stack_pointer = malloc(STACK_SIZE)+STACK_SIZE;
new_thread->initial_function = target;
new_thread->initial_argument = arg;

//mutex related initialization. Pushing structure malloc's out of xxx_init. Main thread has no explicit TCB structure defined.
new_thread->m = malloc(sizeof(struct mutex));
new_thread->c = malloc(sizeof(struct condition));
mutex_init(new_thread->m);
condition_init(new_thread->c);


current_thread->state = READY;
thread_enqueue(&ready_list,current_thread);
new_thread->state = RUNNING;
//printf("address check - %x, %x, %x\n", new_thread->m, new_thread->c, new_thread);
struct thread * thread_to_return = new_thread;
struct thread * temp = current_thread;
set_current_thread(new_thread);
new_thread = temp;
thread_start(new_thread,current_thread);


return thread_to_return;
}

void scheduler_end()
{
spinlock_lock(&ready_list_lock);
    while(is_empty(&ready_list)) {
      spinlock_unlock(&ready_list_lock);
      yield();
      spinlock_lock(&ready_list_lock);
    }
    spinlock_unlock(&ready_list_lock);
}
