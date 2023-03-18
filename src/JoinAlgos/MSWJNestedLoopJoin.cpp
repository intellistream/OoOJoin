
#include <JoinAlgos/MSWJNestedLoopJoin.h>
#include <Utils/UtilityFunctions.hpp>

using namespace INTELLI;
using namespace OoOJoin;

size_t MSWJNestedLoopJoin::join(C20Buffer<OoOJoin::TrackTuplePtr> windS, OoOJoin::TrackTuplePtr tr, int threads) {
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

size_t MSWJNestedLoopJoin::join(C20Buffer<OoOJoin::TrackTuplePtr> windS,
                                C20Buffer<OoOJoin::TrackTuplePtr> windR,
                                int threads) {
    size_t result = 0;
    size_t trLen = windR.size();
    for (size_t i = 0; i < trLen; i++) {
        OoOJoin::TrackTuplePtr tr = windR.data(i)[0];
        int nestResult = join(windS, tr, threads);
        //update to profiler
        productivity_profiler_->updateJoinRes(MSWJ::TupleProductivityProfiler::getD(tr->delay), nestResult);
        result += nestResult;
        tr->processedTime = UtilityFunctions::timeLastUs(timeBaseStruct);
    }
    return result;
}