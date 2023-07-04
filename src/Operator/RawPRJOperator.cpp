//
// Created by tony on 22/11/22.
//

#include <Operator/RawPRJOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>

bool OoOJoin::RawPRJOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
  if (!OoOJoin::AbstractOperator::setConfig(cfg)) {
    return false;
  }
  algoTag = "NPJSingle";
  // OP_INFO("selected join algorithm=" + algoTag);

  joinThreads = 1;

  joinSum = config->tryU64("joinSum", 0, true);
  // OP_INFO("selected join threads=" + to_string(joinThreads));
  return true;
}

bool OoOJoin::RawPRJOperator::start() {

  /**
   * @brief set window
   */
  myWindow.setRange(0, windowLen);
  myWindow.init(sLen, rLen, 1);
  intermediateResult = 0;
  lockedByWaterMark = false;
  return true;
}

void OoOJoin::RawPRJOperator::conductComputation() {
  JoinAlgoTable jt;
  AbstractJoinAlgoPtr algo = jt.findAlgo(algoTag);
  //NestedLoopJoin nj;
  algo->setConfig(config);
  algo->syncTimeStruct(timeBaseStruct);
  // OP_INFO("Invoke algorithm=" + algo->getAlgoName());
  intermediateResult = algo->join(myWindow.windowS, myWindow.windowR, joinThreads);
}

bool OoOJoin::RawPRJOperator::stop() {
  if (lockedByWaterMark) {
    WM_INFO("early terminate by watermark, already have results");
  }
  if (!lockedByWaterMark) {
    WM_INFO("No watermark encountered, compute now");
    //force to flush, if no watermark is given
    conductComputation();
  }
  return true;
}

bool OoOJoin::RawPRJOperator::feedTupleS(OoOJoin::TrackTuplePtr ts) {
  bool shouldGenWM;
  if (lockedByWaterMark) {
    return false;
  }
  myWindow.feedTupleS(ts);
  /**
   * @brief once see the event reaches window end, terminate everything
   */
  if (ts->arrivalTime > myWindow.getEnd()) {
    shouldGenWM = true;
  }
  if (shouldGenWM) {
    lockedByWaterMark = true;
    // run computation
    conductComputation();
  }
  return true;
}

bool OoOJoin::RawPRJOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
  bool shouldGenWM;
  if (lockedByWaterMark) {
    return false;
  }
  myWindow.feedTupleR(tr);
  if (tr->arrivalTime > myWindow.getEnd()) {
    shouldGenWM = true;
  }
  if (shouldGenWM) {
    lockedByWaterMark = true;
    // run computation
    conductComputation();
  }
  return true;
}

size_t OoOJoin::RawPRJOperator::getResult() {
  return intermediateResult;
}
