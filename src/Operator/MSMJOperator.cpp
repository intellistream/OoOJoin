#include <Operator/MSMJOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>

static void *task(void *p) {
    reinterpret_cast<MSMJ::KSlack *>(p)->disorder_handling();
    return nullptr;
}


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
    if (!ts->isEnd) {
        MSMJ::Tuple tuple(1, -1, ts->eventTime);
        tuple.key = ts->key;
        tuple.payload = ts->payload;
        tuple.arrivalTime = ts->arrivalTime;
        sTupleList.push(tuple);
    } else {
        StreamPtr stream = std::make_shared<MSMJ::Stream>(1, 10, sTupleList);
        KSlackPtr kslackS = std::make_shared<MSMJ::KSlack>(stream.get(), bufferSizeManager.get(),
                                                           statisticsManager.get(), synchronizer.get());
        kslackS->setConfig(config);

        pthread_t t1 = 1;
        pthread_create(&t1, nullptr, &task, kslackS.get());
        pthread_join(t1, nullptr);
        lastTimeOfR = UtilityFunctions::timeLastUs(timeBaseStruct);
    }
    return true;
}

bool OoOJoin::MSMJOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
    if (!tr->isEnd) {
        MSMJ::Tuple tuple(2, -1, tr->eventTime);
        tuple.key = tr->key;
        tuple.payload = tr->payload;
        tuple.arrivalTime = tr->arrivalTime;
        rTupleList.push(tuple);
        rTupleRecord.push_back(tr);
    } else {
        StreamPtr stream = std::make_shared<MSMJ::Stream>(2, 10, rTupleList);

        KSlackPtr kslackR = std::make_shared<MSMJ::KSlack>(stream.get(), bufferSizeManager.get(),
                                                           statisticsManager.get(), synchronizer.get());
        kslackR->setConfig(config);

        pthread_t t2 = 2;
        pthread_create(&t2, NULL, &task, kslackR.get());
        pthread_join(t2, NULL);

        lastTimeOfR = UtilityFunctions::timeLastUs(timeBaseStruct);

        for (auto it: rTupleRecord) {
            it->processedTime = lastTimeOfR;
        }
    }
    return true;
}


size_t OoOJoin::MSMJOperator::getResult() {

    return streamOperator->getResult();
    // return confirmedResult;
}

size_t OoOJoin::MSMJOperator::getAQPResult() {
    return streamOperator->getAQPResult();
}




