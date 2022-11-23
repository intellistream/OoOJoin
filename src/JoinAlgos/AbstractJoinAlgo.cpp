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
