#pragma once

namespace bdf {

class CoroutineActor;

struct CoroContext {
  virtual ~CoroContext() {}
  // ���һ��ָ�룬���ͻ���ʱ��Ҫ��
  CoroutineActor* actor = nullptr;
};

}
