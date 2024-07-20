#pragma once

#include <unordered_set>

namespace bdf {

 typedef void (*CoroutineFunc)(void*);
#define STACK_SIZE (256*1024)
#define DEFAULT_COROUTINE 512

 class CoroContext;

class Coroutine {
public:
  enum {
    kCoroutineInvalid = 0,
    kCoroutineReady = 1,
    kCoroutineRunning = 2,
    kCoroutineSuspend = 3,
  };

  enum {
    kCoroutineTypeC = 0, 
  };

  virtual int CoroutineSize() { return 0; }
  virtual bool CoroutineInit(
    CoroutineFunc func, void* data, 
    std::unordered_set<CoroContext*>& free_list, int coroutine_size = DEFAULT_COROUTINE) {
    return false;
  }
  virtual void Release() {}

  // ����Э��
  virtual CoroContext* CoroutineNew(CoroutineFunc, void *ud) { return nullptr; }

  // ��ȡ��ǰ�����е�Э��
  virtual CoroContext* GetCurrentCoroutine() { return nullptr; }

  virtual int CoroutineStatus(CoroContext* coro) { return 0; }

  //����Э��
  virtual void CoroutineStart(CoroContext* coro, CoroutineFunc func = nullptr, void* data = nullptr) {}

  //�������߻ָ������Э��
  virtual bool CoroutineResume(CoroContext* coro) { return false; }

  //���������е�Э��
  virtual void CoroutineYield() {}
};

} 
