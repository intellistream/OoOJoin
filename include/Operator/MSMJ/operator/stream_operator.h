//
// Created by 86183 on 2023/1/8.
//

#ifndef DISORDERHANDLINGSYSTEM_STREAM_OPERATOR_H
#define DISORDERHANDLINGSYSTEM_STREAM_OPERATOR_H


#include <queue>
#include <unordered_map>
#include <list>
#include <mutex>
#include "parallel-hashmap/parallel_hashmap/phmap.h"
#include "Operator/MSMJ/profiler/tuple_productivity_profiler.h"
#include "Operator/MSMJ/common/define.h"
#include "Utils/ConfigMap.hpp"

typedef std::shared_ptr<class Stream> StreamPtr;
typedef std::shared_ptr<class KSlack> KSlackPtr;
typedef std::shared_ptr<class BufferSizeManager> BufferSizeManagerPtr;
typedef std::shared_ptr<class StatisticsManager> StatisticsManagerPtr;
typedef std::shared_ptr<class TupleProductivityProfiler> TupleProductivityProfilerPtr;
typedef std::shared_ptr<class Synchronizer> SynchronizerPtr;

class StreamOperator {
public:

    explicit StreamOperator(TupleProductivityProfilerPtr profiler);

    ~StreamOperator() = default;

    auto mswj_execution(std::queue<OoOJoin::TrackTuple> &input) -> void;

    auto get_result() -> std::queue<std::vector<OoOJoin::TrackTuple>>;

    auto getJoinResultCount() -> int;

    auto setConfig(INTELLI::ConfigMapPtr opConfig) -> void;

private:

    auto can_join_(OoOJoin::TrackTuple t1, OoOJoin::TrackTuple t2) -> bool;

    INTELLI::ConfigMapPtr opConfig;

    //互斥锁
    std::mutex latch_;

    //连接时的T
    int T_op_{};

    //window map
    phmap::parallel_flat_hash_map<uint64_t, std::list<OoOJoin::TrackTuple>> window_map_{};

    //结果元组
    std::queue<std::vector<OoOJoin::TrackTuple>> result_{};

    //元组生产力监视器
    TupleProductivityProfilerPtr productivity_profiler_;

};


#endif //DISORDERHANDLINGSYSTEM_STREAM_OPERATOR_H
