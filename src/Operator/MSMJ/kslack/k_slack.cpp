//
// Created by 86183 on 2023/1/4.
//

#include <iostream>
#include <utility>
#include "Operator/MSMJ/kslack/k_slack.h"

using namespace OoOJoin;

KSlack::KSlack(StreamPtr stream, BufferSizeManagerPtr buffer_size_manager, StatisticsManagerPtr statistics_manager,
               SynchronizerPtr synchronizer, phmap::parallel_flat_hash_map<uint64_t, Stream *> stream_map) {
    stream_ = std::move(stream);
    buffer_size_manager_ = std::move(buffer_size_manager);
    statistics_manager_ = std::move(statistics_manager);
    synchronizer_ = std::move(synchronizer);
    stream_map_ = std::move(stream_map);
}


auto KSlack::get_output() -> std::queue<OoOJoin::TrackTuple> {
    return watch_output_;
}

auto KSlack::get_id() -> uint64_t {
    return stream_->get_id();
}

//K-Slack算法对无序流进行处理
auto KSlack::disorder_handling() -> void {
    uint64_t L = opConfig->getU64("L");
    while (!stream_->get_tuple_list().empty()) {
        TrackTuple tuple = stream_->get_tuple_list().front();

        //更新local time
        current_time_ = std::max(current_time_, tuple.eventTime);

        //每L个时间单位调整K值
        if (current_time_ != 0 && current_time_ % L == 0) {
            buffer_size_ = buffer_size_manager_->k_search(stream_->get_id());
        }

        //计算出tuple的delay,T - ts, 方便统计管理器统计记录
        tuple.delay = current_time_ - tuple.eventTime;

        //加入statistics_manager的历史记录统计表以及T值
        statistics_manager_->add_record(stream_->get_id(), tuple);
        statistics_manager_->add_record(stream_->get_id(), current_time_, buffer_size_);

        //先让缓冲区所有满足条件的tuple出队进入输出区
        while (!buffer_.empty()) {
            TrackTuple tuple = *buffer_.begin();

            //对应论文的公式：ei. ts + Ki <= T
            if (tuple.eventTime + buffer_size_ > current_time_) {
                break;
            }

            //满足上述公式，加入输出区
            output_.push(tuple);
            watch_output_.push(tuple);
            buffer_.erase(buffer_.begin());
        }
        stream_->pop_tuple();

        //加入tuple进入buffer
        buffer_.insert(tuple);

        //将output_加入同步器
        synchronizer_->synchronize_stream(output_);

    }

    //将buffer区剩下的元素加入output
    while (!buffer_.empty()) {
        output_.push(*buffer_.begin());
        watch_output_.push(*buffer_.begin());
        buffer_.erase(buffer_.begin());
    }
    //将剩下的output_加入同步器
    synchronizer_->synchronize_stream(output_);

}

auto KSlack::setConfig(INTELLI::ConfigMapPtr opConfig) -> void {
    this->opConfig = std::move(opConfig);
}


