
#include <Operator/IMAIAWJOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>

bool OoOJoin::IMAIAWJOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
    if (!OoOJoin::AbstractOperator::setConfig(cfg)) {
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

bool OoOJoin::IMAIAWJOperator::start() {
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
    timeBreakDown_prediction = 0;
    timeBreakDown_index = 0;
    timeBreakDown_join = 0;
    timeBreakDown_all = 0;timeTrackingStartNoClaim(timeBreakDown_all);
    return true;
}

void OoOJoin::IMAIAWJOperator::conductComputation() {

}

bool OoOJoin::IMAIAWJOperator::stop() {
    /**
     */
    if (lockedByWaterMark) {
        WM_INFO("early terminate by watermark, already have results");
    }
    if (!lockedByWaterMark) {
        WM_INFO("No watermark encountered, compute now");
    }
    timeBreakDown_all = timeTrackingEnd(timeBreakDown_all);
    //lazyComputeOfAQP();
    size_t rLen = myWindow.windowR.size();
    NPJTuplePtr *tr = myWindow.windowR.data();
    tsType timeNow = lastTimeOfR;
    for (size_t i = 0; i < rLen; i++) {
        if (tr[i]->arrivalTime < timeNow) { tr[i]->processedTime = timeNow; }
    }
    return true;
}

bool OoOJoin::IMAIAWJOperator::feedTupleS(OoOJoin::TrackTuplePtr ts) {
    //tsType startPredictionTime,startIndexingTime,startJoinTime;
    bool shouldGenWM, isInWindow;
    if (lockedByWaterMark) {
        return false;
    }
    isInWindow = myWindow.feedTupleS(ts);
    shouldGenWM = wmGen->reportTupleS(ts, 1);
    if (shouldGenWM) {
        lockedByWaterMark = true;
        WM_INFO("water mark in S");
        //  return false;
    }
    // bool shouldGenWM;
    if (isInWindow) {
        IMAStateOfKeyPtr sk;
        /**
         * @brief First get the index of hash table
         */
        timeTrackingStart(tt_index);
        AbstractStateOfKeyPtr skrf = stateOfKeyTableS->getByKey(ts->key);
        if (skrf == nullptr) // this key does'nt exist
        {
            sk = newIMAStateOfKey();
            sk->key = ts->key;
            stateOfKeyTableS->insert(sk);
        } else {
            sk = ImproveStateOfKeyTo(IMAStateOfKey, skrf);
        }
        timeBreakDown_index += timeTrackingEnd(tt_index);
        /**
         *
         */
        timeTrackingStart(tt_prediction);
        updateStateOfKey(sk, ts);
        double futureTuplesS = MeanAQPIAWJOperator::predictUnarrivedTuples(sk);
        timeBreakDown_prediction += timeTrackingEnd(tt_prediction);
        //probe in R
        timeTrackingStart(tt_join);
        AbstractStateOfKeyPtr probrPtr = stateOfKeyTableR->getByKey(ts->key);
        if (probrPtr != nullptr) {
            IMAStateOfKeyPtr py = ImproveStateOfKeyTo(IMAStateOfKey, probrPtr);
            confirmedResult += py->arrivedTupleCnt;
            intermediateResult -=
                    (sk->arrivedTupleCnt + sk->lastUnarrivedTuples - 1) *
                    (py->lastUnarrivedTuples + py->arrivedTupleCnt);
            intermediateResult +=
                    (futureTuplesS + sk->arrivedTupleCnt) * (py->lastUnarrivedTuples + py->arrivedTupleCnt);
        }
        timeBreakDown_join += timeTrackingEnd(tt_join);
        //sk->lastEstimateAllTuples=futureTuplesS+sk->arrivedTupleCnt;
        sk->lastUnarrivedTuples = futureTuplesS;
        lastTimeOfR = UtilityFunctions::timeLastUs(timeBaseStruct);
    }
    return true;
}

bool OoOJoin::IMAIAWJOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
    bool shouldGenWM, isInWindow;
    if (lockedByWaterMark) {
        return false;
    }
    isInWindow = myWindow.feedTupleR(tr);
    shouldGenWM = wmGen->reportTupleR(tr, 1);
    if (shouldGenWM) {
        lockedByWaterMark = true;
        WM_INFO("water mark in R");
        //return false;
    }
    // bool shouldGenWM;
    if (isInWindow) {

        IMAStateOfKeyPtr sk;timeTrackingStart(tt_index);
        AbstractStateOfKeyPtr skrf = stateOfKeyTableR->getByKey(tr->key);

        // lastTimeR=tr->arrivalTime;
        if (skrf == nullptr) // this key does'nt exist
        {
            sk = newIMAStateOfKey();
            sk->key = tr->key;
            stateOfKeyTableR->insert(sk);
        } else {
            sk = ImproveStateOfKeyTo(IMAStateOfKey, skrf);
        }
        timeBreakDown_index += timeTrackingEnd(tt_index);timeTrackingStart(tt_prediction);
        updateStateOfKey(sk, tr);
        double futureTuplesR = MeanAQPIAWJOperator::predictUnarrivedTuples(sk);
        timeBreakDown_prediction += timeTrackingEnd(tt_prediction);
        //probe in S
        timeTrackingStart(tt_join);
        AbstractStateOfKeyPtr probrPtr = stateOfKeyTableS->getByKey(tr->key);
        if (probrPtr != nullptr) {
            IMAStateOfKeyPtr py = ImproveStateOfKeyTo(IMAStateOfKey, probrPtr);
            confirmedResult += py->arrivedTupleCnt;
            intermediateResult -=
                    (sk->arrivedTupleCnt + sk->lastUnarrivedTuples - 1) *
                    (py->lastUnarrivedTuples + py->arrivedTupleCnt);
            intermediateResult +=
                    (futureTuplesR + sk->arrivedTupleCnt) * (py->lastUnarrivedTuples + py->arrivedTupleCnt);
        }
        timeBreakDown_join += timeTrackingEnd(tt_join);
        sk->lastUnarrivedTuples = futureTuplesR;
        lastTimeOfR = UtilityFunctions::timeLastUs(timeBaseStruct);
    }
    return true;
}

size_t OoOJoin::IMAIAWJOperator::getResult() {

    return confirmedResult;
    // return confirmedResult;
}

size_t OoOJoin::IMAIAWJOperator::getAQPResult() {
    return intermediateResult;
}