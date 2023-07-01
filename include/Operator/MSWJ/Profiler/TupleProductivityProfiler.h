//
// Created by 86183 on 2023/1/9.
//

#ifndef DISORDERHANDLINGSYSTEM_TUPLE_PRODUCTIVITY_PROFILER_H
#define DISORDERHANDLINGSYSTEM_TUPLE_PRODUCTIVITY_PROFILER_H

#include <unordered_map>
#include <map>
#include <mutex>
#include <Utils/ConfigMap.hpp>
#include <Operator/MSWJ/Common/MSWJDefine.h>

namespace MSWJ {

    /**
     * @class TupleProductivityProfiler
     * @brief Used to record and analyze tuple productivity and performance.
     */
    class TupleProductivityProfiler {
    public:
        /**
         * @brief Constructor that takes a ConfigMapPtr argument for setting configurations.
         * @param config ConfigMapPtr type, configuration parameters.
         * @note This constructor initializes the join record map, cross-join map, join result map, and their positions.
         */
        explicit TupleProductivityProfiler(INTELLI::ConfigMapPtr config);

        /**
         * @brief Default destructor.
         */
        ~TupleProductivityProfiler() = default;

        /**
         * @brief Returns the join record map.
         * @return std::vector<int> type, the join record map.
         */
        auto getJoinRecordMap() -> std::vector<int>;

        /**
         * @brief Adds a join record based on the given stream ID and count.
         * @param stream_id int type, the stream ID.
         * @param count int type, the count of tuples to add.
         * @return void.
         * @note This function updates the join record map.
         */
        auto addJoinRecord(int stream_id, int count) -> void;

        /**
         * @brief Updates the cross-join result size based on the given D and res values.
         * @param Di int type, the value of discrete random variable D.
         * @param res size_t type, the size of the cross-join result.
         * @return void.
         * @note This function updates the cross-join map and its position.
         */
        auto updateCrossJoin(int Di, size_t res) -> void;

        /**
         * @brief Updates the join result size based on the given D and res values.
         * @param Di int type, the value of discrete random variable D.
         * @param res size_t type, the size of the join result.
         * @return void.
         * @note This function updates the join result map and its position.
         */
        auto updateJoinRes(int Di, size_t res) -> void;

        /**
         * @brief Returns the selectivity ratio based on the given K value.
         * @param K int type, the selectivity ratio.
         * @return double type, the selectivity ratio.
         * @note This function calculates the selectivity ratio based on the join record map.
         */
        auto getSelectRatio(int K) -> double;

        /**
         * @brief Returns the requirement recall.
         * @return double type, the requirement recall.
         * @note This function calculates the requirement recall based on the join result map.
         */
        auto getRequirementRecall() -> double;

        /**
         * @brief Sets the configuration based on the given ConfigMapPtr argument.
         * @param config ConfigMapPtr type, configuration parameters.
         * @return void.
         */
        auto setConfig(INTELLI::ConfigMapPtr config) -> void;

        /**
         * @brief Returns the value of the discrete random variable D based on the given delay value.
         * @param delay int type, the delay value.
         * @return int type, the value of the discrete random variable D.
         * @note If delay(ei) ∈(kg,(k+1)g], then Di=k+1.
         */
        static auto inline getD(int delay) -> int {
            return delay % g == 0 ? delay / g : delay / g + 1;
        }

    private:
        INTELLI::ConfigMapPtr cfg{};

        // Records the number of tuples that arrive at the join operator
        std::vector<int> joinRecordMap{};

        // Records the size of the cross-join result
        std::vector <size_t> crossJoinMap{0};
        std::vector<int> crossJoinPos{};

        // Records the size of the join result
        std::vector <size_t> joinResultMap{0};
        std::vector<int> joinResultPos{};
    };

}
#endif //DISORDERHANDLINGSYSTEM_TUPLE_PRODUCTIVITY_PROFILER_H
