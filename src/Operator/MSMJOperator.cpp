#include <Operator/MSMJOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>

static void *task(void *p) {
    reinterpret_cast<KSlack *>(p)->disorder_handling();
    return nullptr;
}

OoOJoin::MSMJOperator::MSMJOperator() {
    streamR = std::make_shared<Stream>((uint64_t) 1, (uint64_t) 2);
    streamS = std::make_shared<Stream>((uint64_t) 2, (uint64_t) 2);

    stream_map[(uint64_t) 1] = streamR.get();
    stream_map[(uint64_t) 2] = streamS.get();

    tupleProductivityProfiler = std::make_shared<TupleProductivityProfiler>();
    statisticsManager = std::make_shared<StatisticsManager>(tupleProductivityProfiler, stream_map);
    bufferSizeManager = std::make_shared<BufferSizeManager>(statisticsManager, tupleProductivityProfiler, stream_map);
    streamOperator = std::make_shared<StreamOperator>(tupleProductivityProfiler);
    synchronizer = std::make_shared<Synchronizer>(2, streamOperator);

    tupleProductivityProfiler->setConfig(config);
    statisticsManager->setConfig(config);
    bufferSizeManager->setConfig(config);
    synchronizer->setConfig(config);
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
        streamS->push_tuple(*ts);
    } else {
        streamS->set_id(1);
        KSlackPtr kslackS = std::make_shared<KSlack>(streamS, bufferSizeManager, statisticsManager, synchronizer,
                                                     stream_map);
        kslackS->setConfig(config);

        pthread_t t1 = 1;
        pthread_create(&t1, NULL, &task, kslackS.get());
        pthread_join(t1, NULL);
    }
    return true;
}

bool OoOJoin::MSMJOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
    if (!tr->isEnd) {
        streamR->push_tuple(*tr);
    } else {
        streamR->set_id(2);
        KSlackPtr kslackR = std::make_shared<KSlack>(streamR, bufferSizeManager, statisticsManager, synchronizer,
                                                     stream_map);
        kslackR->setConfig(config);


    }
    return true;
}

size_t OoOJoin::MSMJOperator::getResult() {
    return streamOperator->getJoinResultCount();
}


