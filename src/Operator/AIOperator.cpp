
#include <Operator/AIOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>
#include <filesystem>

void OoOJoin::AIOperator::saveAllTensors() {
  if (aiModeEnum == 0) {
    /**
     * @brief 1. save the selectivity tensor
     */
    uint64_t xCols = streamStatisics.vaeSelectivity.getXDimension();
    /**
     * @brief 2. set all final observations
     */
    streamStatisics.selObservations.setFinalObservation(streamStatisics.selectivity);
    streamStatisics.sSkewObservations.setFinalObservation(streamStatisics.sSkew);
    streamStatisics.sRateObservations.setFinalObservation(streamStatisics.sRate);
    streamStatisics.rSkewObservations.setFinalObservation(streamStatisics.rSkew);
    streamStatisics.rRateObservations.setFinalObservation(streamStatisics.rRate);
    /**
     * @brief 3. save all tensors
     */
    if (appendSel) {
      streamStatisics.selObservations.saveXYTensors2Files("torchscripts/" + ptPrefix + "/" + "tensor_selectivity",
                                                          xCols);
    }
    if (appendSkew) {
      streamStatisics.sSkewObservations.saveXYTensors2Files("torchscripts/" + ptPrefix + "/" + "tensor_sSkew", xCols);
      streamStatisics.rSkewObservations.saveXYTensors2Files("torchscripts/" + ptPrefix + "/" + "tensor_rSkew", xCols);
    }
    if (appendRate) {
      streamStatisics.sRateObservations.saveXYTensors2Files("torchscripts/" + ptPrefix + "/" + "tensor_sRate", xCols);
      streamStatisics.rRateObservations.saveXYTensors2Files("torchscripts/" + ptPrefix + "/" + "tensor_rRate", xCols);
    }

    // std::cout<<streamStatisics.selectivityTensorY<<std::endl;
  }
}
void OoOJoin::AIOperator::preparePretrain() {
  /**
   * @brief 1. init all observation tensors
   */
  streamStatisics.selObservations.initObservationBuffer(selLen);
  streamStatisics.sSkewObservations.initObservationBuffer(selLen);
  streamStatisics.rSkewObservations.initObservationBuffer(selLen);
  streamStatisics.sRateObservations.initObservationBuffer(selLen);
  streamStatisics.rRateObservations.initObservationBuffer(selLen);
}
bool OoOJoin::AIOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
  if (!OoOJoin::MeanAQPIAWJOperator::setConfig(cfg)) {
    return false;
  }
  std::string wmTag = config->tryString("wmTag", "arrival", true);
  aiMode = config->tryString("aiMode", "pretrain", true);
  ptPrefix = config->tryString("ptPrefix", "linearVAE", true);
  appendSel = config->tryU64("appendSel", 0, true);
  appendSkew = config->tryU64("appendSkew", 0, true);
  appendRate = config->tryU64("appendRate", 0, true);
  if (appendSel || appendSkew || appendRate) {
    INTELLI_WARNING("The tensors in file system will be overwrite by me because you asked me to do so.");
    selLen = config->tryU64("selLen", 0, true);
    if (selLen == 0) {
      INTELLI_ERROR("Invalid pretrain settings, abort");
      exit(0);
    }
  }
  streamStatisics.vaeSelectivity.loadModule("torchscripts/" + ptPrefix + "/" + ptPrefix + "_selectivity.pt");
  // INTELLI_WARNING("The dimension of DAN is "+to_string(streamStatisics.vaeSelectivity.getXDimension()));
  if (aiMode == "pretrain") {
    aiModeEnum = 0;
    preparePretrain();
  } else if (aiMode == "continual_learning") {
    aiModeEnum = 1;
  } else if (aiMode == "inference") {
    aiModeEnum = 2;
  } else {
    aiModeEnum = 255;
    INTELLI_WARNING("This mode is N.A.");
  }

  WMTablePtr wmTable = newWMTable();
  wmGen = wmTable->findWM(wmTag);
  if (wmGen == nullptr) {
    INTELLI_ERROR("NO such a watermarker named [" + wmTag + "]");
    return false;
  }
  INTELLI_INFO("Using the watermarker named [" + wmTag + "]");

  return true;
}

bool OoOJoin::AIOperator::start() {
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
  streamStatisics.reset();

  return true;
}

void OoOJoin::AIOperator::conductComputation() {

}

