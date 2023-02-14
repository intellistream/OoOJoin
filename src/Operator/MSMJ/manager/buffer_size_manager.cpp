//
// Created by 86183 on 2023/1/5.
//

#include "Operator/MSMJ/manager/buffer_size_manager.h"

#include <utility>

BufferSizeManager::BufferSizeManager(StatisticsManagerPtr statistics_manager, TupleProductivityProfilerPtr profiler,
                                     phmap::parallel_flat_hash_map<int, Stream *> stream_map) {
    statistics_manager_ = std::move(statistics_manager);
    productivity_profiler_ = std::move(profiler);
    stream_map_ = std::move(stream_map);
}


/**
 *
 * @param L  - buffer-size manager的自适应时间间隔
 * @param g  K*搜索粒度
 */
auto BufferSizeManager::k_search(int stream_id) -> int {
    int max_DH = statistics_manager_->get_maxD(stream_id);

    if (max_DH == 0) {
        return 1;
    }

    uint64_t g = 1;
    if (opConfig->exist("g")) {
        g = opConfig->getU64("g");
    } else {
        INTELLI::INTELLI_INFO("Param g should be given");
    }

    int k = 0;
    while (k <= max_DH && y(k) < productivity_profiler_->get_requirement_recall()) {
        k = k + g;
    }
    return k;
}

auto BufferSizeManager::y(int K) -> double {
    std::lock_guard<std::mutex> lock(latch_);
    //SEL比值
    double sel_radio = productivity_profiler_->get_select_ratio(K);

    int m = stream_map_.size();

    //basicWindow
    uint64_t b = opConfig->getU64("b");

    //分子
    int numerator = 0;
    for (int i = 1; i <= m; i++) {
        int res = 1;
        for (int j = 1; j <= m; j++) {
            if (j == i) {
                continue;
            }
            int wj = stream_map_[j]->get_window_size();
            int nj = wj / b;
            int sum = 0;
            for (int l = 1; l <= nj; l++) {
                sum += statistics_manager_->wil(l, j, K);
            }
            res *= sum;
        }
        numerator += statistics_manager_->fDk(0, i, K) * res;
    }

    //分母
    int denominator = 0;
    for (int i = 1; i <= m; i++) {
        int res = 1;
        for (int j = 1; j <= m; j++) {
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

    return static_cast<int>(sel_radio * numerator / denominator);
}

auto BufferSizeManager::setConfig(INTELLI::ConfigMapPtr opConfig) -> void {
    this->opConfig = std::move(opConfig);
}



