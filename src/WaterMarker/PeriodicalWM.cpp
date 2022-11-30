//
// Created by tony on 25/11/22.
//

#include <WaterMarker/PeriodicalWM.h>

bool AllianceDB::PeriodicalWM::setConfig(INTELLI::ConfigMapPtr cfg) {
  if (!AbstractWaterMarker::setConfig(cfg)) {
    return false;
  }
  if (config->existU64("watermarkPeriod")) {
    watermarkPeriod = config->getU64("watermarkPeriod");
        WM_INFO("watermarkPeriod=" + to_string(watermarkPeriod));
  } else {
        WM_WARNNING("Leaving watermarkPeriod as blank, will use windowLen+1 instead\n");
  }
  return true;
}

size_t AllianceDB::PeriodicalWM::creatWindow(AllianceDB::tsType tBegin, AllianceDB::tsType tEnd) {
  windowLen = tEnd - tBegin;
  if (watermarkPeriod == 0) {
    nextWMDelta = windowLen + 1;
  } else {
    nextWMDelta = watermarkPeriod;
  }
  nextWMPoint = nextWMDelta;
  return 1;
}
bool AllianceDB::PeriodicalWM::isReachWMPoint(AllianceDB::TrackTuplePtr tp) {
  //tsType tNow=UtilityFunctions::UtilityFunctions::timeLastUs(timeBaseStruct)/timeStep;
  if (tp->arrivalTime >= nextWMPoint) {
        WM_INFO("watermark reached at" + tp->toString());
    nextWMPoint += nextWMDelta;
    return true;
  }
  return false;
}
bool AllianceDB::PeriodicalWM::reportTupleS(AllianceDB::TrackTuplePtr ts, size_t wid) {
  if (!wid) {
    return false;
  }
  return isReachWMPoint(ts);
}

bool AllianceDB::PeriodicalWM::reportTupleR(AllianceDB::TrackTuplePtr tr, size_t wid) {
  if (!wid) {
    return false;
  }
  return isReachWMPoint(tr);
}