bool OoOJoin::AIOperator::stop() {
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

bool OoOJoin::AIOperator::feedTupleS(OoOJoin::TrackTuplePtr ts) {
  bool shouldGenWM, isInWindow;
  if (lockedByWaterMark) {
    return false;
  }
  isInWindow = myWindow.feedTupleS(ts);
  shouldGenWM = wmGen->reportTupleS(ts, 1);
  if (shouldGenWM) {
    lockedByWaterMark = true;
    WM_INFO("water mark in S");
    endOfWindow();
  }
  if (isInWindow) {
    AIStateOfKeyPtr stateOfKey;
    /**
     * @brief update the stream statistics
     *
     */
    streamStatisics.encounterSTuple(ts);
    /**
     * @brief First get the index of hash table
     */
    timeTrackingStart(tt_index);
    AbstractStateOfKeyPtr stateOfSKey = stateOfKeyTableS->getByKey(ts->key);
    if (stateOfSKey == nullptr) // this key doesn't exist
    {
      stateOfKey = newAIStateOfKey();
      stateOfKey->key = ts->key;
      stateOfKeyTableS->insert(stateOfKey);
    } else {
      stateOfKey = ImproveStateOfKeyTo(AIStateOfKey, stateOfSKey);
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
      AIStateOfKeyPtr py = ImproveStateOfKeyTo(AIStateOfKey, probrPtr);
      confirmedResult += py->arrivedTupleCnt;
      /**
       * @brief update selectivity here
       */
      streamStatisics.updateSelectivity(confirmedResult);
      //streamStatisics.selObservations.appendX(streamStatisics.selectivity);
      if (aiModeEnum == 0) {
        streamStatisics.selObservations.setFinalObservation(streamStatisics.selectivity);
        streamStatisics.selObservations.appendX(streamStatisics.selectivity);
        streamStatisics.sSkewObservations.appendX(streamStatisics.sSkew);
        streamStatisics.sRateObservations.appendX(streamStatisics.sRate);
      }

//            intermediateResult += py->arrivedTupleCnt;
      intermediateResult += (futureTuplesS + stateOfKey->arrivedTupleCnt) *
          (py->lastUnarrivedTuples + py->arrivedTupleCnt) -
          (stateOfKey->arrivedTupleCnt + stateOfKey->lastUnarrivedTuples - 1) *
              (py->lastUnarrivedTuples + py->arrivedTupleCnt);
    }
    timeBreakDownJoin += timeTrackingEnd(tt_join);
    stateOfKey->lastUnarrivedTuples = futureTuplesS;
    lastTimeOfR = UtilityFunctions::timeLastUs(timeBaseStruct);
  }
  return true;
}
void OoOJoin::AIOperator::endOfWindow() {
  if (aiModeEnum == 0) {
    std::cout << streamStatisics.reportStr() << endl;
    std::cout << "joined " + to_string(confirmedResult) << endl;
  }
  saveAllTensors();
}
bool OoOJoin::AIOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
  bool shouldGenWM, isInWindow;
  if (lockedByWaterMark) {
    return false;
  }
  isInWindow = myWindow.feedTupleR(tr);
  shouldGenWM = wmGen->reportTupleR(tr, 1);
  if (shouldGenWM) {
    lockedByWaterMark = true;
    WM_INFO("water mark in R");
    endOfWindow();
  }
  if (isInWindow) {
    /**
   * @brief update the stream statistics
   *
   */
    streamStatisics.encounterRTuple(tr);

    AIStateOfKeyPtr stateOfKey;timeTrackingStart(tt_index);
    AbstractStateOfKeyPtr stateOfRKey = stateOfKeyTableR->getByKey(tr->key);

    // lastTimeR=tr->arrivalTime;
    if (stateOfRKey == nullptr) // this key does'nt exist
    {
      stateOfKey = newAIStateOfKey();
      stateOfKey->key = tr->key;
      stateOfKeyTableR->insert(stateOfKey);
    } else {
      stateOfKey = ImproveStateOfKeyTo(AIStateOfKey, stateOfRKey);
    }
    timeBreakDownIndex += timeTrackingEnd(tt_index);timeTrackingStart(tt_prediction);
    updateStateOfKey(stateOfKey, tr);
    double futureTuplesR = MeanAQPIAWJOperator::predictUnarrivedTuples(stateOfKey);
    timeBreakDownPrediction += timeTrackingEnd(tt_prediction);
    //probe in S
    timeTrackingStart(tt_join);
    AbstractStateOfKeyPtr probrPtr = stateOfKeyTableS->getByKey(tr->key);
    if (probrPtr != nullptr) {
      AIStateOfKeyPtr py = ImproveStateOfKeyTo(AIStateOfKey, probrPtr);
      confirmedResult += py->arrivedTupleCnt;
      /**
     * @brief update selectivity here
     */
      /**
        * @brief update selectivity here
        */
      streamStatisics.updateSelectivity(confirmedResult);
      //streamStatisics.selObservations.appendX(streamStatisics.selectivity);
      if (aiModeEnum == 0) {
        streamStatisics.selObservations.setFinalObservation(streamStatisics.selectivity);
        streamStatisics.selObservations.appendX(streamStatisics.selectivity);
        streamStatisics.rSkewObservations.appendX(streamStatisics.rSkew);
        streamStatisics.rRateObservations.appendX(streamStatisics.rRate);
      }
      //streamStatisics.updateSelectivity(confirmedResult);
      // appendSelectivityTensorX();
//            intermediateResult += py->arrivedTupleCnt;
      intermediateResult += (futureTuplesR + stateOfKey->arrivedTupleCnt) *
          (py->lastUnarrivedTuples + py->arrivedTupleCnt) -
          (stateOfKey->arrivedTupleCnt + stateOfKey->lastUnarrivedTuples - 1) *
              (py->lastUnarrivedTuples + py->arrivedTupleCnt);
    }
    timeBreakDownJoin += timeTrackingEnd(tt_join);
    stateOfKey->lastUnarrivedTuples = futureTuplesR;
    lastTimeOfR = UtilityFunctions::timeLastUs(timeBaseStruct);
  }
  return true;
}

size_t OoOJoin::AIOperator::getResult() {

  return confirmedResult;
}

size_t OoOJoin::AIOperator::getAQPResult() {
  return intermediateResult;
}