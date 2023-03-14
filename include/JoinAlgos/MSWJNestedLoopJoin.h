//
// Created by 86183 on 2023/3/14.
//

#ifndef INTELLISTREAM_MSWJNESTEDLOOPJOIN_H
#define INTELLISTREAM_MSWJNESTEDLOOPJOIN_H

#include "Operator/MSMJ/profiler/tuple_productivity_profiler.h"
#include <JoinAlgos/AbstractJoinAlgo.h>

namespace OoOJoin {

    class MSWJNestedLoopJoin : public AbstractJoinAlgo {
    public:

        MSWJNestedLoopJoin() {
            setAlgoName("MSWJNestedLoopJoin");
        }

        ~MSWJNestedLoopJoin() {}

        /**
        * @brief The function to execute join, window to tuple
        * @param windS The window of S tuples
         * @param tr The tuple r
         * @param threads The threads for executing this join
         * @note The threads parameter will be ignored
        * @return The joined tuples
        */
        virtual size_t join(C20Buffer <OoOJoin::TrackTuplePtr> windS, OoOJoin::TrackTuplePtr tr, int threads = 1);

        /**
        * @brief The function to execute join,
        * @param windS The window of S tuples
         * @param windR The window of R tuples
         * @param threads The threads for executing this join
         * @note The threads parameter will be ignored
        * @return The joined tuples
        */
        virtual size_t join(C20Buffer <OoOJoin::TrackTuplePtr> windS,
                            C20Buffer <OoOJoin::TrackTuplePtr> windR,
                            int threads = 1);
    };

/**
 * @typedef NestedLoopJoinPtr
 * @ingroup ADB_JOINALGOS_NLJ
 * @brief The class to describe a shared pointer to @ref NestedLoopJoin
 */
    typedef std::shared_ptr<MSWJNestedLoopJoin> MSWJNestedLoopJoinPtr;
/**
 * @def newNestedLoopJoin
 * @ingroup ADB_JOINALGOS_NLJ
 * @brief (Macro) To creat a new @ref newNestedLoop under shared pointer.
 */
#define  newMSWJNestedLoopJoin std::make_shared<MSWJNestedLoopJoin>


#endif //INTELLISTREAM_MSWJNESTEDLOOPJOIN_H
}
