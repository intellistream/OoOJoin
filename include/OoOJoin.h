/*! \file OoOJoin.h*/
//
// Created by tony on 22/11/22.
//

#ifndef INTELLISTREAM_OOOJOIN_H
#define INTELLISTREAM_OOOJOIN_H
/**
 * @mainpage Introduction
 * The OoOJoin project contains the basic ideas and evaluation on a novel join operator that supports error-bounded processing
 * on out-of-order tuples
 * @section Sys_overview  System Overview
 * The whole work is complete join operator, including basic windowing, buffering and join computation. However, watermark
 * generation is the major focus.
 * \image html System.png
 * @section Code_overview  Code Overview
 * The code structure aligns well with system design.
 * \image html UML_ALL.png
 */
//The groups of modules
/**
 * * @defgroup INTELLI_COMMON_BASIC Basic Definitions and Data Structures
 * @{
 * We define the classes of Tuple, window, queue, etc. here
 **/
#include <Common/OoOTuple.h>
/**
 * @}
 */
#endif //INTELLISTREAM_OOOJOIN_H
