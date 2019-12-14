
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

  int Send(const MatrixItem* item);

  inline MatrixStatMapPtr GetMatrixStatMap() {
    return stat_map_;
  }

 private:
  LOGGER_CLASS_DECL(logger);
  LOGGER_CLASS_DECL(collector_logger);
  LOGGER_CLASS_DECL(collector_logger_simple);

  inline QueueType* GetQueue(uint64_t& idx) {
    //根据线程id来选，当线程比较少的时候，容易冲突，而且容易集中到某个bucket
    //idx = ThreadId::Get() & (bucket_count_ - 1);
    //idx = rand() % bucket_count_;
    //经过测试rand()多线程并发比较消耗cpu,改为顺序分配,降低cpu，提高性能
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
  //竞争激烈的时候一定不要用SpinLock,否则cpu消耗非常大,性能急剧下降
  //用互斥锁加多个桶(32)的情况下和无锁队列效果差不多,或者略微差一点，这里最用就用了无所队列
  //std::vector<std::mutex*>locks_;
  uint32_t bucket_count_;
  uint32_t queue_size_;
  std::thread* thread_;
  bool running_;
  std::string monitor_file_name_pre_;
  std::string monitor_file_name_;
  MatrixStatMapPtr stat_map_;

  std::atomic<int> queue_push_failed_times;
  std::atomic<int64_t> pause_push_stop_times;
};

}
}
