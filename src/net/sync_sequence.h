#pragma once

#include <list>
#include <mutex>
#include "common/logger.h"
#include "event/timer/timer.h"

namespace bdf {

class EventMessage;
class SyncClientConnect;

class SyncSequence :public OnTimerBase {
public:
  SyncSequence(SyncClientConnect* sync_client_con,uint32_t timeout_in_ms);
  virtual ~SyncSequence();

  int Put(EventMessage* message);
  EventMessage* Get();

  int Size(){
    return list_.size();
  }

  virtual void OnTimer(void* function_data);
  std::list<EventMessage*> Clear();

  void ClearTimer();

private:
  SyncClientConnect* sync_client_con_;
  uint32_t timeout_ms_;
  std::list<EventMessage*> list_;

  Timer timer_;
  bool time_check_started_;

  void StartTimeCheck();

  LOGGER_CLASS_DECL(logger_);

  //��������ֻ��service handle���߳��й�ϵ����io handle�߳��޹أ��������
  //Ӧ�ÿ����Ż�
  std::mutex lock_;

  std::mutex time_lock_;

};

}
