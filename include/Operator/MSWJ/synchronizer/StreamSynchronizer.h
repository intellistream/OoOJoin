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
#include "Operator/MSWJ/operator/StreamOperator.h"
#include "Operator/MSWJ/common/MSWJDefine.h"
#include "Utils/ConfigMap.hpp"

namespace MSWJ {
    class Synchronizer {
    public:
        explicit Synchronizer(int streamCount, StreamOperator *streamOperator, INTELLI::ConfigMapPtr config);

        ~Synchronizer() = default;

        Synchronizer(const Synchronizer &) = delete;

        Synchronizer &operator=(const Synchronizer &) = delete;

        Synchronizer(Synchronizer &&) = delete;

        Synchronizer &operator=(Synchronizer &&) = delete;

        // Synchronize process
        auto synchronizeStream(const TrackTuplePtr &tuple) -> void;

        auto setConfig(INTELLI::ConfigMapPtr config) -> void;

    private:
        // Connector
        StreamOperator *streamOperator;

        //write and read lock
        mutable std::shared_mutex mu;

        INTELLI::ConfigMapPtr cfg{};

        // SyncBuf buffer mapping
        std::vector<std::priority_queue<TrackTuplePtr>> synBufferMap{};

        // Tsync
        int tSync{};

        // The number of streams that have tuples in the current buffer
        int ownStream{};
    };
}

#endif //DISORDERHANDLINGSYSTEM_SYNCHRONIZER_H
