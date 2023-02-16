#include <Operator/MSMJOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>

static void *task(void *p) {
    reinterpret_cast<MSMJ::KSlack *>(p)->disorder_handling();
    return nullptr;
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
    }
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
//        kslackS->setConfig(config);

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
//        kslackS->setConfig(config);

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
    return streamOperator->getJoinResultCount();
}

void OoOJoin::MSMJOperator::init(ConfigMapPtr config) {
    config->edit("a", "s");

    tupleProductivityProfiler = std::make_shared<MSMJ::TupleProductivityProfiler>();
    statisticsManager = std::make_shared<MSMJ::StatisticsManager>(tupleProductivityProfiler.get());
    bufferSizeManager = std::make_shared<MSMJ::BufferSizeManager>(statisticsManager.get(),
                                                                  tupleProductivityProfiler.get());
    streamOperator = std::make_shared<MSMJ::StreamOperator>(tupleProductivityProfiler.get());
    synchronizer = std::make_shared<MSMJ::Synchronizer>(2, streamOperator.get());

//    tupleProductivityProfiler->setConfig(config);
//    statisticsManager->setConfig(config);
//    bufferSizeManager->setConfig(config);
//    synchronizer->setConfig(config);
}


