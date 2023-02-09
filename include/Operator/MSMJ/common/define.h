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

struct TupleComparator {
    //按到达时间来排序
    bool operator()(OoOJoin::TrackTuple e1, OoOJoin::TrackTuple e2) const {
        return e1.eventTime < e2.eventTime;
    }
};


class Stream {
public:

    explicit Stream(uint64_t stream_id, uint64_t window_size, std::queue<OoOJoin::TrackTuple> tuple_list);

    ~Stream() = default;

    auto get_window_size() const -> uint64_t;

    auto get_id() const -> uint64_t;

    auto get_tuple_list() -> std::queue<OoOJoin::TrackTuple>;

    auto pop_tuple() -> void;

    auto push_tuple(OoOJoin::TrackTuple tuple) -> void;

    auto set_id(int id) -> void;

private:

    //论文中的Wi
    uint64_t window_size_{};

    //流id
    uint64_t stream_id_{};

    //元组
    std::queue<OoOJoin::TrackTuple> tuple_list_{};
};


#endif //DISORDERHANDLINGSYSTEM_DEFINE_H
