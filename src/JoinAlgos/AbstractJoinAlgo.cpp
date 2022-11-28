//
// Created by tony on 11/03/22.
//

#include <JoinAlgos/AbstractJoinAlgo.h>
using namespace AllianceDB;
size_t AllianceDB::AbstractJoinAlgo::join(C20Buffer<AllianceDB::TrackTuplePtr> windS,
                                          C20Buffer<AllianceDB::TrackTuplePtr> windR,
                                          int threads) {

  assert(windS.data());
  assert(windR.data());
  assert(threads);
  return 0;
}
bool AllianceDB::AbstractJoinAlgo::setConfig(INTELLI::ConfigMapPtr cfg) {
  config = cfg;
  if (config == nullptr) {
    return false;
  }
 /* if (config->existU64("timeStep")) {
    timeStep = config->getU64("timeStep");
  } else {
        ALGO_WARNNING("No setting of timeStep, use 1\n");
    timeStep = 1;
  }*/
  return true;
}
