
#include "coroutine/coroutine_actor.h"
#include "coroutine/coroutine_schedule.h"
#include "coroutine/coroutine_context.h"
#include "event/timer/timer_base.h"
#include "event/timer/timer.h"
#include "service/coroutine_service_handle.h"
#include "message.h"
#include "common/thread_id.h"

namespace bdf{

LOGGER_CLASS_IMPL(logger_, CoroutineActor);

//for test,will be to delete
//////////////////////////////////////////////////////////////////////////
static std::vector<int> coro_id_test;
static void InitCoroTest(){
  if (coro_id_test.size() != 0){
    return;
  }
  for (int idx = 0; idx < 100000;idx++) {
    coro_id_test.emplace_back(idx);
  }
}
//////////////////////////////////////////////////////////////////////////

CoroutineActor::CoroutineActor() :
  is_waiting_(false),
  waiting_id_(-1) {
  msg_list_.clear();
  InitCoroTest();
}

EventMessage* CoroutineActor::RecieveMessage(EventMessage* message,uint32_t timeout_ms){
  TRACE(logger_, "CoroutineActor::RecieveMessage,timeout_ms:"
    << timeout_ms<<",msg_list_:"<<msg_list_.size());
  uint64_t time_id = 0;
  CoroutineSchedule* scheduler = CoroutineContext::Instance().GetScheduler();
  uint64_t seq_id_send_tmp = message->sequence_id;
  int coro_id_tmp = message->coroutine_id;
  if (coro_id_tmp<0 || coro_id_tmp>512){
    WARN(logger_, "error coro_id_tmp:"<< coro_id_tmp);
  }
  if (msg_list_.empty()){
    if (timeout_ms) {
      CoroutineServiceHandler* hdl = CoroutineContext::Instance().GetServiceHandler();
      TimerData data;
      data.function_data = &coro_id_test[coro_id_tmp];
      data.time_proc = hdl;
      //���ﶨʱ������5ms����Ϊ��async_client_connect��sync_sequenceҲ�и���ʱ�����Ǹ�ʱ���Ǳ�׼��
      //��Ҫconnect�еĶ�ʱ���ȴ������������bug�����������ȼ򵥽�������ӳ�5ms
      //���Э���еĶ�ʱ���ȴ������ᵼ����Ϣ˳�򲻶�
      int real_time_out = timeout_ms + 5;
      time_id = CoroutineContext::Instance().GetTimer()->AddTimer(real_time_out, data);
    }
    TRACE(logger_, "ThreadId:"<< ThreadId::Get()
      <<",RecieveMessage CoroutineYield:" << this<<",msg"<< *message);
    scheduler->CoroutineYield(coro_id_tmp);
  }
  is_waiting_ = false;
  waiting_id_ = -1;
  if (0 != time_id){
    CoroutineContext::Instance().GetTimer()->DelTimer(time_id);
  }
  
  if (!msg_list_.empty()) {
    EventMessage* msg = msg_list_.front();
    msg_list_.pop_front();
    if (seq_id_send_tmp != msg->sequence_id) {
      WARN(logger_, "ThreadId:" << ThreadId::Get()
        << ",send message seq_id:" << seq_id_send_tmp << ",resp message:" << *msg);
    }
    return msg;
  } else {
    INFO(logger_, "maybe time out:"<< timeout_ms <<",coro id:" << 
      coro_id_tmp << ",running id:" << scheduler ->GetRunningId()<<",prt:"<< this);
    return nullptr;
  }
}

bool CoroutineActor::SendMessage(EventMessage* message){
  TRACE(logger_, "SendMessage,is_waiting_" << is_waiting_
    << ",sequence id:" << message->sequence_id << ",waiting_id:" << waiting_id_
    << ",msg_list_size:" << msg_list_.size());
  if (is_waiting_ && message->sequence_id != waiting_id_) {
    INFO(logger_, "sequence_id mismatch:" << waiting_id_ 
      << ",msg_list_size:"<< msg_list_ .size()<<",msg:" << *message);
    //ͬ����Э�鳬ʱ�͹ر������ӣ�������������,�����洦��
    //rapidЭ�鳬ʱ֮�󷵻صģ�������ʱ����ԭ��
    //if (is_waiting_ > message->sequence_id){
    //  MessageFactory::Destroy(message);
    //}
    return false;
  }
  TRACE(logger_, "ThreadId:" << ThreadId::Get() << ",respose message:" << *message);
  msg_list_.emplace_back(message);
  waiting_id_ = -1;
  return true;
}

}
