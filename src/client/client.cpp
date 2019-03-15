#include "client/client.h"
#include "service/io_service.h"

namespace bdf{

Client::Client(
  const std::string& name,
  const std::string& address,
  uint32_t timeout_ms,
  uint32_t heartbeat_ms): 
  name_(name), 
  address_(address),
  timeout_ms_(timeout_ms), 
  heartbeat_ms_(heartbeat_ms), 
  connect_(NULL){
}

Client::~Client(){
  Stop();
}

int Client::Start() {
  ClientConnect* con = CreateClient(address_, timeout_ms_, heartbeat_ms_);
  if (!con) {
    ERROR(logger, "Client::Start CreateClient fail"<< ", address:" << address_);
    return -1;
  }

  if (0 != con->TryConnect()) {
    ERROR(logger, "Client::Start fail"<< ", address:" << address_);
    delete con;
    return -2;
  }

  connect_ = con;
  return 0;
}

ClientConnect* Client::CreateClient(
  const std::string& address, uint32_t timeout_ms, uint32_t heartbeat_ms){
  return nullptr;
}

int Client::Stop() {
  if (!connect_) {
    return 0;
  }

  int ret = connect_->Stop();
  connect_ = NULL;
  return ret;
}

int64_t Client::GetSequenceId(){
  static std::atomic<int64_t> sequence_id_(0);

  int64_t ret = sequence_id_++;
  if (ret < 0){
    sequence_id_.store(0);
  }
  return ret;
}

bool Client::Send(EventMessage* message) {
  if (GetClientStatus() != kWorking) {
    //DEBUG(logger, "Client::Send Channel Broken" << *this);
    MessageFactory::Destroy(message);
    return false;
  }

  message->sequence_id = GetSequenceId();
  message->descriptor_id = (int64_t)((void*)connect_);
  message->direction = MessageBase::kOneway;
  IoService::GetInstance().SendToIoHandle(message);
  return true;
}

EventMessage* Client::DoSendRecieve(EventMessage* message, uint32_t timeout_ms) {
  if (GetClientStatus() != kWorking) {
    //DEBUG(logger, "Client::Send Channel Broken" << *this);
    MessageFactory::Destroy(message);
    return NULL;
  }

  message->sequence_id = GetSequenceId();
  message->descriptor_id = (int64_t)((void*)connect_);
  message->direction = MessageBase::kOutgoingRequest;
  IoService::GetInstance().SendToIoHandle(message);
  //TODO...
  return nullptr;
}

}
