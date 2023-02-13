//
// Created by 86183 on 2023/1/6.
//

#ifndef DISORDERHANDLINGSYSTEM_STATISTICS_MANAGER_H
#define DISORDERHANDLINGSYSTEM_STATISTICS_MANAGER_H


#include <vector>
#include <unordered_map>
#include <mutex>
#include <parallel-hashmap/parallel_hashmap/phmap.h>
#include "Operator/MSMJ/profiler/tuple_productivity_profiler.h"
#include "Common/Tuples.h"
#include "Utils/ConfigMap.hpp"

typedef std::shared_ptr<class Stream> StreamPtr;
typedef std::shared_ptr<class KSlack> KSlackPtr;
typedef std::shared_ptr<class BufferSizeManager> BufferSizeManagerPtr;
typedef std::shared_ptr<class StatisticsManager> StatisticsManagerPtr;
typedef std::shared_ptr<class TupleProductivityProfiler> TupleProductivityProfilerPtr;
typedef std::shared_ptr<class Synchronizer> SynchronizerPtr;


class StatisticsManager {
public:

    explicit StatisticsManager(TupleProductivityProfilerPtr profiler,
                               phmap::parallel_flat_hash_map<uint64_t, Stream *> stream_map);

    ~StatisticsManager() = default;

    //获得元组最大delay
    auto get_maxD(uint64_t stream_id) -> uint64_t;

    //离散随机变量Dik的概率分布函数fDiK， Dik表示连接算子在k设置下接受相应流中一个元组的粗粒度延迟
    auto fDk(uint64_t d, uint64_t stream_id, uint64_t K) -> double;

    //|wi^l|的估计
    auto wil(uint64_t l, uint64_t stream_id, uint64_t K) -> uint64_t;

    auto add_record(uint64_t stream_id, OoOJoin::TrackTuple tuple) -> void;

    auto add_record(uint64_t stream_id, uint64_t T, uint64_t K) -> void;

    auto setConfig(INTELLI::ConfigMapPtr opConfig) -> void;

private:
    auto inline get_D(uint64_t delay) -> uint64_t {
        uint64_t g = opConfig->getU64("g");
        return delay % g == 0 ? delay / g : delay / g + 1;
    }

    //获得Ksync
    auto get_ksync(uint64_t stream_id) -> uint64_t;

    //获取avg(ksync)
    auto get_avg_ksync(uint64_t stream_id) -> uint64_t;

    //估计未来的ksync
    auto get_future_ksync(uint64_t stream_id) -> uint64_t;

    //参考文献[25]的自适应窗口方法, 传入的主要参数待定，需阅读文献[25]
    auto get_R_stat(uint64_t stream_id) -> uint64_t;

    //离散随机变量Di的概率分布函数fDi
    auto fD(uint64_t d, uint64_t stream_id) -> double;

    INTELLI::ConfigMapPtr opConfig;

    //互斥锁
    std::mutex latch_;

    //Rstat窗口大小
    phmap::parallel_flat_hash_map<uint64_t, uint64_t> R_stat_map_{};

    phmap::parallel_flat_hash_map<uint64_t, Stream *> stream_map_{};

    //历史流Si输入记录的映射
    phmap::parallel_flat_hash_map<uint64_t, std::vector<OoOJoin::TrackTuple>> record_map_{};

    //历史流Si的T记录
    phmap::parallel_flat_hash_map<uint64_t, uint64_t> T_map_{};

    //历史流的K记录
    phmap::parallel_flat_hash_map<uint64_t, uint64_t> K_map_{};

    //保存所有的K_sync，方便抽取样本预测未来的ksync
    phmap::parallel_flat_hash_map<uint64_t, std::vector<uint64_t>> ksync_map_{};

    //直方图映射
    phmap::parallel_flat_hash_map<uint64_t, std::vector<double>> histogram_map_{};

    //元组生产力
    TupleProductivityProfilerPtr productivity_profiler_;
};


#endif //DISORDERHANDLINGSYSTEM_STATISTICS_MANAGER_H
