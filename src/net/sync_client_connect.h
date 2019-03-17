#pragma once

#include "net/client_connect.h"
#include "net/sync_sequence.h"
#include "common/logger.h"

namespace bdf{

class SyncClientConnect:public ClientConnect{

public:
  SyncClientConnect(uint32_t timeout_ms, uint32_t heartbeat_ms);
  ~SyncClientConnect();

  virtual void OnDecodeMessage(EventMessage* message);

private:
  SyncSequence sync_sequence_;

  LOGGER_CLASS_DECL(logger);
};

}