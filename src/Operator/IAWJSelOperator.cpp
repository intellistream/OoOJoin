
#include <Operator/IAWJSelOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>

bool OoOJoin::IAWJSelOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
    if (!OoOJoin::MeanAQPIAWJOperator::setConfig(cfg)) {
        return false;
    }
    std::string wmTag = config->tryString("wmTag", "arrival", true);
    WMTablePtr wmTable = newWMTable();
    wmGen = wmTable->findWM(wmTag);
    if (wmGen == nullptr) {
        INTELLI_ERROR("NO such a watermarker named [" + wmTag + "]");
        return false;
    }
    INTELLI_INFO("Using the watermarker named [" + wmTag + "]");
    return true;
}

bool OoOJoin::IAWJSelOperator::start() {
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
    // wmGen->creatWindow(0, windowLen);
    /**
     * @brief set window
     */
    stateOfKeyTableR = newStateOfKeyHashTable(4096, 4);
    stateOfKeyTableS = newStateOfKeyHashTable(4096, 4);
    myWindow.setRange(0, windowLen);
    windowBound = windowLen;
    myWindow.init(sLen, rLen, 1);

    intermediateResult = 0;
    confirmedResult = 0;
    lockedByWaterMark = false;
    timeBreakDownPrediction = 0;
    timeBreakDownIndex = 0;
    timeBreakDownJoin = 0;
    timeBreakDownAll = 0;timeTrackingStartNoClaim(timeBreakDownAll);
    return true;
}

void OoOJoin::IAWJSelOperator::conductComputation() {

}

bool OoOJoin::IAWJSelOperator::stop() {
    if (lockedByWaterMark) {
        WM_INFO("early terminate by watermark, already have results");
    }
    if (!lockedByWaterMark) {
        WM_INFO("No watermark encountered, compute now");
    }
    timeBreakDownAll = timeTrackingEnd(timeBreakDownAll);

    size_t rLen = myWindow.windowR.size();
    NPJTuplePtr *tr = myWindow.windowR.data();
    tsType timeNow = lastTimeOfR;
    for (size_t i = 0; i < rLen; i++) {
        if (tr[i]->arrivalTime < timeNow) {
            tr[i]->processedTime = timeNow;
        }
    }
    return true;
}

bool OoOJoin::IAWJSelOperator::feedTupleS(OoOJoin::TrackTuplePtr ts) {
    bool shouldGenWM, isInWindow;
    if (lockedByWaterMark) {
        return false;
    }
    isInWindow = myWindow.feedTupleS(ts);
    shouldGenWM = wmGen->reportTupleS(ts, 1);
    if (shouldGenWM) {
        lockedByWaterMark = true;
        WM_INFO("water mark in S");
    }
    if (isInWindow) {
        IMAStateOfKeyPtr stateOfKey;
        /**
         * @brief First get the index of hash table
         */
        timeTrackingStart(tt_index);
        AbstractStateOfKeyPtr stateOfSKey = stateOfKeyTableS->getByKey(ts->key);
        if (stateOfSKey == nullptr) // this key doesn't exist
        {
            stateOfKey = newIMAStateOfKey();
            stateOfKey->key = ts->key;
            stateOfKeyTableS->insert(stateOfKey);
        } else {
            stateOfKey = ImproveStateOfKeyTo(IMAStateOfKey, stateOfSKey);
        }
        timeBreakDownIndex += timeTrackingEnd(tt_index);
        /**
         *
         */
        timeTrackingStart(tt_prediction);
        updateStateOfKey(stateOfKey, ts);
        double futureTuplesS = MeanAQPIAWJOperator::predictUnarrivedTuples(stateOfKey);
        timeBreakDownPrediction += timeTrackingEnd(tt_prediction);
        //probe in R
        timeTrackingStart(tt_join);
        AbstractStateOfKeyPtr probrPtr = stateOfKeyTableR->getByKey(ts->key);
        if (probrPtr != nullptr) {
            IMAStateOfKeyPtr py = ImproveStateOfKeyTo(IMAStateOfKey, probrPtr);
            confirmedResult += py->arrivedTupleCnt;
            intermediateResult += selectivityS * (myWindow.windowS.size() * myWindow.windowR.size());
            selectivityS = confirmedResult / (myWindow.windowS.size() * myWindow.windowR.size());
        }
        timeBreakDownJoin += timeTrackingEnd(tt_join);
        stateOfKey->lastUnarrivedTuples = futureTuplesS;
        lastTimeOfR = UtilityFunctions::timeLastUs(timeBaseStruct);
    }
    return true;
}

bool OoOJoin::IAWJSelOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
    bool shouldGenWM, isInWindow;
    if (lockedByWaterMark) {
        return false;
    }
    isInWindow = myWindow.feedTupleR(tr);
    shouldGenWM = wmGen->reportTupleR(tr, 1);
    if (shouldGenWM) {
        lockedByWaterMark = true;
        WM_INFO("water mark in R");
    }
    if (isInWindow) {

        IMAStateOfKeyPtr stateOfKey;timeTrackingStart(tt_index);
        AbstractStateOfKeyPtr stateOfRKey = stateOfKeyTableR->getByKey(tr->key);

        // lastTimeR=tr->arrivalTime;
        if (stateOfRKey == nullptr) // this key does'nt exist
        {
            stateOfKey = newIMAStateOfKey();
            stateOfKey->key = tr->key;
            stateOfKeyTableR->insert(stateOfKey);
        } else {
            stateOfKey = ImproveStateOfKeyTo(IMAStateOfKey, stateOfRKey);
        }
        timeBreakDownIndex += timeTrackingEnd(tt_index);timeTrackingStart(tt_prediction);
        updateStateOfKey(stateOfKey, tr);
        double futureTuplesR = MeanAQPIAWJOperator::predictUnarrivedTuples(stateOfKey);
        timeBreakDownPrediction += timeTrackingEnd(tt_prediction);
        //probe in S
        timeTrackingStart(tt_join);
        AbstractStateOfKeyPtr probrPtr = stateOfKeyTableS->getByKey(tr->key);
        if (probrPtr != nullptr) {
            IMAStateOfKeyPtr py = ImproveStateOfKeyTo(IMAStateOfKey, probrPtr);
            confirmedResult += py->arrivedTupleCnt;
            intermediateResult += selectivityR * (myWindow.windowS.size() * myWindow.windowR.size());
            selectivityR = confirmedResult / (myWindow.windowS.size() * myWindow.windowR.size());
        }
        timeBreakDownJoin += timeTrackingEnd(tt_join);
        stateOfKey->lastUnarrivedTuples = futureTuplesR;
        lastTimeOfR = UtilityFunctions::timeLastUs(timeBaseStruct);
    }
    return true;
}

size_t OoOJoin::IAWJSelOperator::getResult() {
    return confirmedResult;
}

size_t OoOJoin::IAWJSelOperator::getAQPResult() {
    return intermediateResult;
}