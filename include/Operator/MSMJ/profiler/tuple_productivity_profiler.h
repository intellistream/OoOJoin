//
// Created by 86183 on 2023/1/9.
//

#ifndef DISORDERHANDLINGSYSTEM_TUPLE_PRODUCTIVITY_PROFILER_H
#define DISORDERHANDLINGSYSTEM_TUPLE_PRODUCTIVITY_PROFILER_H


#include <unordered_map>
#include <map>
#include <mutex>
#include "parallel-hashmap/parallel_hashmap/phmap.h"
#include "Utils/ConfigMap.hpp"
#include <parallel-hashmap/parallel_hashmap/btree.h>

namespace MSMJ {


    class TupleProductivityProfiler {
    public:

        TupleProductivityProfiler(INTELLI::ConfigMapPtr config);

        ~TupleProductivityProfiler() = default;

        auto get_join_record_map() -> std::vector<int>;

        auto add_join_record(int stream_id, int count) -> void;

        auto update_cross_join(int Di, int res) -> void;

        auto update_join_res(int Di, int res) -> void;

        auto get_select_ratio(int K) -> double;

        auto get_requirement_recall() -> double;

        auto setConfig(INTELLI::ConfigMapPtr config) -> void;

    private:

        INTELLI::ConfigMapPtr cfg = nullptr;

        //到达join operator的元组数量记录
        std::vector<int> join_record_map_{};

        //the join operator records both the number of cross-join result size,
        std::vector<int> cross_join_map_{0};
        std::vector<int> cross_join_pos_{};

        //the number of join results, using map for sorting
        std::vector<int> join_result_map_{0};
        std::vector<int> join_result_pos_{};

    };

}
#endif //DISORDERHANDLINGSYSTEM_TUPLE_PRODUCTIVITY_PROFILER_H
