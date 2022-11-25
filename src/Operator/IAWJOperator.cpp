//
// Created by tony on 22/11/22.
//

#include <Operator/IAWJOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>
bool AllianceDB::IAWJOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
  if (!AllianceDB::AbstractOperator::setConfig(cfg)) {
    return false;
  }
  // read the algorithm name
  if (config->existString("algo")) {
    algoTag = config->getString("algo");
  }
      OP_INFO("selected join algorithm=" + algoTag);
  if (config->existU64("threads")) {
    joinThreads = config->getU64("threads");
  }
      OP_INFO("selected join threads=" + to_string(joinThreads));
  return true;
}
bool AllianceDB::IAWJOperator::start() {
  /**
   * @brief set watermark generator
   */
  wmGen=newPeriodicalWM();
  wmGen->setConfig(config);
  wmGen->syncTimeStruct(timeBaseStruct);
  wmGen->creatWindow(0,windowLen);
  /**
   * @brief set window
   */
  myWindow.setRange(0, windowLen);
  myWindow.init(sLen, rLen, 1);
  intermediateResult = 0;
  lockedByWaterMark= false;
  return true;
}
void AllianceDB::IAWJOperator::conductComputation() {
  JoinAlgoTable jt;
  AbstractJoinAlgoPtr algo = jt.findAlgo(algoTag);
  //NestedLoopJoin nj;
  algo->setConfig(config);
  algo->syncTimeStruct(timeBaseStruct);
      OP_INFO("Invoke algorithm=" + algo->getAlgoName());
  intermediateResult = algo->join(myWindow.windowS, myWindow.windowR, joinThreads);
}
bool AllianceDB::IAWJOperator::stop() {
  /**
   */
   if(lockedByWaterMark)
   {
     WM_INFO("early terminate by watermark, already have results");
   }
   if(!lockedByWaterMark) {
     WM_INFO("No watermark encountered, compute now");
     //force to flush, if no watermark is given
     conductComputation();
   }
  return true;
}
bool AllianceDB::IAWJOperator::feedTupleS(AllianceDB::TrackTuplePtr ts) {
  bool shouldGenWM;
  if(lockedByWaterMark)
  {
    return false;
  }
  myWindow.feedTupleS(ts);
  shouldGenWM=wmGen->reportTupleS(ts,1);
  if(shouldGenWM)
  {
    lockedByWaterMark=true;
   // run computation
    conductComputation();
  }
  return true;
}

bool AllianceDB::IAWJOperator::feedTupleR(AllianceDB::TrackTuplePtr tr) {
  bool shouldGenWM;
  if(lockedByWaterMark)
  {
    return false;
  }
  myWindow.feedTupleR(tr);
  shouldGenWM=wmGen->reportTupleR(tr,1);
  if(shouldGenWM)
  {
    lockedByWaterMark=true;
    // run computation
    conductComputation();
  }
  return true;
}
size_t AllianceDB::IAWJOperator::getResult() {
  return intermediateResult;
}
