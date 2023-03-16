//
// Created by 86183 on 2023/1/5.
//

#include "Operator/MSMJ/manager/buffer_size_manager.h"
#include "Operator/MSMJ/profiler/tuple_productivity_profiler.h"
#include "Operator/MSMJ/common/define.h"

using namespace MSMJ;

BufferSizeManager::BufferSizeManager(StatisticsManager *statistics_manager, TupleProductivityProfiler *profiler) :
        statistics_manager_(statistics_manager),
        productivity_profiler_(profiler) {}


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

    int k = 0;

    auto recall = productivity_profiler_->get_requirement_recall();
    while (k <= max_DH && y(k) < recall) {
        k = k + g;
    }
    return k == 0 ? 1 : k;
}

auto BufferSizeManager::y(int K) -> double {
    //SEL比值
    double sel_radio = productivity_profiler_->get_select_ratio(K);

    int m = streamCount;

    //分子
    int numerator = 0;
    for (int i = 1; i <= m; i++) {
        int res = 1;
        for (int j = 1; j <= m; j++) {
            if (j == i) {
                continue;
            }
            std::string key = "Stream_" + std::to_string(j);
            int wj = cfg->getU64(key);
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
            std::string key = "Stream_" + std::to_string(j);
            res *= cfg->getU64(key);;
        }
        denominator += res;
    }

    if (denominator == 0 || numerator == 0 || sel_radio == 0) {
        return 1;
    }

    return static_cast<int>(sel_radio * numerator / denominator);
}

auto BufferSizeManager::setConfig(INTELLI::ConfigMapPtr config) -> void {
    cfg = std::move(config);
}



