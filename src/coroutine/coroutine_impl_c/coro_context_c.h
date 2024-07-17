#pragma once

#include <set>
#include <ucontext.h>
#include "coroutine/coro_context.h"
#include "coroutine/coroutine.h"

namespace bdf {

struct CoroContextList {
  char stack[STACK_SIZE];
  ucontext_t main_ctx;
  int nco;
  int cap;
  CoroContext* running = nullptr;
  // ʵ�ʴ�ŵ���������CoroContextc
  std::set <CoroContext*> coctxs; // ��set������ң��ֲ�����hash setռ����ô���ڴ�
};

struct CoroContextc : public CoroContext {
  CoroutineFunc func;
  void *ud;
  ucontext_t ctx;
  // CoroContextList* ls;
  int ctx_cap;
  int size;
  int status;
  char *stack;
};

}
