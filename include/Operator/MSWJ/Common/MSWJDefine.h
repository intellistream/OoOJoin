//
// Created by 86183 on 2023/1/4.
//

#ifndef DISORDERHANDLINGSYSTEM_DEFINE_H
#define DISORDERHANDLINGSYSTEM_DEFINE_H

#include <unordered_map>
#include <queue>
#include <mutex>

//SystemParam of yuanzhen's paper
namespace MSWJ {
    /**
     * @brief The search granularity value used in yuanzhen's paper.
     */
    extern int g;

    /**
     * @brief The adaptive interval value used in yuanzhen's paper.
     */
    extern int L;

    /**
     * @brief The maximum delay value used in yuanzhen's paper.
     */
    extern int maxDelay;

    /**
     * @brief The user expected recall rate value used in yuanzhen's paper.
     */
    extern double userRecall;

    /**
     * @brief The user defined time P value used in yuanzhen's paper.
     */
    extern int P;

    /**
     * @brief The basic window size value used in yuanzhen's paper.
     */
    extern int b;

    /**
     * @brief The confidence value for estimating the credibility of R value used in yuanzhen's paper.
     */
    extern double confidenceValue;

    /**
     * @brief The number of streams value used in yuanzhen's paper.
     */
    extern int streamCount;
}
#endif //DISORDERHANDLINGSYSTEM_DEFINE_H
