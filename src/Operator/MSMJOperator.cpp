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
    MSMJ::Tuple tuple(1, -1, ts->eventTime);
    tuple.key = ts->key;
    tuple.payload = ts->payload;
    tuple.arrivalTime = ts->arrivalTime;
    tuple.end = ts->isEnd;
    kSlackS->disorder_handling(tuple);
    return true;
}

bool OoOJoin::MSMJOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
    MSMJ::Tuple tuple(2, -1, tr->eventTime);
    tuple.key = tr->key;
    tuple.payload = tr->payload;
    tuple.arrivalTime = tr->arrivalTime;
    tuple.end = tr->isEnd;
    kSlackR->disorder_handling(tuple);
    return true;
}


size_t OoOJoin::MSMJOperator::getResult() {

    return streamOperator->getResult();
    // return confirmedResult;
}

size_t OoOJoin::MSMJOperator::getAQPResult() {
    return streamOperator->getAQPResult();
}




