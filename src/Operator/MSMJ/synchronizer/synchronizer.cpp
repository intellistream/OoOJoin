//
// Created by 86183 on 2023/1/7.
//

#include <iostream>
#include <utility>
#include "Operator/MSMJ/synchronizer/synchronizer.h"

Synchronizer::Synchronizer(uint64_t stream_count, StreamOperatorPtr stream_operator) {
    stream_count_ = stream_count;
    stream_operator_ = std::move(stream_operator);
}


auto Synchronizer::get_output() -> std::queue<TrackTuple> {
    return watch_output_;
}


//从k-slack发送过来的流
auto Synchronizer::synchronize_stream(std::queue<TrackTuple> &input) -> void {
    std::lock_guard<std::mutex> lock(latch_);
    while (!input.empty()) {
        TrackTuple tuple = input.front();
        int stream_id = tuple.streamId;
        input.pop();
        if (tuple.eventTime > T_sync_) {
            if (sync_buffer_map_.find(stream_id) == sync_buffer_map_.end() || sync_buffer_map_[stream_id].empty()) {
                //下一步要插入tuple了
                own_stream_++;
            }
            sync_buffer_map_[stream_id].insert(tuple);
            //检测是否缓冲区拥有所有流的tuple
            while (own_stream_ == stream_count_) {
                //找到Tsync
                T_sync_ = INT8_MAX;
                for (auto it: sync_buffer_map_) {
                    T_sync_ = std::min((uint64_t) T_sync_, it.second.begin()->eventTime);
                }

                for (auto &it: sync_buffer_map_) {
                    //将所有等于Tsync的元组输出
                    while (it.second.begin()->eventTime == T_sync_) {
                        output_.push(*it.second.begin());
                        watch_output_.push(*it.second.begin());
                        it.second.erase(it.second.begin());
                    }
                    if (it.second.empty()) {
                        own_stream_--;
                    }
                }
            }
        } else {
            output_.push(tuple);
            watch_output_.push(tuple);
        }
        stream_operator_->mswj_execution(output_);
    }
}

auto Synchronizer::setConfig(INTELLI::ConfigMapPtr opConfig) -> void {
    this->opConfig = std::move(opConfig);
}

