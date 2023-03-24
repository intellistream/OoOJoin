#include <Operator/MSWJOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>


bool OoOJoin::MSWJOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
    if (!OoOJoin::MeanAQPIAWJOperator::setConfig(cfg)) {
        return false;
    }
    streamOperator->setConfig(cfg);

    return true;
}

bool OoOJoin::MSWJOperator::start() {
    /**
     * @brief set watermark generator
     */
    //wmGen = newPeriodicalWM();
    streamOperator->syncTimeStruct(timeBaseStruct);
    return streamOperator->start();
}


bool OoOJoin::MSWJOperator::stop() {
    /**
     */
    streamOperator->stop();
    timeBreakDown_all = timeTrackingEnd(timeBreakDown_all);
    return true;
}

bool OoOJoin::MSWJOperator::feedTupleS(OoOJoin::TrackTuplePtr ts) {
//    struct timeval timeStart{};
//    if (!initTime) {
//        gettimeofday(&timeStart, nullptr);
//        streamOperator->timeBase = timeStart;
//        initTime = true;
//    }

    ts->streamId = 1;
    kSlackS->disorderHandling(ts);
    return true;
}

bool OoOJoin::MSWJOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
//    struct timeval timeStart{};
////    if (initTime) {
////        gettimeofday(&timeStart, nullptr);
//        streamOperator->timeBase = timeStart;
//        initTime = true;
//    }

    tr->streamId = 2;
    kSlackR->disorderHandling(tr);
    return true;
}


size_t OoOJoin::MSWJOperator::getResult() {

    return streamOperator->getResult();
}

size_t OoOJoin::MSWJOperator::getAQPResult() {
    return streamOperator->getAQPResult();
}




