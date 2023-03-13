//
// Created by 86183 on 2023/1/8.
//



#include <future>
#include <utility>
#include "Operator/MSMJ/operator/stream_operator.h"
#include "Operator/MSMJ/common/define.h"

using namespace MSMJ;

StreamOperator::StreamOperator(TupleProductivityProfiler *profiler, INTELLI::ConfigMapPtr config) :
        cfg(std::move(config)), productivity_profiler_(profiler) {
    window_map_.resize(streamCount + 1);
}


auto StreamOperator::mswj_execution(Tuple *join_tuple) -> void {
    Tuple tuple = *join_tuple;
    int stream_id = tuple.streamId;
    //计算Di
    int delay = tuple.delay;

    //计算cross-join的结果大小
    int cross_join = 1;

    if (tuple.ts >= T_op_) {
        T_op_ = tuple.ts;

        for (int i = 0; i < window_map_.size(); i++) {
            if (window_map_[i].empty()) {
                continue;
            }
            //统计window内元组数量数据
            productivity_profiler_->add_join_record(stream_id, window_map_[i].size());

            if (i == stream_id) {
                continue;
            }

            for (auto iter = window_map_[i].begin(); iter != window_map_[i].end();) {
                Tuple tuple_j = *iter;
                cross_join++;
                if (tuple_j.ts < tuple.ts - window_map_[i].size()) {
                    window_map_[i].erase(iter++);
                    cross_join--;
                } else {
                    iter++;
                }
            }

        }

        //更新cross_join_map

        productivity_profiler_->update_cross_join(get_D(delay), cross_join);

        //连接
        std::unordered_map<int, std::vector<Tuple>> tempJoinMap;
        int res = 1;
        for (int i = 0; i < window_map_.size(); i++) {
            if (window_map_[i].empty()) {
                continue;
            }

            if (i == stream_id) {
                continue;
            }
            while (!window_map_[i].empty()) {
                Tuple tuple_j = window_map_[i].front();
                window_map_[i].pop_front();
                if (can_join_(tuple, tuple_j)) {
                    //时间戳定义为ei.ts
                    tuple_j.ts = tuple.ts;
                    tempJoinMap[i].push_back(tuple_j);
                }
            }
        }

        int tempCount = 1;
        //统计res,先统计二路join
        for (const auto &it: tempJoinMap) {
            tempCount *= it.second.size();
            for (auto l: it.second) {
                std::vector<Tuple> tempVec;
                tempVec.push_back(tuple);
                tempVec.push_back(l);
                result_.push(tempVec);
            }
        }

        res += tempCount;

        joinResultCount_ += res;

        //更新join result map
        productivity_profiler_->update_join_res(get_D(delay), res);

        window_map_[stream_id].push_back(tuple);
    } else if (tuple.ts > T_op_ - window_map_[stream_id].size()) {
        window_map_[stream_id].push_back(tuple);
    }
}

auto StreamOperator::setConfig(INTELLI::ConfigMapPtr config) -> void {
    cfg = std::move(config);
}






