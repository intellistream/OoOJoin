//
// Created by 86183 on 2023/1/4.
//

#ifndef DISORDERHANDLINGSYSTEM_K_SLACK_H
#define DISORDERHANDLINGSYSTEM_K_SLACK_H

#include <cstddef>
#include <queue>
#include <set>
#include <Operator/MSWJ/Common/MSWJDefine.h>
#include <Operator/MSWJ/Synchronizer/StreamSynchronizer.h>
#include <Operator/MSWJ/Manager/StatisticsManager.h>
#include <Operator/MSWJ/Manager/BufferSizeManager.h>

/**
 * @class KSlack
 * @ingroup ADB_OPERATORS
 * @class MSWJOperator Operator/KSlack.h
 * @brief Implementation of the K-Slack operator for handling disordering of incoming tuples
 * @note require configurations:
 * - "bufferSize" U64: The buffer size, equivalent to the K value in the paper. Note that buffer size is measured in time units, not in number of tuples.
 * @note operator tag = "KSLACK"
 */
namespace MSWJ {

    class KSlack {
    public:
        /**
         * @brief Constructor with required parameters
         *
         * @param streamId The stream ID of the KSlack instance
         * @param bufferSizeManager Pointer to the BufferSizeManager object
         * @param statisticsManager Pointer to the StatisticsManager object
         * @param synchronizer Pointer to the Synchronizer object
         */
        explicit KSlack(int streamId, BufferSizeManager* bufferSizeManager, StatisticsManager* statisticsManager,
                        Synchronizer* synchronizer);

        /**
         * @brief Destructor
         */
        ~KSlack() = default;

        // Disable copy constructor and copy assignment operator
        KSlack(const KSlack&) = delete;
        KSlack& operator=(const KSlack&) = delete;

        /**
         * @brief Handles the disordering of incoming tuples
         *
         * This function implements the K-Slack algorithm described in the paper "Quality-Driven Disorder Handling for M-way Sliding Window Stream Joins".
         * It maintains a priority queue of tuples in the buffer and only outputs tuples that are within the window of the current time T.
         * T is updated every time a new tuple is processed.
         *
         * @param tuple Pointer to the incoming tuple
         */
        auto disorderHandling(const TrackTuplePtr& tuple) -> void;

        /**
         * @brief Sets the configuration for the KSlack instance
         *
         * @param config Pointer to the ConfigMap object
         */
        auto setConfig(const INTELLI::ConfigMapPtr& config) -> void;

        // Configuration object
        INTELLI::ConfigMapPtr cfg{};

        // Buffer size, equivalent to the K value in the paper. Note that buffer size is measured in time units, not in number of tuples.
        size_t bufferSize{0};

        // Current time, equivalent to the T value in the paper.
        int currentTime{};

        // Stream ID of the KSlack instance
        int streamId{};

        // Priority queue used as the buffer (min heap)
        std::priority_queue<TrackTuplePtr, std::deque<TrackTuplePtr>, TrackTuplePtrComparator> buffer{};

    private:
        // Buffer size manager
        BufferSizeManager* bufferSizeManager{};

        // Statistics manager
        StatisticsManager* statisticsManager{};

        // Synchronizer object
        Synchronizer* synchronizer{};
    };
}

#endif //DISORDERHANDLINGSYSTEM_K_SLACK_H
