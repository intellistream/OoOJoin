//
// Created by tony on 22/11/22.
//

#include <Operator/AbstractOperator.h>
using namespace AllianceDB;
bool AllianceDB::AbstractOperator::feedTupleS(AllianceDB::TrackTuplePtr ts) {
  assert(ts);
  return true;
}

bool AllianceDB::AbstractOperator::feedTupleR(AllianceDB::TrackTuplePtr tr) {
  assert(tr);
  return true;
}
bool AllianceDB::AbstractOperator::start() {
  return true;
}
bool AllianceDB::AbstractOperator::stop() {
  return true;
}
size_t AllianceDB::AbstractOperator::getResult(){
  return 0;
}

