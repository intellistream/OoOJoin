#include <Operator/MSMJOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>


OoOJoin::MSMJOperator::MSMJOperator() {
    tupleProductivityProfiler = std::make_unique<TupleProductivityProfiler>();
    statisticsManager = std::make_unique<StatisticsManager>(tupleProductivityProfiler);
    bufferSizeManager = std::make_unique<BufferSizeManager>(statisticsManager, tupleProductivityProfiler);
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
        KSlackPtr kslackS = std::make_shared<KSlack>(streamS, bufferSizeManager, statisticsManager, synchronizer);
        kslackS->disorder_handling();
    }
    return true;
}

bool OoOJoin::MSMJOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
    if (!tr->isEnd) {
        streamR->push_tuple(*tr);
    } else {
        streamR->set_id(2);
        KSlackPtr kslackR = std::make_shared<KSlack>(streamR, bufferSizeManager, statisticsManager, synchronizer);
        kslackR->disorder_handling();
    }
    return true;
}

size_t OoOJoin::MSMJOperator::getResult() {
    return intermediateResult;
}


