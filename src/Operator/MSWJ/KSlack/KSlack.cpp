//
// Created by 86183 on 2023/1/4.
//

#include <iostream>
#include <future>
#include "Operator/MSWJ/KSlack/KSlack.h"
#include "Operator/MSWJ/Common/MSWJDefine.h"
#include "Operator/MSWJ/Synchronizer/StreamSynchronizer.h"
#include "Operator/MSWJ/Manager/BufferSizeManager.h"

using namespace MSWJ;

KSlack::KSlack(int streamId, BufferSizeManager *bufferSizeManager, StatisticsManager *statisticsManager,
               Synchronizer *synchronizer) :
        bufferSizeManager(bufferSizeManager),
        statisticsManager(statisticsManager),
        synchronizer(synchronizer) {
    streamId = streamId;
}

auto KSlack::getId() -> int {
    return streamId;
}

//K-Slack算法对无序流进行处理
auto KSlack::disorderHandling(const TrackTuplePtr &tuple) -> void {
    if (tuple->isEnd) {
        //将buffer区剩下的元素加入output
        while (!buffer.empty()) {
            auto syn_tuple = buffer.top();
            //加入同步器
            synchronizer->synchronizeStream(syn_tuple);
            buffer.pop();
        }
        return;
    }

    //update local time
    currentTime = std::max(currentTime, (int) tuple->eventTime);

    //每L个时间单位调整K值
    if (currentTime != 0 && currentTime % L == 0) {
        bufferSize = bufferSizeManager->kSearch(streamId);
    }

    //计算出tuple的delay,T - ts, 方便统计管理器统计记录
    tuple->delay = currentTime - tuple->eventTime;

    //将output_加入同步器
    //加入statistics_manager的历史记录统计表以及T值
    statisticsManager->addRecord(streamId, tuple);
    statisticsManager->addRecord(streamId, currentTime, bufferSize);

    //先让缓冲区所有满足条件的tuple出队进入输出区
    while (!buffer.empty()) {

        auto tuple = buffer.top();

        //对应论文的公式：ei. ts + Ki <= T
        if (tuple->eventTime + bufferSize > currentTime) {
            break;
        }

        //满足上述公式，加入输出区

        //加入同步器
        synchronizer->synchronizeStream(tuple);


        buffer.pop();
    }

    //加入tuple进入buffer
    buffer.push(tuple);


}

auto KSlack::setConfig(const INTELLI::ConfigMapPtr config) -> void {
    cfg = std::move(config);
    g = cfg->getU64("g");
    L = cfg->getU64("L");
    P = cfg->getU64("P");
    b = cfg->getU64("b");
    maxDelay = cfg->getU64("maxDelay");
    streamCount = cfg->getU64("StreamCount");
    userRecall = cfg->getDouble("userRecall");
    confidenceValue = cfg->getDouble("confidenceValue");
}


