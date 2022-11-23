/*! \file NestedLoopJoin.h*/
//
// Created by tony on 19/03/22.
//

#ifndef _JOINALGO_NESTEDLOOPJOIN_H_
#define _JOINALGO_NESTEDLOOPJOIN_H_
#include <JoinAlgos/AbstractJoinAlgo.h>
namespace AllianceDB {
/**
* @ingroup ADB_JOINALGOS
* @{
* @defgroup ADB_JOINALGOS_NLJ The simplest nested loop join
* @{
*Just compare keys in the loop and test if they are matched.
*@}
 * @}
*/
/**
 * @ingroup ADB_JOINALGOS_NLJ
 * @class NestedLoopJoin  JoinAlgos/NestedLoopJoin.h
 *  @brief The top class package of Nested Loop Join providing a "join function"
 *  @note Just single thread, the threads parameter is useless
 */
class NestedLoopJoin : public AbstractJoinAlgo {
 public:
  NestedLoopJoin() {
    setAlgoName("NestedLoopJoin");
  }
  ~NestedLoopJoin() {}
  /**
  * @brief The function to execute join, window to tuple
  * @param windS The window of S tuples
   * @param tr The tuple r
   * @param threads The threads for executing this join
   * @note The threads parameter will be ignored
  * @return The joined tuples
  */
  virtual size_t join(C20Buffer<AllianceDB::TrackTuplePtr> windS, AllianceDB::TrackTuplePtr tr, int threads = 1);
  /**
  * @brief The function to execute join,
  * @param windS The window of S tuples
   * @param windR The window of R tuples
   * @param threads The threads for executing this join
   * @note The threads parameter will be ignored
  * @return The joined tuples
  */
  virtual size_t join(C20Buffer<AllianceDB::TrackTuplePtr> windS,
                      C20Buffer<AllianceDB::TrackTuplePtr> windR,
                      int threads = 1);
};
typedef std::shared_ptr<NestedLoopJoin> NestedLoopJoinPtr;
#define  newNestedLoopJoin() make_shared<NestedLoopJoin>()
}
#endif //ALIANCEDB_INCLUDE_JOINALGO_NESTEDLOOPJOIN_H_
