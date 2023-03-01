//
// Created by 86183 on 2023/1/9.
//

#include <iostream>
#include <utility>
#include "Operator/MSMJ/profiler/tuple_productivity_profiler.h"
#include "parallel-hashmap/parallel_hashmap/phmap.h"
#include "Operator/MSMJ/common/define.h"

using namespace MSMJ;

auto TupleProductivityProfiler::get_join_record_map() -> std::vector<int> {
    return join_record_map_;
}

auto TupleProductivityProfiler::add_join_record(int stream_id, int count) -> void {
    join_record_map_[stream_id] = count;
}

auto TupleProductivityProfiler::update_cross_join(int Di, int res) -> void {
    cross_join_map_[Di] = res;
    cross_join_pos_.push_back(Di);
}

auto TupleProductivityProfiler::update_join_res(int Di, int res) -> void {
    join_result_map_[Di] = res;
    join_result_pos_.push_back(Di);
}

auto TupleProductivityProfiler::get_select_ratio(int K) -> double {
    if (cross_join_pos_.empty() || join_result_pos_.empty()) {
        return 1;
    }

    int M_sum = 0;
    int Mx_sum = 0;

    int temp = 0;
    for (int d = 0; d < join_result_pos_.size(); d++) {
        if (temp == cross_join_pos_[d])continue;
        temp = cross_join_pos_[d];
        if (join_result_pos_[d] > K) {
            continue;
        }
        M_sum += join_result_map_[join_result_pos_[d]];
        Mx_sum += cross_join_map_[cross_join_pos_[d]];
    }

    //边界情况
    if (Mx_sum == 0) {
        return 1;
    }

    int M_DM_sum = 0;
    int Mx_DM_sum = 0;

    temp = 0;
    for (int d = 0; d < join_result_pos_.size(); d++) {
        if (temp == cross_join_pos_[d])continue;
        temp = cross_join_pos_[d];
        Mx_DM_sum += cross_join_map_[cross_join_pos_[d]];
        M_DM_sum += join_result_map_[join_result_pos_[d]];
    }
    double res = (M_sum * 1.0 / Mx_sum) * (Mx_DM_sum * 1.0 / M_DM_sum);
    return res;
}

auto TupleProductivityProfiler::get_requirement_recall() -> double {
    if (cross_join_map_.empty()) {
        return userRecall;
    }

    int max_D = 0;

    for (int i = 0; i < join_result_pos_.size(); i++) {
        max_D = std::max(max_D, cross_join_map_[join_result_pos_[i]]);
    }

    int N_true_L = 0;
    int temp = -1;
    for (int d = 0; d < join_result_pos_.size(); d++) {
        if (temp == join_result_pos_[d])continue;
        temp = join_result_pos_[d];
        N_true_L += join_result_map_[join_result_pos_[d]];
    }

    int N_true_P_L = 0;
    temp = -1;
    for (int d = 0; d < join_result_pos_.size(); d++) {
        if (temp == join_result_pos_[d])continue;
        temp = join_result_pos_[d];
        if (d < max_D * (1 - (P - L) / L))continue;
        N_true_P_L += join_result_map_[join_result_pos_[d]];
    }

    int N_prod_P_L = 0;
    temp = -1;
    for (int d = 0; d < join_result_pos_.size(); d++) {
        if (temp == join_result_pos_[d])continue;
        temp = join_result_pos_[d];
        if (d < max_D * (1 - (P - L) / L))continue;
        N_prod_P_L += join_result_map_[join_result_pos_[d]];
    }

    if (N_true_L == 0) {
        return userRecall;
    }
    //requirement_recall大于等于这个值
    double requirement_recall = (userRecall * (N_true_P_L + N_true_L) - N_prod_P_L) * 1.0 / N_true_L;
    if (requirement_recall > 1) {
        std::cout << "h";
    }
    return std::max(requirement_recall, (double) 1);
}

TupleProductivityProfiler::TupleProductivityProfiler(INTELLI::ConfigMapPtr config) :
        cfg(std::move(config)) {
    join_record_map_.resize(streamCount + 1);
    cross_join_map_.resize(maxDelay);
    join_result_map_.resize(maxDelay);
}

auto TupleProductivityProfiler::setConfig(INTELLI::ConfigMapPtr config) -> void {
    cfg = std::move(config);
}
