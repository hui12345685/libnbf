#pragma once

#include <map>
#include <string>
#include "common/logger.h"
#include "common/common.h"
#include "context.h"

namespace bdf{

struct ClientRoutersConfig;
class ClientRouters;
class EventMessage;

class ClientMgr{

public:

  ClientMgr();
  ~ClientMgr();

  int Start(const ClientRoutersConfig& routers_config);
  int Stop();

  bool Send(const std::string& router,EventMessage* message);
  bool SendHash(const std::string& router, EventMessage* message, uint32_t hash);

  bool Invoke(
    const std::string& router, 
    EventMessage* message,
    const InvokerCallback& cb);

  bool Invoke(
    const std::string& router, 
    EventMessage* message, 
    const InvokerCallback& cb, 
    uint32_t hash);

  //����Э���Ƿŵ�service handle�е�
  //���Ե��øú����ĵط�Ҳ��Ҫ����service handle�߳��У������ǲ�ͬ��Э�̶���
  EventMessage* SendRecieve(
    const std::string& router,
    EventMessage* message,
    uint32_t timeout_ms = 0);
  EventMessage* SendRecieveHash(
    const std::string& router,
    EventMessage* message,
    uint32_t hash,
    uint32_t timeout_ms = 0);

private:
  LOGGER_CLASS_DECL(logger_);
  std::map<std::string, ClientRouters* > router_maps_;

  ClientRouters* GetClientRouters(const std::string& router);

  DISALLOW_COPY_AND_ASSIGN(ClientMgr)

};

class ForTest{
public:
  static ForTest& Inst(){
    static ForTest for_test;
    return for_test;
  }
  bool GetForTest() { return is_test_; }
  void SetForTest(bool is_test) { is_test_ = is_test; }
private:
  ForTest():
    is_test_(false){
  }
  bool is_test_;
};

}

