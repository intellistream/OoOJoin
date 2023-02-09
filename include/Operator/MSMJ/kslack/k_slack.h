//
// Created by 86183 on 2023/1/4.
//

#ifndef DISORDERHANDLINGSYSTEM_K_SLACK_H
#define DISORDERHANDLINGSYSTEM_K_SLACK_H


#include <cstddef>
#include <queue>
#include <set>
#include "Operator/MSMJ/common/define.h"
#include "Operator/MSMJ/manager/buffer_size_manager.h"
#include "Operator/MSMJ/synchronizer/synchronizer.h"
#include "Operator/MSMJ/manager/statistics_manager.h"
#include "Common/Tuples.h"

typedef std::shared_ptr<class Stream> StreamPtr;
typedef std::shared_ptr<class KSlack> KSlackPtr;
typedef std::shared_ptr<class BufferSizeManager> BufferSizeManagerPtr;
typedef std::shared_ptr<class StatisticsManager> StatisticsManagerPtr;
typedef std::shared_ptr<class TupleProductivityProfiler> TupleProductivityProfilerPtr;
typedef std::shared_ptr<class Synchronizer> SynchronizerPtr;

class KSlack {
public:

    explicit KSlack(StreamPtr stream, BufferSizeManagerPtr buffer_size_manager, StatisticsManagerPtr statistics_manager,
                    SynchronizerPtr synchronizer);

    ~KSlack() = default;

    auto disorder_handling() -> void;

    auto get_output() -> std::queue<TrackTuple>;

    auto get_id() -> int;

private:

    INTELLI::ConfigMapPtr opConfig;

    //输出区
    std::queue<OoOJoin::TrackTuple> output_;

    //观察区(用于最后的结果观察)
    std::queue<OoOJoin::TrackTuple> watch_output_;

    //缓冲区大小,相当于论文的K值,注：缓冲区大小并不是指集合的大小，而是以时间为单位来度量的
    size_t buffer_size_{1};

    //当前时间(相当于论文的T值)
    uint64_t current_time_;

    //传输过来的流
    StreamPtr stream_;

    //缓冲区(用随时保持有序的红黑树)
    std::set<OoOJoin::TrackTuple, TupleComparator> buffer_;

    //缓冲区管理器
    BufferSizeManagerPtr buffer_size_manager_;

    //数据统计管理器
    StatisticsManagerPtr statistics_manager_;

    //同步器
    SynchronizerPtr synchronizer_;
};


#endif //DISORDERHANDLINGSYSTEM_K_SLACK_H
