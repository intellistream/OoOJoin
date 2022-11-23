//
// Created by tony on 22/11/22.
//

#include <Operator/IAWJOperator.h>
#include <JoinAlgos/NestedLoopJoin.h>
bool AllianceDB::IAWJOperator::start() {
  myWindow.setRange(0, windowLen);
  myWindow.init(sLen, rLen, 1);
  intermediateResult = 0;
  return true;
}
bool AllianceDB::IAWJOperator::stop() {
  /**
   */
  NestedLoopJoin nj;
  intermediateResult = nj.join(myWindow.windowS, myWindow.windowR, 1);
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
