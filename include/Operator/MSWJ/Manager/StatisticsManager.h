//
// Created by 86183 on 2023/1/6.
//

#ifndef DISORDERHANDLINGSYSTEM_STATISTICS_MANAGER_H
#define DISORDERHANDLINGSYSTEM_STATISTICS_MANAGER_H

#include <vector>
#include <unordered_map>
#include <mutex>
#include <Operator/MSWJ/Common/MSWJDefine.h>
#include <Operator/MSWJ/Profiler/TupleProductivityProfiler.h>
#include <Utils/ConfigMap.hpp>
#include <Common/Tuples.h>

/**
 * @namespace MSWJ
 * @brief Namespace for the MSWJ operator implementation
 */
using namespace OoOJoin;

namespace MSWJ {

    class StatisticsManager {
    public:
        /**
         * @brief Constructor with required parameters
         *
         * @param profiler Pointer to the TupleProductivityProfiler object
         * @param config Pointer to the ConfigMap object
         */
        explicit StatisticsManager(TupleProductivityProfiler* profiler, INTELLI::ConfigMapPtr config);

        /**
         * @brief Destructor
         */
        ~StatisticsManager() = default;

        /**
         * @brief Get the maximum delay of tuples in a stream
         *
         * @param streamId The stream ID
         * @return int The maximum delay in the stream
         */
        auto getMaxD(int streamId) -> int;

        /**
         * @brief Probability density function fDiK for discrete random variable Dik
         *
         * This function calculates the probability density function fDiK for a given delay d, stream ID, and buffer size K.
         * It is used in the paper to estimate the size of the sliding window for a stream.
         *
         * @param d The delay value
         * @param streamId The stream ID
         * @param K The buffer size
         * @return double The probability density function value
         */
        auto fDk(int d, int streamId, int K) -> double;

        /**
         * @brief Estimate the size of the sliding window for a stream
         *
         * This function estimates the size of the sliding window for a given stream ID, delay value, and buffer size K.
         * It is used in the paper to determine the buffer size for the K-Slack operator.
         *
         * @param l The delay value
         * @param streamId The stream ID
         * @param K The buffer size
         * @return int The estimated size of the sliding window
         */
        auto wil(int l, int streamId, int K) -> int;

        /**
         * @brief Add a tuple record to the corresponding stream's recordMap
         *
         * This function adds a tuple record to the corresponding stream's recordMap.
         * It is used to keep track of the input history of a stream.
         *
         * @param streamId The stream ID
         * @param tuple Pointer to the incoming tuple
         */
        auto addRecord(int streamId, const TrackTuplePtr& tuple) -> void;

        /**
         * @brief Add a T and K record to the corresponding stream's tMap and kMap
         *
         * This function adds a T and K record to the corresponding stream's tMap and kMap.
         * It is used to keep track of the T and K values of a stream for the adaptive buffer size algorithm.
         *
         * @param streamId The stream ID
         * @param T The tuple arrival rate
         * @param K The buffer size
         */
        auto addRecord(int streamId, int T, int K) -> void;

        /**
         * @brief Sets the configuration for the StatisticsManager
         *
         * @param config Pointer to the ConfigMap object
         */
        auto setConfig(INTELLI::ConfigMapPtr config) -> void;

        /**
         * @brief Get the value of discrete random variable Di
         *
         * This function calculates the value of discrete random variable Di for a given delay value.
         * It is used in the paper to calculate the histogram for a stream.
         *
         * @param delay The delay value
         * @return int The value of discrete random variable Di
         */
        auto inline getD(int delay) -> int {
            return delay % g == 0 ? delay / g : delay / g + 1;
        }

    private:
        /**
         * @brief Get the value of Ksync for a stream
         *
         * This function calculates the value of Ksync for a given stream ID.
         * It is used in the adaptive buffer size algorithm.
         *
         * @param streamId The stream ID
         * @return int The value of Ksync
         */
        auto getKsync(int streamId) -> int;

        /**
         * @brief Get the average value of Ksync for a stream
         *
         * This function calculates the average value of Ksync for a given stream ID.
         * It is used in the adaptive buffer size algorithm.
         *
         * @param streamId The stream ID
         * @return int The average value of Ksync
         */
        auto getAvgKsync(int streamId) -> int;

        /**
         * @brief Estimate the future value of Ksync for a stream
         *
         * This function estimates the future value of Ksync for a given stream ID.
         * It is usedin the adaptive buffer size algorithm to predict the future buffer size.
         *
         * @param stream_id The stream ID
         * @return int The estimated future value of Ksync
         */
        auto getFutureKsync(int stream_id) -> int;

        /**
         * @brief Apply the adaptive window method described in reference [25], with the main parameters yet to be determined
         *
         * This function applies the adaptive window method described in reference [25] to estimate the size of the sliding window for a stream.
         * The main parameters of the method are yet to be determined in the paper.
         *
         * @param streamId The stream ID
         * @return int The estimated size of the sliding window
         */
        auto getRStat(int streamId) -> int;

        /**
         * @brief Probability density function fDi for discrete random variable Di
         *
         * This function calculates the probability density function fDi for a given delay value and stream ID.
         * It is used in the paper to calculate the histogram for a stream.
         *
         * @param d The delay value
         * @param stream_id The stream ID
         * @return double The probability density function value
         */
        auto fD(int d, int stream_id) -> double;

        INTELLI::ConfigMapPtr cfg{};  // Configuration object

        std::vector<int> RStatMap{0};  // Map recording the RStat values for each stream

        std::vector<std::vector<TrackTuple>> recordMap{};  // Map recording the input history of each stream

        std::vector<int> tMap{0};  // Map recording the T values of each stream

        std::vector<int> kMap{0};  // Map recording the K values of each stream

        std::vector<std::vector<int>> kSyncMap{};  // Map recording all K_sync values, for sampling and predicting future ksync

        std::vector<std::vector<double>> histogramMap{};  // Histogram map
        std::vector<std::vector<int>> histogramPos{};  // Map recording the histogram positions for each stream

        TupleProductivityProfiler* productivityProfiler;  // Tuple productivity profiler
    };

}

#endif //DISORDERHANDLINGSYSTEM_STATISTICS_MANAGER_H
