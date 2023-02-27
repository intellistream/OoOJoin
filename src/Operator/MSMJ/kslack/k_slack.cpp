//
// Created by 86183 on 2023/1/4.
//

#include <iostream>
#include <future>
#include "Operator/MSMJ/kslack/k_slack.h"
#include "Operator/MSMJ/common/define.h"
#include "Operator/MSMJ/synchronizer/synchronizer.h"
#include "Operator/MSMJ/manager/buffer_size_manager.h"

using namespace MSMJ;

KSlack::KSlack(Stream *stream, BufferSizeManager *buffer_size_manager, StatisticsManager *statistics_manager,
               Synchronizer *synchronizer) :
        buffer_size_manager_(buffer_size_manager),
        statistics_manager_(statistics_manager),
        synchronizer_(synchronizer) {

    tuple_list_ = std::move(stream->get_tuple_list());
    stream_id_ = stream->get_id();
}


KSlack::~KSlack() {

}


auto KSlack::get_id() -> int {
    return stream_id_;
}

//K-Slack算法对无序流进行处理
auto KSlack::disorder_handling() -> void {
    while (!tuple_list_.empty()) {
        Tuple tuple = tuple_list_.front();

        //更新local time
        current_time_ = std::max(current_time_, tuple.ts);

        //每L个时间单位调整K值
        if (current_time_ != 0 && current_time_ % L == 0) {
            buffer_size_ = buffer_size_manager_->k_search(stream_id_);
        }

        //计算出tuple的delay,T - ts, 方便统计管理器统计记录
        tuple.delay = current_time_ - tuple.ts;

        //将output_加入同步器
        //加入statistics_manager的历史记录统计表以及T值
        statistics_manager_->add_record(stream_id_, tuple);
        statistics_manager_->add_record(stream_id_, current_time_, buffer_size_);

        //先让缓冲区所有满足条件的tuple出队进入输出区
        while (!buffer_.empty()) {
            Tuple tuple = *buffer_.begin();

            //对应论文的公式：ei. ts + Ki <= T
            if (tuple.ts + buffer_size_ > current_time_) {
                break;
            }

            //满足上述公式，加入输出区

            //加入同步器
            synchronizer_->synchronize_stream(tuple);


            buffer_.erase(buffer_.begin());
        }
        tuple_list_.pop();

        //加入tuple进入buffer
        buffer_.insert(tuple);


    }

    //将buffer区剩下的元素加入output
    while (!buffer_.empty()) {


        //加入同步器
        synchronizer_->synchronize_stream(*buffer_.begin());


        buffer_.erase(buffer_.begin());
    }


}


