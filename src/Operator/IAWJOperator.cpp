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
  myWindow.setRange(0, windowLen);
  myWindow.init(sLen, rLen, 1);
  intermediateResult = 0;
  return true;
}
bool AllianceDB::IAWJOperator::stop() {
  /**
   */
  JoinAlgoTable jt;
  AbstractJoinAlgoPtr algo = jt.findAlgo(algoTag);
  //NestedLoopJoin nj;
  algo->syncTimeStruct(timeBaseStruct);
      OP_INFO("Invoke algorithm=" + algo->getAlgoName());
  intermediateResult = algo->join(myWindow.windowS, myWindow.windowR, joinThreads);
  return true;
}
bool AllianceDB::IAWJOperator::feedTupleS(AllianceDB::TrackTuplePtr ts) {
  myWindow.feedTupleS(ts);
  return true;
}

bool AllianceDB::IAWJOperator::feedTupleR(AllianceDB::TrackTuplePtr tr) {
  myWindow.feedTupleR(tr);
  return true;
}
size_t AllianceDB::IAWJOperator::getResult() {
  return intermediateResult;
}
