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
#include "Utils/ConfigMap.hpp"

namespace MSMJ {
    class Synchronizer {
    public:
        explicit Synchronizer(int stream_count, StreamOperator *stream_operator, INTELLI::ConfigMapPtr config);

        ~Synchronizer() = default;

        //同步过程
        auto synchronize_stream(const TrackTuplePtr &tuple) -> void;

        auto setConfig(INTELLI::ConfigMapPtr config) -> void;

        //连接器
        StreamOperator *stream_operator_;
    private:

        std::shared_mutex mu;

        INTELLI::ConfigMapPtr cfg = nullptr;

        //SyncBuf缓冲区映射
        std::vector<std::priority_queue<TrackTuplePtr>> sync_buffer_map_{};

        //Tsync
        int T_sync_{};

        //stream的数量
        int stream_count_{};

        //当前缓冲区拥有tuple的流的数量
        int own_stream_{};

    };

}
#endif //DISORDERHANDLINGSYSTEM_SYNCHRONIZER_H
