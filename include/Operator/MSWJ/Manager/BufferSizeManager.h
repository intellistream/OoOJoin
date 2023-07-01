//
// Created by 86183 on 2023/1/5.
//

#ifndef DISORDERHANDLINGSYSTEM_BUFFER_SIZE_MANAGER_H
#define DISORDERHANDLINGSYSTEM_BUFFER_SIZE_MANAGER_H

#include <Operator/MSWJ/Manager/StatisticsManager.h>
#include <Operator/MSWJ/Profiler/TupleProductivityProfiler.h>

/**
 * @class BufferSizeManager
 * @ingroup ADB_OPERATORS
 * @class MSWJOperator Operator/BufferSizeManager.h
 * @brief Implementation of the BufferSizeManager for adaptive buffer size management
 */
namespace MSWJ {

    class BufferSizeManager {
    public:
        /**
         * @brief Constructor with required parameters
         *
         * @param statisticsManager Pointer to the StatisticsManager object
         * @param profiler Pointer to the TupleProductivityProfiler object
         */
        explicit BufferSizeManager(StatisticsManager* statisticsManager, TupleProductivityProfiler* profiler);

        /**
         * @brief Destructor
         */
        ~BufferSizeManager() = default;

        /**
         * @brief Adaptive K algorithm
         *
         * This function implements the adaptive K algorithm described in the paper "Quality-Driven Disorder Handling for M-way Sliding Window Stream Joins".
         * It returns the recommended buffer size K for the given stream ID.
         *
         * @param stream_id The stream ID
         * @return int The recommended buffer size K
         */
        auto kSearch(int stream_id) -> int;

        /**
         * @brief Sets the configuration for the BufferSizeManager
         *
         * @param config Pointer to the ConfigMap object
         */
        auto setConfig(INTELLI::ConfigMapPtr config) -> void;

    private:
        /**
         * @brief Function γ(L,T) in the paper
         *
         * This function calculates the value of γ(L,T) for a given buffer size L and tuple arrival rate T.
         * It is used in the adaptive K algorithm to calculate the recommended buffer size K for a stream.
         *
         * @param K The buffer size L
         * @return double The value of γ(L,T)
         */
        auto y(int K) -> double;

        // Configuration object
        INTELLI::ConfigMapPtr cfg = nullptr;

        // Data statistics manager
        StatisticsManager* statisticsManager;

        // Tuple productivity profiler
        TupleProductivityProfiler* productivityProfiler;
    };
}

#endif //DISORDERHANDLINGSYSTEM_BUFFER_SIZE_MANAGER_H
