//
// Created by 86183 on 2023/1/8.
//

#ifndef DISORDERHANDLINGSYSTEM_STREAM_OPERATOR_H
#define DISORDERHANDLINGSYSTEM_STREAM_OPERATOR_H


#include <queue>
#include <unordered_map>
#include <list>
#include <mutex>
#include "Operator/MSMJ/profiler/tuple_productivity_profiler.h"
#include "Operator/MSMJ/common/define.h"

namespace MSMJ {


    class StreamOperator {
    public:

        explicit StreamOperator(TupleProductivityProfiler *profiler, INTELLI::ConfigMapPtr config);

        ~StreamOperator() = default;

        auto mswj_execution(Tuple *tuple) -> void;

        auto inline get_result() -> std::queue<std::vector<Tuple>> {
            return result_;
        }

        auto inline getJoinResultCount() const -> int {
            return joinResultCount_;
        }

        auto setConfig(INTELLI::ConfigMapPtr config) -> void;

        //获得离散随机变量Di的值,如果delay(ei) ∈(kg,(k+1)g]，则Di=k+1
        static auto inline get_D(int delay) -> int {
            return delay % g == 0 ? delay / g : delay / g + 1;
        }

    private:

        static auto inline can_join_(Tuple t1, Tuple t2) -> bool {
            return t1.key == t2.key;
        };

        INTELLI::ConfigMapPtr cfg = nullptr;

        int joinResultCount_{};

        //连接时的T
        int T_op_{};

        //window map
        std::vector<std::list<Tuple>> window_map_{};

        //结果元组
        std::queue<std::vector<Tuple>> result_{};

        //元组生产力监视器
        TupleProductivityProfiler *productivity_profiler_;

    };

}


#endif //DISORDERHANDLINGSYSTEM_STREAM_OPERATOR_H
