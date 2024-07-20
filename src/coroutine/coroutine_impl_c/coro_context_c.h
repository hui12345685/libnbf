#pragma once

#include <set>
#include <ucontext.h>
#include "coroutine/coro_context.h"
#include "coroutine/coroutine.h"

namespace bdf {

struct CoroContextList {
  char stack[STACK_SIZE]; // ����ջ
  ucontext_t main_ctx;
  int nco;
  int cap;
  CoroContext* running = nullptr;
  // ʵ�ʴ�ŵ���������CoroContextc
  std::set <CoroContext*> coctxs; // ��set������ң��ֲ�����hash setռ����ô���ڴ�
};

struct CoroContextc : public CoroContext {
  CoroutineFunc init_func = nullptr; // ֧�ֳ�ʼ����ʱ��Э�̺���
  CoroutineFunc start_func = nullptr; //Ҳ֧������Э�̵�ʱ��Э�̺���
  void *ud;
  ucontext_t ctx;
  int ctx_cap;
  int size; // ջ��С
  int status;
  char *stack;
};

}
