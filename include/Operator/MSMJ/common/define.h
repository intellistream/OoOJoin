//
// Created by 86183 on 2023/1/4.
//

#ifndef DISORDERHANDLINGSYSTEM_DEFINE_H
#define DISORDERHANDLINGSYSTEM_DEFINE_H


#include <unordered_map>
#include <queue>
#include <mutex>
#include "parallel-hashmap/parallel_hashmap/phmap.h"

namespace MSMJ {
//系统参数定义
//搜索粒度
    extern int g;

//自适应时间间隔
    extern int L;

//最大延迟
    extern int maxDelay;

//用户期待的recall率
    extern double userRecall;

//用户设定的时间P
    extern int P;

//basic window size
    extern int b;

//用于估计R的可信度值，范围(0,1)
    extern double confidenceValue;

    extern int streamCount;

    class Stream;

    struct Tuple;

    typedef uint64_t keyType;    /*!< Type of the join key, default uint64_t */
    typedef uint64_t valueType;  /*!< Type of the payload, default uint64_t */
    struct Tuple {
        keyType key;
        valueType payload;
        uint64_t arrivalTime;
        uint64_t processedTime;
        //表示来自输入流Si
        int streamId;
        //第几个到达的元组
        int id;
        //时间戳
        int ts;
        //延迟
        int delay{};

        //标识符
        bool end;

        Tuple(int streamId, int id, int ts) : streamId(streamId), id(id), ts(ts) {}

        bool operator<(const Tuple &other) const {
            return ts > other.ts;
        }
    };

    struct TupleComparator {
        //按到达时间来排序
        bool operator()(Tuple e1, Tuple e2) const {
            return e1.ts < e2.ts;
        }
    };


    class Stream {
    public:

        explicit Stream(int stream_id, int window_size, std::queue<Tuple> tuple_list);

        ~Stream() = default;

        auto get_window_size() -> int;

        auto get_id() -> int;

        auto get_tuple_list() -> std::queue<Tuple> &;

        auto pop_tuple() -> void;

        auto push_tuple(Tuple tuple) -> void;

    private:

        //论文中的Wi
        int window_size_{};

        //流id
        int stream_id_{};

        //元组
        std::queue<Tuple> tuple_list_{};
    };

}
#endif //DISORDERHANDLINGSYSTEM_DEFINE_H
