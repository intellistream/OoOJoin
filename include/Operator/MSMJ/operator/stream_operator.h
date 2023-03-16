//
// Created by 86183 on 2023/1/8.
//

#ifndef DISORDERHANDLINGSYSTEM_STREAM_OPERATOR_H
#define DISORDERHANDLINGSYSTEM_STREAM_OPERATOR_H


#include <queue>
#include <unordered_map>
#include <list>
#include <mutex>
#include "Operator/MSMJ/profiler/tuple_productivity_profiler.h"
#include "Operator/MSMJ/common/define.h"
#include "Common/Window.h"
#include "WaterMarker/AbstractWaterMarker.h"
#include <WaterMarker/LatenessWM.h>
#include <Common/StateOfKey.h>
#include <Operator/MeanAQPIAWJOperator.h>
#include "JoinAlgos/NPJ/MultiThreadHashTable.h"

using namespace OoOJoin;
namespace MSMJ {


    class StreamOperator : public MeanAQPIAWJOperator {
    public:
        size_t rLen{};
        NPJTuplePtr *tr{};
        tsType timeNow{};

        bool rIsInWindow = false;

        bool sIsInWindow = false;

        explicit StreamOperator(TupleProductivityProfiler *profiler, INTELLI::ConfigMapPtr config);

        ~StreamOperator() = default;

        auto mswj_execution(const TrackTuplePtr &tuple) -> bool;

        auto setConfig(INTELLI::ConfigMapPtr config) -> bool;

        //获得离散随机变量Di的值,如果delay(ei) ∈(kg,(k+1)g]，则Di=k+1
        static auto inline get_D(int delay) -> int {
            return delay % g == 0 ? delay / g : delay / g + 1;
        }

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

        /**
         * @brief get the joined sum result under AQP
         * @return The result
         */
        size_t getAQPResult() override;


        class IMAStateOfKey : public MeanStateOfKey {
        public:
            // size_t arrivedTupleCnt = 0;
            //  double arrivalSkew = 0, sigmaArrivalSkew = 0;
            // TrackTuplePtr lastEventTuple = nullptr, lastArrivalTuple = nullptr;
            // tsType  lastSeenTime=0;
            //size_t lastEstimateAllTuples=0;
            double lastUnarrivedTuples = 0;

            // size_t lastAdded=0;
            IMAStateOfKey() = default;

            ~IMAStateOfKey() = default;
        };



        typedef std::shared_ptr<IMAStateOfKey> IMAStateOfKeyPtr;

#define newIMAStateOfKey std::make_shared<IMAStateOfKey>


    private:

        string algoTag = "MSWJNestedLoopJoin";

        uint64_t joinThreads = 1;

        INTELLI::ConfigMapPtr config;

        //元组生产力监视器
        TupleProductivityProfiler *productivity_profiler_;

    };

}


#endif //DISORDERHANDLINGSYSTEM_STREAM_OPERATOR_H
