//
// Created by 86183 on 2023/1/5.
//

#ifndef DISORDERHANDLINGSYSTEM_BUFFER_SIZE_MANAGER_H
#define DISORDERHANDLINGSYSTEM_BUFFER_SIZE_MANAGER_H

#include "Operator/MSMJ/common/define.h"
#include "statistics_manager.h"
#include "Operator/MSMJ/profiler/tuple_productivity_profiler.h"
#include "Utils/ConfigMap.hpp"

typedef std::shared_ptr<class Stream> StreamPtr;
typedef std::shared_ptr<class KSlack> KSlackPtr;
typedef std::shared_ptr<class BufferSizeManager> BufferSizeManagerPtr;
typedef std::shared_ptr<class StatisticsManager> StatisticsManagerPtr;
typedef std::shared_ptr<class TupleProductivityProfiler> TupleProductivityProfilerPtr;
typedef std::shared_ptr<class Synchronizer> SynchronizerPtr;

using namespace OoOJoin;

class BufferSizeManager {
public:

    explicit BufferSizeManager(StatisticsManagerPtr statistics_manager, TupleProductivityProfilerPtr profiler);

    ~BufferSizeManager() = default;

    //自适应K值算法
    auto k_search(uint64_t stream_id) -> uint64_t;

private:

    //论文中的函数γ(L,T)
    auto y(uint64_t K) -> double;

    INTELLI::ConfigMapPtr opConfig;

    //互斥锁
    std::mutex latch_;

    phmap::parallel_flat_hash_map<int, Stream *> stream_map_{};

    //数据统计器
    StatisticsManagerPtr statistics_manager_;

    //元组生产力
    TupleProductivityProfilerPtr productivity_profiler_;

};


#endif //DISORDERHANDLINGSYSTEM_BUFFER_SIZE_MANAGER_H
