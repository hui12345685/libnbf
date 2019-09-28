
#pragma once

#include <thread>
#include <memory>
#include <atomic>
//#include <queue>
//#include <mutex>
#include "../common/lockfree_ringbuffer.h"
#include "../common/logger.h"
#include "../common/thread_id.h"
#include "matrix_item_map.h"

namespace bdf {

namespace monitor {

class MatrixStatMap;

class MatrixCollector {
 public:
  //typedef std::queue<const MatrixItem*> QueueType;
  typedef MPMCLockfreeRingbuffer<const MatrixItem*> QueueType;
  typedef std::shared_ptr<MatrixStatMap> MatrixStatMapPtr;

  MatrixCollector(
    const std::string& monitor_file, 
    uint32_t bucket_count, 
    uint32_t queue_size);
  ~MatrixCollector();

  int Start();
  int Stop();

  inline int Send(const MatrixItem* item) {
    uint64_t idx = 0;
    QueueType* queue = GetQueue(idx);
    //locks_[idx]->lock();
    if (!queue->push(item)){
      return -1;
    }
    //locks_[idx]->unlock();
    return 0;
  }

  inline MatrixStatMapPtr GetMatrixStatMap() {
    return stat_map_;
  }

 private:
  LOGGER_CLASS_DECL(logger);
  LOGGER_CLASS_DECL(collector_logger);
  LOGGER_CLASS_DECL(collector_logger_simple);

  inline QueueType* GetQueue(uint64_t& idx) {
    //�����߳�id��ѡ�����̱߳Ƚ��ٵ�ʱ�����׳�ͻ���������׼��е�ĳ��bucket
    //idx = ThreadId::Get() & (bucket_count_ - 1);
    //idx = rand() % bucket_count_;
    //��������rand()���̲߳����Ƚ�����cpu,��Ϊ˳�����,����cpu���������
    static std::atomic<uint64_t> seq_id(0);
    idx = (++seq_id)% bucket_count_;
    return queue_.at(idx);
  }

  void Run();
  void ProcessQueueList(MatrixStatMapPtr stat_map);
  void ProcessQueue(QueueType* queue, MatrixStatMapPtr stat_map, uint32_t& idx);

  std::string AppendData(std::string monitor_file_pre);
  bool GetFileName(std::string& new_name);

  std::vector<QueueType*> queue_;
  //�������ҵ�ʱ��һ����Ҫ��SpinLock,����cpu���ķǳ���,���ܼ����½�
  //�û������Ӷ��Ͱ(32)������º���������Ч�����,������΢��һ�㣬�������þ�������������
  //std::vector<std::mutex*>locks_;
  uint32_t bucket_count_;
  uint32_t queue_size_;
  std::thread* thread_;
  bool running_;
  std::string monitor_file_name_pre_;
  std::string monitor_file_name_;
  MatrixStatMapPtr stat_map_;
};

}
}
