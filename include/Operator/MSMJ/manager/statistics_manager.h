//
// Created by 86183 on 2023/1/6.
//

#ifndef DISORDERHANDLINGSYSTEM_STATISTICS_MANAGER_H
#define DISORDERHANDLINGSYSTEM_STATISTICS_MANAGER_H


#include <vector>
#include <unordered_map>
#include <mutex>
#include <parallel-hashmap/parallel_hashmap/phmap.h>
#include "Operator/MSMJ/common/define.h"
#include "Operator/MSMJ/profiler/tuple_productivity_profiler.h"
#include "Utils/ConfigMap.hpp"

namespace MSMJ {

    class StatisticsManager {
    public:

        explicit StatisticsManager(TupleProductivityProfiler *profiler, INTELLI::ConfigMapPtr config);

        ~StatisticsManager() = default;

        //获得元组最大delay
        auto get_maxD(int stream_id) -> int;

        //离散随机变量Dik的概率分布函数fDiK， Dik表示连接算子在k设置下接受相应流中一个元组的粗粒度延迟
        auto fDk(int d, int stream_id, int K) -> double;

        //|wi^l|的估计
        auto wil(int l, int stream_id, int K) -> int;

        auto add_record(int stream_id, Tuple tuple) -> void;

        auto add_record(int stream_id, int T, int K) -> void;

        auto setConfig(INTELLI::ConfigMapPtr config) -> void;

        //获得离散随机变量Di的值,如果delay(ei) ∈(kg,(k+1)g]，则Di=k+1
        auto inline get_D(int delay) -> int {
            int g = cfg->getU64("g");
            return delay % g == 0 ? delay / g : delay / g + 1;
        }

    private:
        //获得Ksync
        auto get_ksync(int stream_id) -> int;

        //获取avg(ksync)
        auto get_avg_ksync(int stream_id) -> int;

        //估计未来的ksync
        auto get_future_ksync(int stream_id) -> int;

        //参考文献[25]的自适应窗口方法, 传入的主要参数待定，需阅读文献[25]
        auto get_R_stat(int stream_id) -> int;

        //离散随机变量Di的概率分布函数fDi
        auto fD(int d, int stream_id) -> double;

        INTELLI::ConfigMapPtr cfg = nullptr;


        //Rstat窗口大小
        std::vector<int> R_stat_map_{0};

        //历史流Si输入记录的映射
        std::vector<std::vector<Tuple>> record_map_{};

        //历史流Si的T记录
        std::vector<int> T_map_{0};

        //历史流的K记录
        std::vector<int> K_map_{0};

        //保存所有的K_sync，方便抽取样本预测未来的ksync
        std::vector<std::vector<int>> ksync_map_{};

        //直方图映射
        std::vector<std::vector<double>> histogram_map_{};
        std::vector<std::vector<int>> histogram_pos_{};

        //元组生产力
        TupleProductivityProfiler *productivity_profiler_;
    };

}

#endif //DISORDERHANDLINGSYSTEM_STATISTICS_MANAGER_H
