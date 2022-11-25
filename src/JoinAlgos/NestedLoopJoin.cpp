//
// Created by tony on 19/03/22.
//

#include <JoinAlgos/NestedLoopJoin.h>
#include <Utils/UtilityFunctions.hpp>
using namespace INTELLI;
using namespace AllianceDB;
size_t NestedLoopJoin::join(C20Buffer<AllianceDB::TrackTuplePtr> windS, AllianceDB::TrackTuplePtr tr, int threads) {
  assert(threads > 0);
  size_t result = 0;
  size_t tsLen = windS.size();
  for (size_t i = 0; i < tsLen; i++) {
    if (windS.data(i)[0]->key == tr->key) {
      //cout<<to_string(ts[i]->subKey)+","+ to_string(tr->subKey)<<endl;
      result++;
    }
  }
  // cout<<result<<endl;
  return result;
}
size_t NestedLoopJoin::join(C20Buffer<AllianceDB::TrackTuplePtr> windS,
                            C20Buffer<AllianceDB::TrackTuplePtr> windR,
                            int threads) {
  size_t result = 0;
  size_t trLen = windR.size();
  for (size_t i = 0; i < trLen; i++) {
    AllianceDB::TrackTuplePtr tr = windR.data(i)[0];
    result += join(windS, tr, threads);
    tr->processedTime= UtilityFunctions::timeLastUs(timeBaseStruct)/timeStep ;
  }
  return result;
}