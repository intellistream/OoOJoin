//
// Created by 86183 on 2023/1/8.
//

#ifndef DISORDERHANDLINGSYSTEM_STREAM_OPERATOR_H
#define DISORDERHANDLINGSYSTEM_STREAM_OPERATOR_H


#include <queue>
#include <unordered_map>
#include <list>
#include <mutex>
#include "Operator/MSWJ/profiler/TupleProductivityProfiler.h"
#include "Operator/MSWJ/common/MSWJDefine.h"
#include "Common/Window.h"
#include "WaterMarker/AbstractWaterMarker.h"
#include <WaterMarker/LatenessWM.h>
#include <Common/StateOfKey.h>
#include <Operator/MeanAQPIAWJOperator.h>
#include "JoinAlgos/NPJ/MultiThreadHashTable.h"

using namespace OoOJoin;

namespace MSWJ {
    class StreamOperator : public MeanAQPIAWJOperator {
    public:
        // The length of r tuple
        size_t rLen{};
        // The pointer to NPJ tuple
        NPJTuplePtr *tr{};
        // The current time
        tsType timeNow{};
        // Whether r tuple is in window
        bool rIsInWindow = false;
        // Whether s tuple is in window
        bool sIsInWindow = false;

        explicit StreamOperator(TupleProductivityProfiler *profiler, INTELLI::ConfigMapPtr config);

        ~StreamOperator() = default;

        // The main execution function of MSWJ
        auto mswjExecution(const TrackTuplePtr &tuple) -> bool;

        auto setConfig(INTELLI::ConfigMapPtr config) -> bool;

        // Get the value of discrete random variable D. If delay(ei) ∈(kg,(k+1)g], Di=k+1
        static auto inline get_D(int delay) -> int {
            return delay % g == 0 ? delay / g : delay / g + 1;
        }

        /**
         * @brief Start this operator
         * @return bool, whether start successfully
         */
        bool start() override;

        /**
         * @brief Stop this operator
         * @return bool, whether stop successfully
         */
        bool stop() override;

        /**
         * @brief Get the joined sum result
         * @return The result
         */
        size_t getResult() override;

        /**
         * @brief Get the joined sum result under AQP
         * @return The result
         */
        size_t getAQPResult() override;

        class IMAStateOfKey : public MeanStateOfKey {
        public:
            // The number of arrived tuples
            // double arrivalSkew = 0, sigmaArrivalSkew = 0;
            // The pointer to the last event tuple
            // The pointer to the last arrival tuple
            // The last seen time
            // The last estimate all tuples
            double lastUnarrivedTuples = 0;

            // The last added number
            // The default constructor of IMAStateOfKey
            IMAStateOfKey() = default;

            // The default destructor of IMAStateOfKey
            ~IMAStateOfKey() = default;
        };

        // The shared pointer to IMAStateOfKey
        typedef std::shared_ptr<IMAStateOfKey> IMAStateOfKeyPtr;
        // Define newIMAStateOfKey using std::make_shared
#define newIMAStateOfKey std::make_shared<IMAStateOfKey>

    private:
        // The tag of MSWJ algorithm
        string algoTag = "MSWJNestedLoopJoin";
        // The pointer to the configuration map
        INTELLI::ConfigMapPtr config;
        // The pointer to the tuple productivity profiler
        TupleProductivityProfiler *productivityProfiler;
    };
}


#endif //DISORDERHANDLINGSYSTEM_STREAM_OPERATOR_H
