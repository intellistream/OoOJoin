//
// Created by 86183 on 2023/1/5.
//

#include "Operator/MSMJ/manager/buffer_size_manager.h"

#include <utility>

BufferSizeManager::BufferSizeManager(StatisticsManagerPtr statistics_manager, TupleProductivityProfilerPtr profiler) {
    statistics_manager_ = std::move(statistics_manager);
    productivity_profiler_ = std::move(profiler);
}


/**
 *
 * @param L  - buffer-size manager的自适应时间间隔
 * @param g  K*搜索粒度
 */
auto BufferSizeManager::k_search(uint64_t stream_id) -> uint64_t {
    uint64_t max_DH = statistics_manager_->get_maxD(stream_id);

    if (max_DH == 0) {
        return 1;
    }

    uint64_t g = opConfig->getI64("g");

    uint64_t k = 0;
    while (k <= max_DH && y(k) < productivity_profiler_->get_requirement_recall()) {
        k = k + g;
    }
    return k;
}

auto BufferSizeManager::y(uint64_t K) -> double {
    std::lock_guard<std::mutex> lock(latch_);
    //SEL比值
    double sel_radio = productivity_profiler_->get_select_ratio(K);

    uint64_t m = stream_map_.size();

    //basicWindow
    uint64_t b = opConfig->getI64("b");

    //分子
    uint64_t numerator = 0;
    for (uint64_t i = 1; i <= m; i++) {
        uint64_t res = 1;
        for (uint64_t j = 1; j <= m; j++) {
            if (j == i) {
                continue;
            }
            uint64_t wj = stream_map_[j]->get_window_size();
            uint64_t nj = wj / b;
            uint64_t sum = 0;
            for (uint64_t l = 1; l <= nj; l++) {
                sum += statistics_manager_->wil(l, j, K);
            }
            res *= sum;
        }
        numerator += statistics_manager_->fDk(0, i, K) * res;
    }

    //分母
    uint64_t denominator = 0;
    for (uint64_t i = 1; i <= m; i++) {
        uint64_t res = 1;
        for (uint64_t j = 1; j <= m; j++) {
            if (j == i) {
                continue;
            }
            res *= stream_map_[j]->get_window_size();
        }
        denominator += res;
    }

    if (denominator == 0) {
        return 1;
    }

    return static_cast<uint64_t>(sel_radio * numerator / denominator);
}



