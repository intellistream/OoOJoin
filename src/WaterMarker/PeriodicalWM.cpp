//
// Created by tony on 25/11/22.
//

#include <WaterMarker/PeriodicalWM.h>

bool OoOJoin::PeriodicalWM::setConfig(INTELLI::ConfigMapPtr cfg) {
  if (!AbstractWaterMarker::setConfig(cfg)) {
    return false;
  }
  if (config->existU64("watermarkPeriod")) {
    watermarkPeriod = config->getU64("watermarkPeriod");
        //WM_INFO("watermarkPeriod=" + to_string(watermarkPeriod));
  } else {
       // WM_WARNNING("Leaving watermarkPeriod as blank, will use windowLen+1 instead\n");
  }
  return true;
}

size_t OoOJoin::PeriodicalWM::creatWindow(OoOJoin::tsType tBegin, OoOJoin::tsType tEnd) {
  windowLen = tEnd - tBegin;
  if (watermarkPeriod == 0) {
    nextWMDelta = windowLen + 1;
  } else {
    nextWMDelta = watermarkPeriod;
  }
  nextWMPoint = nextWMDelta;
  return 1;
}
bool OoOJoin::PeriodicalWM::isReachWMPoint(OoOJoin::TrackTuplePtr tp) {
  //tsType tNow=UtilityFunctions::UtilityFunctions::timeLastUs(timeBaseStruct)/timeStep;
  if (tp->arrivalTime >= nextWMPoint) {
     //   WM_INFO("watermark reached at" + tp->toString());
    nextWMPoint += nextWMDelta;
    return true;
  }
  return false;
}
bool OoOJoin::PeriodicalWM::reportTupleS(OoOJoin::TrackTuplePtr ts, size_t wid) {
  if (!wid) {
    return false;
  }
  return isReachWMPoint(ts);
}

bool OoOJoin::PeriodicalWM::reportTupleR(OoOJoin::TrackTuplePtr tr, size_t wid) {
  if (!wid) {
    return false;
  }
  return isReachWMPoint(tr);
}