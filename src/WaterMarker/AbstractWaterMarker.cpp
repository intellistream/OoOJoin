//
// Created by tony on 24/11/22.
//

#include <WaterMarker/AbstractWaterMarker.h>
using namespace AllianceDB;
bool AllianceDB::AbstractWaterMarker::init() {
  return true;
}
bool AllianceDB::AbstractWaterMarker::setConfig(INTELLI::ConfigMapPtr cfg) {
  config = cfg;
  if (config == nullptr) {
    return false;
  }
  if (config->existDouble("errorBound")) {
    errorBound = config->getDouble("errorBound");
  } else {
        WM_WARNNING("NO assigned errorBound, set to 0.01");
    errorBound = 0.01;
  }
  /*if (config->existU64("timeStep")) {
    timeStep = config->getU64("timeStep");
  }
  else
  {
    WM_WARNNING("No setting of timeStep, use 1\n");
    timeStep=1;
  }*/
  return true;
}
size_t AllianceDB::AbstractWaterMarker::creatWindow(AllianceDB::tsType tBegin, AllianceDB::tsType tEnd) {
  assert(tEnd >= tBegin);
  return 1;
}
bool AllianceDB::AbstractWaterMarker::deleteWindow(size_t wid) {
  return (wid == 1);
}
bool AllianceDB::AbstractWaterMarker::reportTupleS(AllianceDB::TrackTuplePtr ts, size_t wid) {
  assert(ts);
  assert(wid);
  return false;
}

bool AllianceDB::AbstractWaterMarker::reportTupleR(AllianceDB::TrackTuplePtr tr, size_t wid) {
  assert(tr);
  assert(wid);
  return false;
}
double AllianceDB::AbstractWaterMarker::estimateError(size_t wid) {
  assert(wid);
  return 0;
}