
#include <functional>
#include <sstream>
#include <iomanip>
#include <sys/prctl.h>
#include "matrix_collector.h"
#include "matrix_stat_map.h"
#include "matrix.h"

namespace bdf {

namespace monitor {

LOGGER_CLASS_IMPL(logger, MatrixCollector);
LOGGER_CLASS_IMPL_NAME(collector_logger, MatrixCollector, "bdf.monitor");
LOGGER_CLASS_IMPL_NAME(collector_logger_simple, MatrixCollector, "bdf.monitor.simple");

MatrixCollector::MatrixCollector(
  const std::string& monitor_file, 
  uint32_t bucket_count, 
  uint32_t queue_size)
  : bucket_count_(bucket_count)
  , queue_size_(queue_size)
  , thread_(nullptr)
  , running_(false) 
  , monitor_file_name_pre_(monitor_file)
  , stat_map_(new MatrixStatMap(time(nullptr))) {

  queue_.resize(bucket_count_);
  for (auto& queue : queue_) {
    queue = new QueueType();
    //queue->resize(queue_size);
  }
  locks_.resize(queue_.size());
  for (auto& lock : locks_){
    lock = new SpinLock();
  }

  monitor_file_name_ = AppendData(monitor_file_name_pre_);
}

MatrixCollector::~MatrixCollector() {
  Stop();

  for (auto& queue : queue_) {
    delete queue;
  }
  queue_.clear();
  for (auto& lock : locks_) {
    delete lock;
  }
  locks_.clear();
}

int MatrixCollector::Start() {
  if (thread_) {
    return 0;
  }

  running_ = true;
  thread_ = new std::thread(std::bind(&MatrixCollector::Run, this));
  return 0;
}

int MatrixCollector::Stop() {
  if (!thread_) {
    return 0;
  }

  running_ = false;
  thread_->join();
  delete thread_;
  thread_ = nullptr;
  return 0;
}

std::string MatrixCollector::AppendData(std::string monitor_file_pre){
  time_t now = time(nullptr);
  struct tm *tm_now;
  tm_now = localtime(&now);
  tm_now->tm_year += 1900;
  tm_now->tm_mon++;
  std::stringstream str_log;
  str_log << monitor_file_pre << '.'<< tm_now->tm_year << '_' << std::setfill('0')
    << std::setw(2) << tm_now->tm_mon << '_'
    << std::setw(2) << tm_now->tm_mday;
  return str_log.str();
}

bool MatrixCollector::GetFileName(std::string& new_name){
  new_name = AppendData(monitor_file_name_pre_);
  if (new_name != monitor_file_name_){
    return true;
  }
  return false;
}

void MatrixCollector::Run() {
  prctl(PR_SET_NAME, "matrix");

  INFO(logger, "MatrixCollector Running");
  FILE* fp = fopen(monitor_file_name_.c_str(), "a");
  if (fp == nullptr){
    ERROR(logger, "open monitor file failed. file:" << monitor_file_name_);
    return;
  }

  MatrixStatMapPtr stat_map(new MatrixStatMap(time(nullptr)));
  while (running_) {
    std::string new_name;
    if (GetFileName(new_name)){
      fclose(fp);
      monitor_file_name_ = new_name;
      FILE* fp = fopen(monitor_file_name_.c_str(), "a");
      if (fp == nullptr) {
        ERROR(logger, "open monitor file failed. file:" << monitor_file_name_);
        return;
      }
    }
    if (time(nullptr) - stat_map->GetStartTime() > 30) {
      stat_map->Freeze();
      INFO(collector_logger, "MatrixCollector:\n" << *stat_map);
      const std::string& dump = stat_map->ToStringSimple();
      fwrite(dump.c_str(), dump.size(), 1, fp);
      fflush(fp);

      std::atomic_store(&stat_map_, stat_map);
      stat_map = MatrixStatMapPtr(new MatrixStatMap(time(nullptr)));
      stat_map->Inherit(*stat_map_);
    }
    ProcessQueueList(stat_map);
    
    sleep(1);
  }
  fclose(fp);
  INFO(logger, "MatrixCollector Exit");
}

void MatrixCollector::ProcessQueueList(MatrixStatMapPtr stat_map){
  for (uint32_t i = 0; i != bucket_count_; ++i) {
    QueueType* queue = queue_[i];
    if (queue->size() > queue_size_) {
      WARN(logger, "queue size is bigger than:" << queue_size_
        << ",size is:" << queue->size());
    }
    ProcessQueue(queue, stat_map, i);
  }
}

void MatrixCollector::ProcessQueue(QueueType* queue, MatrixStatMapPtr stat_map, uint32_t& idx) {
  while (!queue->empty()) {
    locks_[idx]->Lock();
    const MatrixItem* item = queue->front();
    queue->pop();
    locks_[idx]->UnLock();
    switch (item->operation) {
      case Matrix::kSet : 
        stat_map->Set(item->name, item->val, item->persistent);
        break;
      case Matrix::kAdd : 
        stat_map->Add(item->name, item->val, item->persistent); 
        break;
      case Matrix::kSub : 
        stat_map->Sub(item->name, item->val, item->persistent); 
        break;
      case Matrix::kReset :
        stat_map->Reset(item->name); 
        break;
      case Matrix::kTimeDistribute : {
        if (item->result.empty()) {
          stat_map->TimeDistrubute(item->name, item->val, item->persistent);
        } else {
          stat_map->TimeDistrubute(item->name, item->result, item->val, item->persistent);
        }
        break;
      }
      default: 
        WARN(logger, "MatrixCollector::ProcessEvent unknown operation:" << *item); 
        break;
    }
    delete item;
  }
}

}

}
