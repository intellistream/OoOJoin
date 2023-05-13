
#include <Operator/AIOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>
#include <filesystem>
void OoOJoin::AIOperator::readTensors() {
  streamStatisics.selectivityTensorX = torch::zeros({1, (long) selLen});
  streamStatisics.selectivityTensorX = streamStatisics.selectivityTensorX.reshape({1, -1});
  streamStatisics.selectivityTensorY = tryTensor("torchscripts/" + ptPrefix + "/" + "tensor_selectivity_y.pt");
  streamStatisics.selectivityTensorY = streamStatisics.selectivityTensorY.reshape({-1, 1});
}
torch::Tensor OoOJoin::AIOperator::tryTensor(std::string fileName) {
  if (std::filesystem::exists(fileName)) {
    try {
      torch::Tensor load_tensor;
      // Load the tensor from the file
      torch::load(load_tensor, fileName);
      // If we get here, the file was loaded successfully
      return load_tensor;

    } catch (const std::runtime_error &error) {
      // Handle the error
      //  std::cerr << "Error loading tensor: " << error.what() << std::endl;
      INTELLI_ERROR("the tensor can not be loaded");
      // Return an error code
      return torch::empty({1, 0});;
    }
  }
  INTELLI_WARNING("the tensor DOES NOT exist");
  return torch::empty({1, 0});;
}
torch::Tensor OoOJoin::AIOperator::appendFloat2Tensor(torch::Tensor a, float b) {
  //float b = 7.0;
  if (streamStatisics.selectivityObservations < selLen) {
    a[0][streamStatisics.selectivityObservations] = b;
    streamStatisics.selectivityObservations++;
  }

  return a;
}
torch::Tensor OoOJoin::AIOperator::reshapeColedTensor(torch::Tensor a, uint64_t elementsInACol) {
  uint64_t rows = a.size(0);
  uint64_t cols = a.size(1);
  //auto b=a.reshape({1,(long)(rows*cols)});
  uint64_t elementsSelected = rows * cols / elementsInACol;
  elementsSelected = elementsSelected * elementsInACol;
  auto b = a.narrow(1, 0, elementsSelected);
  b = b.reshape({(long) (elementsSelected / elementsInACol), (long) elementsInACol});
  return b;
}
void OoOJoin::AIOperator::appendSelectivityTensorX() {
  if (aiModeEnum == 0 && appendTensor) {
    appendFloat2Tensor(streamStatisics.selectivityTensorX, streamStatisics.selectivity);

  }
}
void OoOJoin::AIOperator::saveAllTensors() {
  if (aiModeEnum == 0 && appendTensor) {
    /**
     * @brief 1. save the selectivity-x tensor
     */
    //std::cout<<streamStatisics.selectivityTensorX<<std::endl;
    auto oldSelectivityTensorX = tryTensor("torchscripts/" + ptPrefix + "/" + "tensor_selectivity_x.pt");
    auto validSelTensorX = streamStatisics.selectivityTensorX.narrow(1, 0, streamStatisics.selectivityObservations);
    streamStatisics.selectivityTensorX =
        reshapeColedTensor(validSelTensorX, streamStatisics.vaeSelectivity.getXDimension());
    if (oldSelectivityTensorX.size(1) != 0) {
      streamStatisics.selectivityTensorX =
          torch::cat({oldSelectivityTensorX, streamStatisics.selectivityTensorX}, /*dim=*/0);
    }
    //oldSelectivityTensorX= reshapeColedTensor(oldSelectivityTensorX,streamStatisics.vaeSelectivity.getXDimension());


    uint64_t selectivityRows = streamStatisics.selectivityTensorX.size(0);
    uint64_t selectivityCols = streamStatisics.selectivityTensorX.size(1);
    INTELLI_INFO(
        "Now we have [" + to_string(selectivityRows) + "x" + to_string(selectivityCols) + "] selectivity tensor");
    torch::save({streamStatisics.selectivityTensorX}, "torchscripts/" + ptPrefix + "/" + "tensor_selectivity_x.pt");
    /**
    * @brief 2. save the selectivity-y tensor
    */
    auto newSelectivityY = torch::ones({(long) (streamStatisics.selectivityObservations / selectivityCols), 1})
        * streamStatisics.selectivity;
    streamStatisics.selectivityTensorY = torch::cat({streamStatisics.selectivityTensorY, newSelectivityY}, /*dim=*/0);
    uint64_t yRows = streamStatisics.selectivityTensorY.size(0);
    uint64_t yCols = streamStatisics.selectivityTensorY.size(1);
    INTELLI_INFO("Now we have [" + to_string(yRows) + "x" + to_string(yCols) + "] selectivity-label tensor");
    torch::save({streamStatisics.selectivityTensorY}, "torchscripts/" + ptPrefix + "/" + "tensor_selectivity_y.pt");
    // std::cout<<streamStatisics.selectivityTensorY<<std::endl;
  }
}
bool OoOJoin::AIOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
  if (!OoOJoin::MeanAQPIAWJOperator::setConfig(cfg)) {
    return false;
  }
  std::string wmTag = config->tryString("wmTag", "arrival", true);
  aiMode = config->tryString("aiMode", "pretrain", true);
  ptPrefix = config->tryString("ptPrefix", "linearVAE", true);
  appendTensor = config->tryU64("appendTensor", 0, true);
  if (appendTensor) {
    INTELLI_WARNING("The tensors in file system will be overwrite by me because you asked me to do so.");
    selLen = config->tryU64("selLen", 0, true);
    if (selLen == 0) {
      INTELLI_ERROR("Invalid pretrain settings, abort");
      exit(0);
    }
  }
  streamStatisics.vaeSelectivity.loadModule("torchscripts/" + ptPrefix + "/" + ptPrefix + "_selectivity.pt");
  readTensors();
  // INTELLI_WARNING("The dimension of DAN is "+to_string(streamStatisics.vaeSelectivity.getXDimension()));
  if (aiMode == "pretrain") {
    aiModeEnum = 0;
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
      appendSelectivityTensorX();
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