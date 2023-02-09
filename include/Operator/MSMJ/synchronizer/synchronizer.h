//
// Created by 86183 on 2023/1/7.
//

#ifndef DISORDERHANDLINGSYSTEM_SYNCHRONIZER_H
#define DISORDERHANDLINGSYSTEM_SYNCHRONIZER_H


#include <vector>
#include <queue>
#include <set>
#include <list>
#include <mutex>
#include "Operator/MSMJ/operator/stream_operator.h"
#include "Operator/MSMJ/common/define.h"
#include "Common/Tuples.h"

typedef std::shared_ptr<class Stream> StreamPtr;
typedef std::shared_ptr<class KSlack> KSlackPtr;
typedef std::shared_ptr<class BufferSizeManager> BufferSizeManagerPtr;
typedef std::shared_ptr<class StatisticsManager> StatisticsManagerPtr;
typedef std::shared_ptr<class TupleProductivityProfiler> TupleProductivityProfilerPtr;
typedef std::shared_ptr<class Synchronizer> SynchronizerPtr;
typedef std::shared_ptr<class StreamOperator> StreamOperatorPtr;

using namespace OoOJoin;

class Synchronizer {
public:
    explicit Synchronizer(int stream_count, StreamOperatorPtr stream_operator);

    ~Synchronizer() = default;

    //同步过程
    auto synchronize_stream(std::queue<TrackTuple> &input) -> void;

    auto get_output() -> std::queue<TrackTuple>;

private:

    INTELLI::ConfigMapPtr opConfig;

    //互斥锁
    std::mutex latch_;

    //SyncBuf缓冲区映射
    phmap::parallel_flat_hash_map<int, phmap::btree_set<TrackTuple, TupleComparator>> sync_buffer_map_{};

    //同步输出区
    std::queue<TrackTuple> output_{};

    //观察区
    std::queue<TrackTuple> watch_output_{};

    //Tsync
    int T_sync_{};

    //stream的数量
    int stream_count_{};

    //当前缓冲区拥有tuple的流的数量
    int own_stream_{};

    //连接器
    StreamOperatorPtr stream_operator_;
};


#endif //DISORDERHANDLINGSYSTEM_SYNCHRONIZER_H
