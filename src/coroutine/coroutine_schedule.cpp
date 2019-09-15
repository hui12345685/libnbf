#include "coroutine_schedule.h"
#include "coroutine_actor.h"
#include "coroutine_context.h"
#include "common/thread_id.h"

namespace bdf {

LOGGER_CLASS_IMPL(logger_, CoroutineSchedule);

CoroutineSchedule::~CoroutineSchedule(){
  if (nullptr != coro_sche_){
    coro_impl_.CoroutineClose(coro_sche_);
  }
}

void CoroutineSchedule::InitCoroSchedule(
  CoroutineFunc func, void* data, int coroutine_size){

  coro_sche_ = coro_impl_.CoroutineInit(coroutine_size);
  int max_coro_id = -1;
  for (int idx = 0; idx < coro_sche_->cap;idx++) {
    int coroutine_id = coro_impl_.CoroutineNew(coro_sche_, func, data);
    all_coro_list_.emplace_back(coroutine_id);
    available_coro_list_.insert(coroutine_id);
    if (coroutine_id > max_coro_id){
      max_coro_id = coroutine_id;
    }
  }

  CoroutineID::GetInst().InitAllIds(max_coro_id+1);
  yield_time_debug_.resize(max_coro_id+1);
}

int CoroutineSchedule::GetAvailableCoroId(){
  ////for debug begin
  //static thread_local time_t now = time(NULL);
  //time_t cur_time = time(NULL);
  //if ((cur_time - now) > 60) {
  //  //1����һ��trace
  //  INFO(logger_, "ThreadId:" << ThreadId::Get()
  //    <<",AvailableCoroId size:" << available_coro_list_.size());
  //  now = cur_time;
  //}
  ////for debug end

  for (const auto id: available_coro_list_){
    return id;
  }

  WARN(logger_, "ThreadId:" << ThreadId::Get() 
    << ",no available coro,coro size:" << available_coro_list_.size());
  return -1;
}

void CoroutineSchedule::ProcessDebug(){
  static thread_local time_t now = time(NULL);
  //static thread_local time_t now_1 = now;
  time_t cur_time = time(NULL);
  if ((cur_time - now) > 60) {
    //60��һ��trace
    INFO(logger_, "ThreadId:" << ThreadId::Get()
      << ",AvailableCoroId size:" << available_coro_list_.size());
    now = cur_time;
    for (size_t idx = 0; idx < yield_time_debug_.size(); idx++) {
      if (yield_time_debug_[idx].is_yield) {
        int yield_times = cur_time - yield_time_debug_[idx].yield_time;
        if (yield_times >= 10) {
          INFO(logger_, "ThreadId:" << ThreadId::Get() << ",try to resume coro id:"
            << idx << ",yield times:" << yield_times);
        }
      }
    }
  }
  //10��20���ڵ���һ�ξ���
  //if ((cur_time - now_1) > 10) {
  //  now_1 = cur_time;
  //  for (size_t idx = 0; idx < yield_time_debug_.size(); idx++) {
  //    if (yield_time_debug_[idx].is_yield) {
  //      int yield_times = cur_time - yield_time_debug_[idx].yield_time;
  //      if (yield_times >= 10) {
  //        INFO(logger_, "ThreadId:" << ThreadId::Get() << ",try to resume coro id:"
  //          << idx << ",yield times:" << yield_times);
  //        CoroutineYieldToActive(idx);
  //      } else {
  //        TRACE(logger_, "ThreadId:" << ThreadId::Get() << ",coro id:" << idx
  //          << ",yield times:" << yield_times);
  //      }
  //    }
  //  }
  //}
}

void CoroutineSchedule::CoroutineYield(int coro_id){
  available_coro_list_.erase(coro_id);
  if (coro_id == GetRunningId()){
    TRACE(logger_, "will be yield coroutine:"<< coro_id);
    yield_time_debug_[coro_id].is_yield = true;
    yield_time_debug_[coro_id].yield_time = time(nullptr);
    CoroutineYield();
  } else {
    WARN(logger_, "coro_id:" << coro_id<<",GetRunningId:"<< GetRunningId());
  }
}

//����ǰЭ�̣������յ���Ϣ��Э�̼���´���������
bool CoroutineSchedule::CoroutineYieldToActive(int coro_id){
  if (coro_id < 0 || coro_id >= CoroutineSize()) {
    return false;
  }

  if (GetRunningId() == coro_id) {
    //�ȷ����ˣ��������յ���timeoutô?
    WARN(logger_, "may be timeout,running id:"<< GetRunningId());
    return false;
  }

  //˵��Ҫ�л�Э�̣��л�����֮��������յ��˿ͻ��˵���Ϣ���߳�ʱ
  active_coro_list_.emplace(coro_id);
  //if (!available_coro_list_.insert(coro_id).second) {
  //  INFO(logger_, "repeated resume coro id:" << coro_id);
  //  //return false;
  //}
  if (GetRunningId() >= 0) {
    //��ǰ����Э���У��г�ȥ
    TRACE(logger_, "cur coro id:" << GetRunningId() << ",change to coro id:" << coro_id);
    CoroutineYield();
    return true;
  } else{
    WARN(logger_, "warn CoroutineYieldToActive coro id:" << coro_id);
    return false;
  }
}

bool CoroutineSchedule::AfterYieldToAvailable(int coro_id){
  yield_time_debug_[coro_id].is_yield = false;
  yield_time_debug_[coro_id].yield_time = time(nullptr);
  if (!available_coro_list_.insert(coro_id).second) {
    INFO(logger_, "repeated resume coro id:" << coro_id);
    //return false;
  }
  return true;
}

CoroutineActor* CoroutineSchedule::GetCoroutineCtx(int id){
  return coro_impl_.GetCoroutineCtx(coro_sche_,id);
}

//����Э�̻��߻ָ��з��ص�Э��,����Ϣ���أ������л�����
bool CoroutineSchedule::CoroutineResumeActive(){
  if (GetRunningId() >= 0){
    INFO(logger_, "not to here,cur coro id:" << GetRunningId());
    return false;
  }
  if (active_coro_list_.size()>0){
    int id = active_coro_list_.front();
    TRACE(logger_, "change to coro id:" << id);
    active_coro_list_.pop();
    if (id < 0 || id >= CoroutineSize()) {
      WARN(logger_, "error coro id:" << id);
      return false;
    }
    CoroutineResume(id);
    return true;
  }
  return false;
}

void CoroutineSchedule::CoroutineResume(int id) {
  //ҵ���߼�����Э���д������Ե��øú���һ������Э����
  //if (GetRunningId() >= 0) {
  //  //�����ǰ����Э���У����г�ȥ
  //  INFO(logger_, "cur coro id:" << GetRunningId() << ",change to coro id:" << id);
  //  CoroutineContext::SetCurCoroutineId(-1);
  //  CoroutineYield();
  //}
  if (id < 0 || id >= CoroutineSize()){
    WARN(logger_,"invalid id:"<<id);
    return;
  }

  CoroutineContext::SetCurCoroutineId(id);
  TRACE(logger_, "will be resume coroutine:" << id);
  coro_impl_.CoroutineResume(coro_sche_, id);
}

int CoroutineSchedule::GetRunningId(){
  return coro_impl_.CoroutineRunning(coro_sche_);
}

int CoroutineSchedule::CoroutineStatus(int id) {
  return coro_impl_.CoroutineStatus(coro_sche_, id);
}

void CoroutineSchedule::CoroutineYield() {
  CoroutineContext::SetCurCoroutineId(-1);
  coro_impl_.CoroutineYield(coro_sche_);
}

void CoroutineID::InitAllIds(int max_coro_id){
  if (!all_coro_ids_.empty()){
    return;
  }

  //��Ϊ����汾�� coroutine id�Ǵ�0���ε���
  for (int idx = 0; idx < max_coro_id; idx++) {
    int* tmp_id = new int;
    *tmp_id = idx;
    all_coro_ids_.emplace_back(tmp_id);
  }
}

}
