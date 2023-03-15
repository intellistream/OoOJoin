#include <Operator/MSMJOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>



bool OoOJoin::MSMJOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
    if (!OoOJoin::MeanAQPIAWJOperator::setConfig(cfg)) {
        return false;
    }
    streamOperator->setConfig(cfg);

    return true;
}

bool OoOJoin::MSMJOperator::start() {
    /**
     * @brief set watermark generator
     */
    //wmGen = newPeriodicalWM();
    return streamOperator->start();
}


bool OoOJoin::MSMJOperator::stop() {
    /**
     */
    streamOperator->stop();

    return true;
}

bool OoOJoin::MSMJOperator::feedTupleS(OoOJoin::TrackTuplePtr ts) {
    struct timeval timeStart{};
    gettimeofday(&timeStart, nullptr);
    streamOperator->syncTimeStruct(timeStart);
    ts->streamId = 1;
    kSlackS->disorder_handling(ts);
    return true;
}

bool OoOJoin::MSMJOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
    struct timeval timeStart{};
    gettimeofday(&timeStart, nullptr);
    streamOperator->syncTimeStruct(timeStart);
    tr->streamId = 2;
    kSlackR->disorder_handling(tr);
    return true;
}


size_t OoOJoin::MSMJOperator::getResult() {

    return streamOperator->getResult();
}

size_t OoOJoin::MSMJOperator::getAQPResult() {
    return streamOperator->getAQPResult();
}




