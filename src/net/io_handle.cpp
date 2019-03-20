
#include <unistd.h>
#include "net/io_handle.h"
#include "message_base.h"
#include "handle_data.h"
#include "net/connect.h"
#include "service/io_service.h"

namespace bdf{

LOGGER_CLASS_IMPL(logger, IoHandler);

thread_local IoHandler* IoHandler::io_handler_ = NULL;

void IoHandler::Run(void* handle, HandleData* data){
  TRACE(logger, "IoHandler::Run start.");
  IoHandler* io_handle = (IoHandler*)handle;
  io_handler_ = io_handle;
  while (data->is_run) {
    io_handle->GetTimer().ProcessTimer();
    if (data->data_.empty()) {
      usleep(10);
      continue;
    }

    std::queue<EventMessage*> temp;
    data->lock_.lock();
    temp.swap(data->data_);
    data->lock_.unlock();

    while (!temp.empty()) {
      EventMessage *msg = temp.front();
      temp.pop();
      io_handle->Handle(msg);
    }
  }
  TRACE(logger, "IoHandler::Run exit.");
}

void IoHandler::Handle(EventMessage* message){
  //TRACE(logger, "IoHandler::Handle " << *message);

  switch (message->type_io_event) {
    case MessageType::kIoMessageEvent:
    HandleIoMessageEvent(message);
    break;
  case MessageType::kIoActiveCloseEvent:
    HandleIoActiveCloseEvent(message);
    break;
  default:
    WARN(logger, "IOHanlder unknown message:" << message->type_id);
    break;
  }
}

void IoHandler::HandleIoMessageEvent(EventMessage* message){
  TRACE(logger, "HandleIOMessageEvent");
  Connecting* con = (Connecting*)((void*)(message->descriptor_id));
  if (!con) {
    WARN(logger, "IoHandler::HandleIoMessageEventt descriptor is not Connecting");
    HandleIoMessageFailed(message);
    return;
  }

  if (0 != con->EncodeMsg(message)){
    HandleIoMessageFailed(message);
    return;
  }
  //������Ϣencode֮�󼴿��ͷţ�˫����Ϣ���ڷ��ػ��߳�ʱ��ʱ���ͷ�
  //server�𸴵���Ϣ��Ҫ�������ͷŵ�
  if (message->direction == EventMessage::kOneway
    || message->direction == EventMessage::kOutgoingResponse) {
    MessageFactory::Destroy(message);
  }

  con->OnWrite();
}

void IoHandler::HandleIoMessageFailed(EventMessage* message) {
  if (message->direction == EventMessage::kOutgoingRequest) {
    message->status = EventMessage::kInternalFailure;
    IoService::GetInstance().SendToServiceHandle(message);
  } else {
    MessageFactory::Destroy(message);
  }
}

void IoHandler::HandleIoActiveCloseEvent(EventMessage* message){
  TRACE(logger, "HandleIOMessageEvent");
  Connecting* con = (Connecting*)((void*)(message->descriptor_id));
  if (!con) {
    WARN(logger, "IoHandler::HandleIoMessageEventt descriptor is not Connecting");
    return;
  }

  MessageFactory::Destroy(message);
  con->OnActiveClose();
  con->Destroy();
}

}
