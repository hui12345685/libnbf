#pragma once

#include <string>
#include <atomic>
#include <vector>
#include "common/logger.h"
#include "context.h"

namespace bdf {

class EventMessage;
class ClientRouter;
class ClientConfig;

class ClientRouters {
public:
  ClientRouters(
    const std::string& name, 
    const std::string& mapping,
    const bool& sigle_send_sigle_recv);
  ~ClientRouters();

  int Start(const std::vector<ClientConfig>& clients);
  int Stop();

  bool Send(EventMessage * message);
  bool SendHash(EventMessage* message, uint32_t hash);

  bool Invoke(EventMessage * message, const InvokerCallback& cb);
  bool Invoke(EventMessage* message, const InvokerCallback& cb,uint32_t hash);

  EventMessage* SendRecieve(EventMessage* message, uint32_t timeout_ms = 0);
  EventMessage* SendRecieveHash(
    EventMessage* message, uint32_t hash, uint32_t timeout_ms = 0);

private:
  LOGGER_CLASS_DECL(logger_);

  std::string name_;
  //���Ը��ݷ��͵�msg��ĳ��ֵhash��ĳ����������ip�ϣ���δʵ��
  std::string mapping_;
  std::vector<ClientRouter*> client_routers_;
  std::atomic<uint32_t> current_;
  //ֻ��һЩͬ���Ŀͻ��˺�Э���֧��
  bool sigle_send_sigle_recv_;
};

}

