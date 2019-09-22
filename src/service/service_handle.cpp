#include <unistd.h>
#include <sys/prctl.h>
#include "service/service_handle.h"
#include "message.h"
#include "handle_data.h"

namespace bdf {

LOGGER_CLASS_IMPL(logger_, ServiceHandler);

void ServiceHandler::Run( HandleData* data){
  prctl(PR_SET_NAME, "ServiceHandlerThread");
  INFO(logger_, "ServiceHandler::Run start.");
  time_t now = time(NULL);
  while (data->is_run) {
    if (data->data_.empty()){
      usleep(1);
      continue;
    }

    std::queue<EventMessage*> temp;
    data->lock_.lock();
    temp.swap(data->data_);
    data->lock_.unlock();
    while (!temp.empty()){
      EventMessage *msg = temp.front();
      temp.pop();
      Handle(msg);
      //for debug
      time_t cur_time = time(NULL);
      if ((cur_time- now)>300){
        INFO(logger_, "msg size:" << data->data_.size()
          <<",handle size:"<< temp.size());
        now = cur_time;
      }
    }
  }
  INFO(logger_, "ServiceHandler::Run exit.");
}

void ServiceHandler::Handle(EventMessage* message) {
  //if (message->status != EventMessage::kStatusOK) {
  //  MessageFactory::Destroy(message);
  //  return;
  //}
  TRACE(logger_, "msg type:" << (int)(message->type_id) << ",direction:" 
    << (int)(message->direction) << ",status:" << (int)(message->status));
  switch (message->type_id) {
  case MessageType::kHttpMessage:
    return message->direction == MessageBase::kIncomingRequest
      ? HttpRequestMessage(static_cast<HttpMessage*>(message))
      : HttpResponseMessage(static_cast<HttpMessage*>(message));
  case MessageType::kRapidMessage:
    return message->direction == MessageBase::kIncomingRequest
      ? RapidRequestMessage(static_cast<RapidMessage*>(message))
      : RapidResponseMessage(static_cast<RapidMessage*>(message));
  default:
    MessageFactory::Destroy(message);
    ERROR(logger_,"unkown message,type_id:"<< message->type_id);
  }
}

void ServiceHandler::HttpRequestMessage(HttpMessage* message){
  OnHttpRequestMessage(message);
}

void ServiceHandler::RapidRequestMessage(RapidMessage* message){
  OnRapidRequestMessage(message);
}

void ServiceHandler::HttpResponseMessage(HttpMessage* message){
  if (nullptr != message->ctx && nullptr != message->ctx->callback){
    if (message->status == EventMessage::kStatusOK){
      monitor::GlobalMatrix::Instance().MarkEnd(message->ctx->monitor_id, true);
    }else{
      monitor::GlobalMatrix::Instance().MarkEnd(message->ctx->monitor_id, false);
    }
    message->ctx->callback(message);
  }else{
    OnHttpResponseMessage(message);
  }
}

void ServiceHandler::RapidResponseMessage(RapidMessage* message){
  if (nullptr != message->ctx && nullptr != message->ctx->callback){
    if (message->status == EventMessage::kStatusOK) {
      monitor::GlobalMatrix::Instance().MarkEnd(message->ctx->monitor_id, true);
    } else {
      monitor::GlobalMatrix::Instance().MarkEnd(message->ctx->monitor_id, false);
    }
    message->ctx->callback(message);
  } else {
    OnRapidResponseMessage(message);
  }
}

void ServiceHandler::OnHttpRequestMessage(HttpMessage* message){
  MessageFactory::Destroy(message);
  WARN(logger_, "not should here,not implement:" << message->type_id);
}

//if use coroutine,should not handle
void ServiceHandler::OnHttpResponseMessage(HttpMessage* message){
  TRACE(logger_, "not should here,not implement:" << message->type_id);
  MessageFactory::Destroy(message);
}

void ServiceHandler::OnRapidRequestMessage(RapidMessage* message){
  WARN(logger_, "not should here,not implement:" << message->type_id);
  MessageFactory::Destroy(message);
}

//if use coroutine,should not handle
void ServiceHandler::OnRapidResponseMessage(RapidMessage* message){
  TRACE(logger_, "not should here,not implement:" << message->type_id);
  MessageFactory::Destroy(message);
}

}
