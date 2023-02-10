//
// Created by 86183 on 2023/1/9.
//

#ifndef DISORDERHANDLINGSYSTEM_TUPLE_PRODUCTIVITY_PROFILER_H
#define DISORDERHANDLINGSYSTEM_TUPLE_PRODUCTIVITY_PROFILER_H


#include <unordered_map>
#include <map>
#include <mutex>
#include "parallel-hashmap/parallel_hashmap/phmap.h"
#include "Utils/ConfigMap.hpp"
#include <parallel-hashmap/parallel_hashmap/btree.h>


typedef std::shared_ptr<class Stream> StreamPtr;
typedef std::shared_ptr<class KSlack> KSlackPtr;
typedef std::shared_ptr<class BufferSizeManager> BufferSizeManagerPtr;
typedef std::shared_ptr<class StatisticsManager> StatisticsManagerPtr;
typedef std::shared_ptr<class TupleProductivityProfiler> TupleProductivityProfilerPtr;
typedef std::shared_ptr<class Synchronizer> SynchronizerPtr;


class TupleProductivityProfiler {
public:

    TupleProductivityProfiler() = default;

    ~TupleProductivityProfiler() = default;

    auto get_join_record_map() -> phmap::parallel_flat_hash_map<uint64_t, uint64_t>;

    auto add_join_record(uint64_t stream_id, uint64_t count) -> void;

    auto update_cross_join(uint64_t Di, uint64_t res) -> void;

    auto update_join_res(uint64_t Di, uint64_t res) -> void;

    auto get_select_ratio(uint64_t K) -> double;

    auto get_requirement_recall() -> double;

    auto setConfig(INTELLI::ConfigMapPtr opConfig) -> void;

private:

    INTELLI::ConfigMapPtr opConfig;

    //互斥锁
    std::mutex latch_;

    //到达join operator的元组数量记录
    phmap::parallel_flat_hash_map<uint64_t, uint64_t> join_record_map_{};

    //the join operator records both the number of cross-join result size,
    phmap::btree_map<uint64_t, uint64_t> cross_join_map_{};

    //the number of join results, using map for sorting
    phmap::btree_map<uint64_t, uint64_t> join_result_map_{};

};


#endif //DISORDERHANDLINGSYSTEM_TUPLE_PRODUCTIVITY_PROFILER_H
