//
// Created by 86183 on 2023/1/8.
//





#include <utility>

#include "Operator/MSMJ/common/define.h"

Stream::Stream(uint64_t stream_id, uint64_t window_size, std::queue<OoOJoin::TrackTuple> tuple_list) {
    stream_id_ = stream_id;
    window_size_ = window_size;
    tuple_list_ = std::move(tuple_list);
}

auto Stream::get_window_size() const -> uint64_t {
    return window_size_;
}

auto Stream::get_id() const -> uint64_t {
    return stream_id_;
}

auto Stream::get_tuple_list() -> std::queue<OoOJoin::TrackTuple> {
    return tuple_list_;
}

auto Stream::pop_tuple() -> void {
    tuple_list_.pop();
}

auto Stream::push_tuple(OoOJoin::TrackTuple tuple) -> void {
    tuple_list_.push(tuple);
}

auto Stream::set_id(int id) -> void {
    stream_id_ = id;
}



