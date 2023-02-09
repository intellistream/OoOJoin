#ifndef INTELLISTREAM_INCLUDE_OPERATOR_IAWJOPERATOR_H_
#define INTELLISTREAM_INCLUDE_OPERATOR_IAWJOPERATOR_H_

#include <Operator/AbstractOperator.h>
#include <Common/Window.h>
#include <atomic>
#include <WaterMarker/LatenessWM.h>
#include "Operator/MSMJ/kslack/k_slack.h"

namespace OoOJoin {
    /**
 * @ingroup ADB_OPERATORS
 * @typedef MSWJOperatorPtr
 * @brief The class to describe a shared pointer to @ref MSWJOperator
 */

    typedef std::shared_ptr<class Stream> StreamPtr;
    typedef std::shared_ptr<class MSMJOperator> MSMJOperatorPtr;
    typedef std::shared_ptr<class KSlack> KSlackPtr;
    typedef std::shared_ptr<class BufferSizeManager> BufferSizeManagerPtr;
    typedef std::shared_ptr<class StatisticsManager> StatisticsManagerPtr;
    typedef std::shared_ptr<class TupleProductivityProfiler> TupleProductivityProfilerPtr;
/**
 * @ingroup ADB_OPERATORS
 * @def newMSWJOperator
 * @brief (Macro) To creat a new @ref MSWJOperator under shared pointer.
 */
#define newMSMJOperator std::make_shared<OoOJoin::MSMJOperator>

/**
 * @class MSWJOperator
 * @ingroup ADB_OPERATORS
 * @class MSWJOperator Operator/MSWJOperator.h
 * @brief The intra window join (MSWJ) operator, only considers a single window
 * @note require configurations:
 * - "windowLen" U64: The length of window
 * - "slideLen" U64: The length of slide
 * - "sLen" U64: The length of S buffer,
 * - "rLen" U64: The length of R buffer
 * - "algo" String: The specific join algorithm (optional, default nested loop)
 * - "threads" U64: The threads to conduct intra window join (optional, default 1)
 * - "wmTag" String: The tag of watermarker, default is arrival for @ref ArrivalWM
 * @note In current version, the computation will block feeding
 * @note operator tag = "MSWJ"
 */
    class MSMJOperator : public AbstractOperator {
    protected:
        Window myWindow;
        size_t intermediateResult = 0;
        string algoTag = "NestedLoopJoin";
        uint64_t joinThreads = 1;
        /**
         * @brief if operator is locked by watermark, it will never accept new incoming
         * @todo current implementation is putting rotten, fix later
         */
        atomic_bool lockedByWaterMark = false;
        AbstractWaterMarkerPtr wmGen = nullptr;

        void conductComputation();

    private:
        //流
        StreamPtr streamS;
        StreamPtr streamR;

        //bufferSizeManager,  is used to update K(bufferSize)
        BufferSizeManagerPtr bufferSizeManager{};

        //statisticsManager, is used to statistic
        StatisticsManagerPtr statisticsManager{};

        //TupleProductivityProfiler,is used to get productivity of tuple at join stage.
        TupleProductivityProfilerPtr tupleProductivityProfiler{};

        SynchronizerPtr synchronizer{};

    public:
        MSMJOperator();

        ~MSMJOperator() = default;

        /**
         * @todo Where this operator is conducting join is still putting rotten, try to place it at feedTupleS/R
        * @brief Set the config map related to this operator
        * @param cfg The config map
         * @return bool whether the config is successfully set
        */
        bool setConfig(ConfigMapPtr cfg) override;

        /**
       * @brief feed a tuple s into the Operator
       * @param ts The tuple
        * @warning The current version is simplified and assuming only used in SINGLE THREAD!
        * @return bool, whether tuple is fed.
       */
        bool feedTupleS(TrackTuplePtr ts) override;

        /**
          * @brief feed a tuple R into the Operator
          * @param tr The tuple
          * @warning The current version is simplified and assuming only used in SINGLE THREAD!
          *  @return bool, whether tuple is fed.
          */
        bool feedTupleR(TrackTuplePtr tr) override;

        /**
         * @brief start this operator
         * @return bool, whether start successfully
         */
        bool start() override;

        /**
         * @brief stop this operator
         * @return bool, whether start successfully
         */
        bool stop() override;

        /**
         * @brief get the joined sum result
         * @return The result
         */
        size_t getResult() override;

    };

}
#endif //INTELLISTREAM_INCLUDE_OPERATOR_IAWJOPERATOR_H_
