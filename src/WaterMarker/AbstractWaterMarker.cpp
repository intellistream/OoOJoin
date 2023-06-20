//
// Created by tony on 24/11/22.
//

#include <WaterMarker/AbstractWaterMarker.h>

using namespace OoOJoin;

bool OoOJoin::AbstractWaterMarker::init() {
    return true;
}

bool OoOJoin::AbstractWaterMarker::setConfig(INTELLI::ConfigMapPtr cfg) {
    config = cfg;
    if (config == nullptr) {
        return false;
    }
    /*if (config->existDouble("errorBound")) {
      errorBound = config->getDouble("errorBound");
    } else {
      WM_WARNNING("NO assigned errorBound, set to 0.01");
      errorBound = 0.01;
    }*/

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

size_t OoOJoin::AbstractWaterMarker::creatWindow(OoOJoin::tsType tBegin, OoOJoin::tsType tEnd) {
    assert(tEnd >= tBegin);
    return 1;
}

bool OoOJoin::AbstractWaterMarker::deleteWindow(size_t wid) {
    return (wid == 1);
}

bool OoOJoin::AbstractWaterMarker::reportTupleS(OoOJoin::TrackTuplePtr ts, size_t wid) {
    assert(ts);
    assert(wid);
    return false;
}

bool OoOJoin::AbstractWaterMarker::reportTupleR(OoOJoin::TrackTuplePtr tr, size_t wid) {
    assert(tr);
    assert(wid);
    return false;
}

double OoOJoin::AbstractWaterMarker::estimateError(size_t wid) {
    assert(wid);
    return 0;
}