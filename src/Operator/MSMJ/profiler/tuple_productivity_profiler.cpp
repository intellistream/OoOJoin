//
// Created by 86183 on 2023/1/9.
//


#include "Operator/MSMJ/profiler/tuple_productivity_profiler.h"

#include <utility>

auto TupleProductivityProfiler::get_join_record_map() -> phmap::parallel_flat_hash_map<int, int> {
    return join_record_map_;
}

auto TupleProductivityProfiler::add_join_record(int stream_id, int count) -> void {
    std::lock_guard<std::mutex> lock(latch_);
    join_record_map_[stream_id] = count;
}

auto TupleProductivityProfiler::update_cross_join(int Di, int res) -> void {
    std::lock_guard<std::mutex> lock(latch_);
    cross_join_map_[Di] = res;
}

auto TupleProductivityProfiler::update_join_res(int Di, int res) -> void {
    std::lock_guard<std::mutex> lock(latch_);
    join_result_map_[Di] = res;
}

auto TupleProductivityProfiler::get_select_ratio(int K) -> double {
    std::lock_guard<std::mutex> lock(latch_);
    if (join_result_map_.empty() || cross_join_map_.empty()) {
        return 1;

    }
    int M_sum = 0;
    int Mx_sum = 0;
    for (int d = 0; d <= K; d++) {
        M_sum += join_result_map_[d];
        Mx_sum += cross_join_map_[d];
    }
    int M_DM_sum = 0;
    int Mx_DM_sum = 0;

    if (join_result_map_.empty()) {
        return 1;
    }

    for (int d = 0; d <= (--join_result_map_.end())->first && join_result_map_.find(d) != join_result_map_.end() &&
                         cross_join_map_.find(d) != cross_join_map_.end(); d++) {
        Mx_DM_sum += cross_join_map_[d];
        M_DM_sum += join_result_map_[d];
    }
    double res = (M_sum * 1.0 / Mx_sum) * (Mx_DM_sum * 1.0 / M_DM_sum);
    return res;
}

auto TupleProductivityProfiler::get_requirement_recall() -> double {
    std::lock_guard<std::mutex> lock(latch_);

    double userRecall = opConfig->getDouble("userRecall");
    uint64_t P = opConfig->getU64("P");
    uint64_t L = opConfig->getU64("L");

    int max_D = 0;
    if (!cross_join_map_.empty()) {
        max_D = (--cross_join_map_.end())->first;
    }

    int N_true_L = 0;
    for (int d = 0; d <= max_D && join_result_map_.find(d) != join_result_map_.end(); d++) {
        N_true_L += join_result_map_[d];
    }

    int N_true_P_L = 0;
    for (int d = max_D * (1 - (P - L) / L); d <= max_D && join_result_map_.find(d) != join_result_map_.end(); d++) {
        N_true_P_L += join_result_map_[d];
    }

    int N_prod_P_L = 0;
    for (int d = max_D * (1 - (P - L) / L); d <= max_D && join_result_map_.find(d) != join_result_map_.end(); d++) {
        N_prod_P_L += join_result_map_[d];
    }

    //requirement_recall大于等于这个值
    if (N_true_L == 0)return userRecall;
    double requirement_recall = (userRecall * (N_true_P_L + N_true_L) - N_prod_P_L) * 1.0 / N_true_L;
    return requirement_recall;
}

auto TupleProductivityProfiler::setConfig(INTELLI::ConfigMapPtr opConfig) -> void {
    this->opConfig = std::move(opConfig);
}
