//
// Created by 86183 on 2023/1/4.
//

#ifndef DISORDERHANDLINGSYSTEM_DEFINE_H
#define DISORDERHANDLINGSYSTEM_DEFINE_H


#include <unordered_map>
#include <queue>
#include <mutex>
#include "parallel-hashmap/parallel_hashmap/phmap.h"
#include "Common/Tuples.h"

//系统参数定义
//搜索粒度
constexpr int g{1};

//自适应时间间隔
constexpr int L{3};

//最大延迟
constexpr int maxDelay = 100;

//用户期待的recall率
constexpr double userRecall{0.4};

//用户设定的时间P
constexpr int P{4};

//basic window size
constexpr int b{1};

//用于估计R的可信度值，范围(0,1)
constexpr double confidenceValue{0.5};

class Stream;

//获得离散随机变量Di(粗粒度延迟)的值
auto get_D(int delay) -> int;



struct TupleComparator {
    //按到达时间来排序
    bool operator()(OoOJoin::TrackTuple e1, OoOJoin::TrackTuple e2) const {
        return e1.eventTime < e2.eventTime;
    }
};


class Stream {
public:

    explicit Stream(int stream_id, int window_size, std::queue<OoOJoin::TrackTuple> tuple_list);

    ~Stream() = default;

    auto get_window_size() const -> int;

    auto get_id() const -> int;

    auto get_tuple_list() -> std::queue<OoOJoin::TrackTuple>;

    auto pop_tuple() -> void;

    auto push_tuple(OoOJoin::TrackTuple tuple) -> void;

    auto set_id(int id) -> void;

private:

    //论文中的Wi
    int window_size_{};

    //流id
    int stream_id_{};

    //元组
    std::queue<OoOJoin::TrackTuple> tuple_list_{};
};


#endif //DISORDERHANDLINGSYSTEM_DEFINE_H
