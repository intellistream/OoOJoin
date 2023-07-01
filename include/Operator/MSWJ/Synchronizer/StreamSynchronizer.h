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
#include <Operator/MSWJ/Operator/StreamOperator.h>
#include <Operator/MSWJ/Common/MSWJDefine.h>
#include <Utils/ConfigMap.hpp>
#include <shared_mutex>

namespace MSWJ {
/**
 * @class Synchronizer
 * @brief A class used to synchronize multiple streams of tuples.
 */
    class Synchronizer {
    public:
        /**
         * @brief Constructor that takes the number of streams, a pointer to a StreamOperator, and a ConfigMapPtr argument.
         * @param streamCount int type, the number of streams.
         * @param streamOperator StreamOperator pointer type, a pointer to a StreamOperator object.
         * @param config ConfigMapPtr type, a configuration parameter.
         * @note This constructor initializes the synchronization buffer map and sets the value of the "cfg" member variable to the input configuration parameter.
         */
        explicit Synchronizer(int streamCount, StreamOperator *streamOperator, INTELLI::ConfigMapPtr config);

        /**
         * @brief Default destructor.
         */
        ~Synchronizer() = default;

        /**
         * @brief Copy constructor is deleted.
         */
        Synchronizer(const Synchronizer &) = delete;

        /**
         * @brief Copy assignment operator is deleted.
         */
        Synchronizer &operator=(const Synchronizer &) = delete;

        /**
         * @brief Move constructor is deleted.
         */
        Synchronizer(Synchronizer &&) = delete;

        /**
         * @brief Move assignment operator is deleted.
         */
        Synchronizer &operator=(Synchronizer &&) = delete;

        /**
         * @brief Synchronizes a stream with the synchronization buffer.
         * @param tuple TrackTuplePtr type, a pointer to the tuple to be synchronized.
         * @return void.
         * @note This function adds the tuple to the corresponding synchronization buffer.
         */
        auto synchronizeStream(const TrackTuplePtr &tuple) -> void;

        /**
         * @brief Sets the configuration based on the given ConfigMapPtr argument.
         * @param config ConfigMapPtr type, configuration parameters.
         * @return void.
         * @note This function updates the value of the "cfg" member variable to the input configuration parameter.
         */
        auto setConfig(INTELLI::ConfigMapPtr config) -> void;

        /**
         * @brief A ConfigMapPtr type member variable for storing configuration parameters.
         */
        INTELLI::ConfigMapPtr cfg{};

        /**
         * @brief An int type member variable for storing the Tsync value.
         */
        int tSync{};

        /**
         * @brief An int type member variable for storing the number of streams that have tuples in the current buffer.
         */
        int ownStream{};

        /**
         * @brief A vector of priority queues of TrackTuplePtr type for storing the synchronization buffer map.
         */
        std::vector<std::priority_queue<TrackTuplePtr, std::deque<TrackTuplePtr>, TrackTuplePtrComparator>> synBufferMap{};

    private:
        /**
         * @brief A pointer to a StreamOperator object.
         */
        StreamOperator *streamOperator;

        /**
         * @brief A mutable shared mutex used to lock threads for synchronization.
         */
        mutable std::shared_mutex mu;
    };
}

#endif //DISORDERHANDLINGSYSTEM_SYNCHRONIZER_H
