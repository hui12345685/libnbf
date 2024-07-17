#pragma once

#include <iostream>
#include "coroutine/coroutine_schedule.h"

namespace bdf {

struct CoroutineActor;
class CoroutineSchedule;
class CoroutineServiceHandler;
class TimerMgr;

// ÿһ���õ����߳�һ��CoroutineManager����
// ���Ծ�ע����ʹ��CoroutineManager�Ķ�����Ҫ���߳�������
struct CoroutineManager {
 public:

  static CoroutineManager& Instance() {
    static thread_local CoroutineManager context_;
    return context_;
  }

  // ÿһ��CoroutineManager�������һ����ʱ������
  static void Init(
    CoroutineServiceHandler* service_handle,
    TimerMgr* time_mgr);

  inline static CoroContext* GetCurrentCoroutine() {
    return Instance().scheduler_->GetCurrentCoroutine();
  }

  inline static void SetCurrentCoroutine(CoroContext* coro) {
    //TODO
  }

  inline static CoroutineSchedule* GetScheduler() {
    return Instance().scheduler_;
  }

  inline static TimerMgr* GetTimerMgr() {
    return Instance().time_mgr_;
  }

  inline static CoroutineServiceHandler* GetServiceHandler() {
    return Instance().service_handle_;
  }

 private:
   CoroutineManager(){
  }

  ~CoroutineManager() {
    if (nullptr != scheduler_){
      delete scheduler_;
    }
  }

  DISALLOW_COPY_AND_ASSIGN(CoroutineManager);

  CoroutineSchedule* scheduler_ = nullptr;
  CoroutineServiceHandler* service_handle_ = nullptr;
  TimerMgr* time_mgr_ = nullptr;
};

}
