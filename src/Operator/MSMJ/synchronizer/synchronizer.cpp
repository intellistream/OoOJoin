//
// Created by 86183 on 2023/1/7.
//

#include <iostream>
#include <future>
#include "Operator/MSMJ/synchronizer/synchronizer.h"
#include "Operator/MSMJ/operator/stream_operator.h"

using namespace MSMJ;

Synchronizer::Synchronizer(int stream_count, StreamOperator *stream_operator) : stream_count_(stream_count),
                                                                                stream_operator_(stream_operator) {
    sync_buffer_map_.resize(3);
}


auto Synchronizer::get_output() -> std::queue<Tuple> {
    return watch_output_;
}


//从k-slack发送过来的流
auto Synchronizer::synchronize_stream(Tuple *syn_tuple) -> void {
    Tuple tuple = *syn_tuple;
    int stream_id = tuple.streamId;
    if (stream_id != 1 && stream_id != 2) {
        std::cout << "err" << std::endl;
    }
    if (tuple.ts > T_sync_) {
        if (sync_buffer_map_[stream_id].empty()) {
            //下一步要插入tuple了
            own_stream_++;
        }
        sync_buffer_map_[stream_id].insert(tuple);
        //检测是否缓冲区拥有所有流的tuple
        while (own_stream_ == stream_count_) {
            //找到Tsync
            T_sync_ = INT32_MAX;
            for (auto it: sync_buffer_map_) {
                if (it.empty()) {
                    continue;
                }
                T_sync_ = std::min(T_sync_, it.begin()->ts);
            }

            for (auto &it: sync_buffer_map_) {
                if (it.empty()) {
                    continue;
                }
                //将所有等于Tsync的元组输出
                while (it.begin()->ts == T_sync_) {
                    stream_operator_->mswj_execution(&(*it.begin()));

                    it.erase(it.begin());
                }
                if (it.empty()) {
                    own_stream_--;
                }
            }
        }
    } else {
        stream_operator_->mswj_execution(syn_tuple);

    }


}

