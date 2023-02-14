#include <Operator/MSMJOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>

static void *task(void *p) {
    reinterpret_cast<MSMJ::KSlack *>(p)->disorder_handling();
    return nullptr;
}

OoOJoin::MSMJOperator::MSMJOperator() {
}


bool OoOJoin::MSMJOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
    if (!OoOJoin::AbstractOperator::setConfig(cfg)) {
        return false;
    }
    // read the algorithm name
    if (config->existString("algo")) {
        algoTag = config->getString("algo");
    }
    // OP_INFO("selected join algorithm=" + algoTag);
    if (config->existU64("threads")) {
        joinThreads = config->getU64("threads");
    }
    std::string wmTag = config->tryString("wmTag", "arrival", true);
    WMTablePtr wmTable = newWMTable();
    wmGen = wmTable->findWM(wmTag);
    if (wmGen == nullptr) {
        INTELLI_ERROR("NO such a watermarker named [" + wmTag + "]");
        return false;
    }
    INTELLI_INFO("Using the watermarker named [" + wmTag + "]");
    // OP_INFO("selected join threads=" + to_string(joinThreads));
    return true;
}

bool OoOJoin::MSMJOperator::start() {
    /**
     * @brief set watermark generator
     */
    //wmGen = newPeriodicalWM();
    wmGen->setConfig(config);
    wmGen->syncTimeStruct(timeBaseStruct);
    /**
     * @note:
    */
    wmGen->creatWindow(0, windowLen);
    /**
     * @brief set window
     */
    myWindow.setRange(0, windowLen);
    myWindow.init(sLen, rLen, 1);
    intermediateResult = 0;
    lockedByWaterMark = false;
    return true;
}

//Using this function to join
void OoOJoin::MSMJOperator::conductComputation() {
    JoinAlgoTable jt;
    AbstractJoinAlgoPtr algo = jt.findAlgo(algoTag);
    //NestedLoopJoin nj;
    algo->setConfig(config);
    algo->syncTimeStruct(timeBaseStruct);
    // OP_INFO("Invoke algorithm=" + algo->getAlgoName());
    intermediateResult = algo->join(myWindow.windowS, myWindow.windowR, joinThreads);
}

bool OoOJoin::MSMJOperator::stop() {
    /**
     */
    if (lockedByWaterMark) {
        WM_INFO("early terminate by watermark, already have results");
    }
    if (!lockedByWaterMark) {
        WM_INFO("No watermark encountered, compute now");
        //force to flush, if no watermark is given
        conductComputation();
    }
/*  size_t rLen = myWindow.windowR.size();
  NPJTuplePtr *tr = myWindow.windowR.data();
  tsType timeNow= UtilityFunctions::timeLastUs(timeBaseStruct);
  for (size_t i = 0; i < rLen; i++) {
    if (tr[i]->arrivalTime < timeNow)
    {tr[i]->processedTime = timeNow;}
  }*/
    return true;
}

bool OoOJoin::MSMJOperator::feedTupleS(OoOJoin::TrackTuplePtr ts) {
    if (!ts->isEnd) {
        MSMJ::Tuple tuple(1, -1, ts->eventTime);
        sTupleList.push(tuple);
    } else {
        MSMJ::Stream *stream = new MSMJ::Stream(1, windowLen, sTupleList);
        MSMJ::KSlack *kslackS = new MSMJ::KSlack(stream, bufferSizeManager,
                                                 statisticsManager, synchronizer);
//        kslackS->setConfig(config);

        pthread_t t1 = 1;
        pthread_create(&t1, NULL, &task, kslackS);
        pthread_join(t1, NULL);
    }
    return true;
}

bool OoOJoin::MSMJOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
    if (!tr->isEnd) {
        MSMJ::Tuple tuple(2, -1, tr->eventTime);
        rTupleList.push(tuple);
    } else {
        MSMJ::Stream *stream = new MSMJ::Stream(2, windowLen, rTupleList);
        MSMJ::KSlack *kslackR = new MSMJ::KSlack(stream, bufferSizeManager,
                                                 statisticsManager, synchronizer);
//        kslackS->setConfig(config);

        pthread_t t2 = 2;
        pthread_create(&t2, NULL, &task, kslackR);
        pthread_join(t2, NULL);
    }
    return true;
}

size_t OoOJoin::MSMJOperator::getResult() {
//    return streamOperator->getJoinResultCount();
    return 1;
}

void OoOJoin::MSMJOperator::init(ConfigMapPtr config) {
    config = nullptr;

    tupleProductivityProfiler = new MSMJ::TupleProductivityProfiler();
    statisticsManager = new MSMJ::StatisticsManager(tupleProductivityProfiler);
    bufferSizeManager = new MSMJ::BufferSizeManager(statisticsManager,
                                                    tupleProductivityProfiler);
    streamOperator = new MSMJ::StreamOperator(tupleProductivityProfiler);
    synchronizer = new MSMJ::Synchronizer(2, streamOperator);

//    tupleProductivityProfiler->setConfig(config);
//    statisticsManager->setConfig(config);
//    bufferSizeManager->setConfig(config);
//    synchronizer->setConfig(config);
}


