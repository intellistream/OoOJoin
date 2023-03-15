//
// Created by 86183 on 2023/1/4.
//

#ifndef DISORDERHANDLINGSYSTEM_K_SLACK_H
#define DISORDERHANDLINGSYSTEM_K_SLACK_H


#include <cstddef>
#include <queue>
#include <set>
#include "Operator/MSMJ/common/define.h"
#include "Operator/MSMJ/synchronizer/synchronizer.h"
#include "Operator/MSMJ/manager/statistics_manager.h"
#include "Operator/MSMJ/manager/buffer_size_manager.h"

namespace MSMJ {

    class KSlack {
    public:

        explicit KSlack(int streamId, BufferSizeManager *buffer_size_manager, StatisticsManager *statistics_manager,
                        Synchronizer *synchronizer);

        ~KSlack();

        auto disorder_handling(const TrackTuplePtr& tuple) -> void;

        auto get_id() -> int;

        auto setConfig(INTELLI::ConfigMapPtr config) -> void;

        //同步器
        Synchronizer *synchronizer_;
    private:

        INTELLI::ConfigMapPtr cfg = nullptr;


        //缓冲区大小,相当于论文的K值,注：缓冲区大小并不是指集合的大小，而是以时间为单位来度量的
        size_t buffer_size_{0};

        //当前时间(相当于论文的T值)
        int current_time_{};

        //流id
        int stream_id_{};

        //缓冲区(用小根堆)
        std::priority_queue<TrackTuplePtr> buffer_;

        //缓冲区管理器
        BufferSizeManager *buffer_size_manager_;

        //数据统计管理器
        StatisticsManager *statistics_manager_;

    };
}

#endif //DISORDERHANDLINGSYSTEM_K_SLACK_H
