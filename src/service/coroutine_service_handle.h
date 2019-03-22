
#pragma once

#include <queue>
#include "event/timer/timer.h"
#include "event/timer/timer_base.h"
#include "service/service_handle.h"

namespace bdf { 

class HandleData;
class CoroutineActor;

class CoroutineServiceHandler : public ServiceHandler, public OnTimerBase {
 public:

  virtual void Run(HandleData* data);

  virtual ServiceHandler* Clone() {
    return new CoroutineServiceHandler();
  }

protected:
  virtual void OnTimer(void* function_data);
private:
  LOGGER_CLASS_DECL(logger_);

  static void Process(CoroutineActor* coroutine, void* data);
  void ProcessTimer();
  void Process(HandleData* data);

  void ProcessTask(HandleData* data);

  void ProcessMessage(std::queue<EventMessage*>& msg_list);

  int current_coroutine_id_ = -1;

  Timer timer_;
};

}
