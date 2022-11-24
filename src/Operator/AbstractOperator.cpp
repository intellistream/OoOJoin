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
size_t AllianceDB::AbstractOperator::getResult() {
  return 0;
}
bool AbstractOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
  config = cfg;
  if (config == nullptr) {
    return false;
  }
  if (config->existU64("windowLen")) {
    windowLen = config->getU64("windowLen");
  }
  if (config->existU64("slideLen")) {
    slideLen = config->getU64("slideLen");
  }
  if (config->existU64("sLen")) {
    sLen = config->getU64("sLen");
  } else {

    //printf("sLen not set\r\n");
        OP_ERROR("empty buffer for S stream \n");

    return false;
  }
  if (config->existU64("rLen")) {
    rLen = config->getU64("rLen");
  } else {

        OP_ERROR("empty buffer for R stream \n");
    return false;
  }
  return true;
}

