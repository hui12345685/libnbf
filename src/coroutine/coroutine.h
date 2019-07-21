#pragma once

#include <ucontext.h>

#define STACK_SIZE (512*1024)
#define DEFAULT_COROUTINE 128

namespace bdf {

struct CoroutineActor;

struct CoroSchedule {
  char stack[STACK_SIZE];
  ucontext_t main_ctx;
  int nco;
  int cap;
  int running;
  CoroutineActor **coctx;
};

typedef void(*CoroutineFunc)(void*);

struct CoroContext {
  CoroutineFunc func;
  void *ud;
  ucontext_t ctx;
  CoroSchedule* corotine;
  int cap;
  int size;
  int status;
  char *stack;
};

}
