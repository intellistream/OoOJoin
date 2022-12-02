/*! \file StateOfKey.h*/
//
// Created by tony on 02/12/22.
//

#ifndef INTELLISTREAM_STATEOFKEY_H
#define INTELLISTREAM_STATEOFKEY_H
#include <stdint.h>
#include <string>
#include <memory>
#include <Common/Tuples.h>
#include <vector>
namespace OoOJoin {

/**
* @ingroup INTELLI_COMMON_BASIC Basic Definitions and Data Structures
* @{
 * @defgroup INTELLI_COMMON_BASIC_STATE_OF_KEY The state of key group
 * @{
*
*/
/**
 * @class  AbstractStateOfKey Common/StateOfKey.h
 * @brief The statistics and prediction state of a key
 * @note normally inherited by more complicated states
 * @ingroup INTELLI_COMMON_BASIC_STATE_OF_KEY
 */
class AbstractStateOfKey {
public:
    AbstractStateOfKey(){

    }
    ~AbstractStateOfKey(){}
    keyType key=0;
};
/**
 * @typedef AbstractStateOfKeyPtr
 * @brief The class to describe a shared pointer to @ref AbstractStateOfKey
 */
typedef std::shared_ptr<class AbstractStateOfKey> AbstractStateOfKeyPtr;
    class StateOfKeyBucket;
    typedef std::shared_ptr<class StateOfKeyBucket>StateOfKeyBucketPtr;
/**
 * @class StateOfKeyBucket Common/StateOfKey.h
 * @brief The multithread-supported bucket of @ref StateOfKey
 * @ingroup INTELLI_COMMON_BASIC_STATE_OF_KEY
 */
class StateOfKeyBucket {
    private:
        std::mutex m_mut;
        //vector<NPJTuplePtr>tuples;
    size_t bucketSize;

    public:
    size_t count = 0;
    std::vector<AbstractStateOfKeyPtr>cells;
    StateOfKeyBucketPtr next = nullptr;
    /**
     * @brief set how many cells are allowed in one bucket
     * @param cnt
     * @note Please set correct number for cache awareness
     */
        void setCellCount(size_t cnt)
        {
            cells=std::vector<AbstractStateOfKeyPtr>(cnt);
            bucketSize=cnt;
        }
        /**
         * @brief lock this bucket
         */
        void lock() {
            while (!m_mut.try_lock());
        }
        /**
         * @brief unlock this bucket
         */
        void unlock() {
            m_mut.unlock();
        }
        /**
         * @brief Insert a StateOfKey
         * @param ask The AbstractStateOfKey
         */
        void insert(AbstractStateOfKeyPtr ask);
        /**
         * @brief probe if we have something with the same key as ask
         * @param probeKey The key for probing
         * @return the stored AbstractStateOfKey, nullptr if nothing
         */
        AbstractStateOfKeyPtr getByKey(keyType probeKey);
        /**
         * @brief get the next bucket
         * @return this->next
         */
        /*MtBucketPtr getNext(){
          return next;
        }*/
    };
    /**
    * @class StateOfKeyHashTable Common/StateOfKey.h
    * @brief The multithread-supported hash table, holding buckets of stateofkey
     * @ingroup INTELLI_COMMON_BASIC_STATE_OF_KEY
    */
    class StateOfKeyHashTable {
    private:

    public:
        keyType hash_mask;
        keyType skip_bits;
        std::vector<StateOfKeyBucket> buckets;
        StateOfKeyHashTable() {}
        /**
         * @brief pre-init with several buckets
         * @param bks the number of buckets
         * @param cells How many cells are allowed in one bucket
         */
        StateOfKeyHashTable(size_t bks,size_t cells=4);
        ~StateOfKeyHashTable() {}
        /**
         * @brief Insert a StateOfKey
         * @param ask The AbstractStateOfKey
         * @note not thread safe, use insertLock instaead for thread safe
         */
        void insert(AbstractStateOfKeyPtr ask);
        /**
      * @brief Insert a StateOfKey, a thread safe version
      * @param ask The AbstractStateOfKey
      */
        void insertSafe(AbstractStateOfKeyPtr ask);
        /**
         * @brief probe if we have something with the same key as ask
         * @param probeKey The key for probing
         * @return the stored AbstractStateOfKey, nullptr if nothing
         */
        AbstractStateOfKeyPtr getByKey(keyType probeKey);

    };
    typedef std::shared_ptr<StateOfKeyHashTable> StateOfKeyHashTablePtr;
/**
 * @}
 */
/**
* @}
*/
}
#endif //INTELLISTREAM_STATEOFKEY_H
