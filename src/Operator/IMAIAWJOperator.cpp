
#include <Operator/IMAIAWJOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>
bool OoOJoin::IMAIAWJOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
  if (!OoOJoin::AbstractOperator::setConfig(cfg)) {
    return false;
  }
  return true;
}
bool OoOJoin::IMAIAWJOperator::start() {
  /**
  * @brief set watermark generator
  */
  wmGen = newPeriodicalWM();
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
  bool shouldGenWM, isInWindow;
  if (lockedByWaterMark) {
    return false;
  }
  isInWindow = myWindow.feedTupleS(ts);
  shouldGenWM = wmGen->reportTupleS(ts, 1);
  if (shouldGenWM) {
    lockedByWaterMark = true;
    //  return false;

  }
  // bool shouldGenWM;
  if (isInWindow) {
    IMAStateOfKeyPtr sk;
    AbstractStateOfKeyPtr skrf = stateOfKeyTableS->getByKey(ts->key);
    //lastTimeS=ts->arrivalTime;
    if (skrf == nullptr) // this key does'nt exist
    {
      sk = newIMAStateOfKey();
      sk->key = ts->key;
      stateOfKeyTableS->insert(sk);
    } else {
      sk = ImproveStateOfKeyTo(IMAStateOfKey, skrf);
    }
    updateStateOfKey(sk, ts);
    //probe in R
    double futureTuplesS = MeanAQPIAWJOperator::predictUnarrivedTuples(sk);
    AbstractStateOfKeyPtr probrPtr = stateOfKeyTableR->getByKey(ts->key);
    if (probrPtr != nullptr) {
      IMAStateOfKeyPtr py = ImproveStateOfKeyTo(IMAStateOfKey, probrPtr);
      confirmedResult += py->arrivedTupleCnt;
      intermediateResult -=
          (sk->arrivedTupleCnt + sk->lastUnarrivedTuples - 1) * (py->lastUnarrivedTuples + py->arrivedTupleCnt);
      intermediateResult += (futureTuplesS + sk->arrivedTupleCnt) * (py->lastUnarrivedTuples + py->arrivedTupleCnt);
    }
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
    //return false;
  }
  // bool shouldGenWM;
  if (isInWindow) {

    IMAStateOfKeyPtr sk;
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
    updateStateOfKey(sk, tr);
    //size_t futureTuplesR=predictUnarrivedTuples(sk);
    //probe in S
    AbstractStateOfKeyPtr probrPtr = stateOfKeyTableS->getByKey(tr->key);
    double futureTuplesR = MeanAQPIAWJOperator::predictUnarrivedTuples(sk);
    if (probrPtr != nullptr) {
      IMAStateOfKeyPtr py = ImproveStateOfKeyTo(IMAStateOfKey, probrPtr);
      confirmedResult += py->arrivedTupleCnt;
      intermediateResult -=
          (sk->arrivedTupleCnt + sk->lastUnarrivedTuples - 1) * (py->lastUnarrivedTuples + py->arrivedTupleCnt);
      intermediateResult += (futureTuplesR + sk->arrivedTupleCnt) * (py->lastUnarrivedTuples + py->arrivedTupleCnt);
    }
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

