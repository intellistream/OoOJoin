//
// Created by 86183 on 2023/1/6.
//

#include <map>
#include <list>
#include <complex>
#include <iostream>
#include <utility>
#include "Operator/MSWJ/manager/StatisticsManager.h"
#include "Operator/MSWJ/common/MSWJDefine.h"
#include "Operator/MSWJ/profiler/TupleProductivityProfiler.h"

using namespace MSWJ;

StatisticsManager::StatisticsManager(TupleProductivityProfiler *profiler, INTELLI::ConfigMapPtr config)
        : cfg(std::move(config)), productivityProfiler(profiler) {
    recordMap.resize(streamCount + 1);
    kSyncMap.resize(streamCount + 1);
    RStatMap.resize(streamCount + 1);
    tMap.resize(streamCount + 1);
    kMap.resize(streamCount + 1);
    histogramMap.resize(streamCount + 1);
    histogramPos.resize(streamCount + 1);
}


auto StatisticsManager::addRecord(int streamId, const TrackTuplePtr &tuple) -> void {
    recordMap[streamId].push_back(*tuple);
}

auto StatisticsManager::addRecord(int streamId, int T, int K) -> void {
    tMap[streamId] = T;
    kMap[streamId] = K;
    kSyncMap[streamId].push_back(getKsync(streamId));
}


auto StatisticsManager::getMaxD(int streamId) -> int {
    int maxD = 0;

    if (recordMap[streamId].empty()) {
        return maxD;
    }

    for (auto it: recordMap[streamId]) {
        maxD = std::max(maxD, (int) it.delay);
    }
    return maxD;
}


auto StatisticsManager::getRStat(int streamId) -> int {
    std::vector<TrackTuple> record = recordMap[streamId];
    if (record.empty()) {
        return 1;
    }

    //当前Rstat大小
    int RStat = 2;
    if (RStatMap[streamId] != 0) {
        RStat = RStatMap[streamId];
    }

    //窗口
    std::list<int> window1List;
    std::list<int> window0List;

    //维护sum变量,方便取平均值
    int sumW0 = 0;
    int sumW1 = 0;

    //取出record里面当前R_stat大小范围的数据,并计算频率，用频率估计概率
    for (int i = record.size() - 1; i >= std::max((int) record.size() - RStat, 0); i--) {
        int xi = getD(record[i].delay);
        window1List.push_back(xi);
        sumW1 += xi;
    }

    double eCut = 0;
    while (window1List.size() > 1) {
        //将窗口分为w0,w1两部分
        int w1Front = window1List.front();
        window0List.push_back(w1Front);
        window1List.pop_front();
        sumW1 -= w1Front;
        sumW0 += w1Front;

        //更新e_cut
        double m = 1 / (1 / window0List.size() + 1 / window1List.size());
        double confidenceValue = confidenceValue / (window1List.size() + window0List.size());
        eCut = std::sqrt((1 / 2 * m) * std::log1p(4 / confidenceValue));

        if (std::abs(sumW1 * 1.0 / window1List.size() - sumW0 * 1.0 / window0List.size()) >= eCut) {
            RStatMap[streamId] = window1List.size();
            break;
        }
    }

    return window1List.size();
}


//获取Ksync的值，Ksync = iT - ki - min{iT - ki| i∈[1,latch_]}
auto StatisticsManager::getKsync(int streamId) -> int {
    if (tMap[streamId] == 0
        || kMap[streamId] == 0
        || recordMap.empty()) {
        return 0;
    }

    int miniTKi = tMap[streamId] - kMap[streamId];

    for (int i = 0; i < recordMap.size(); i++) {
        if (recordMap[i].empty())continue;
        miniTKi = std::min(miniTKi, tMap[i] - kMap[i]);
    }
    return tMap[streamId] - kMap[streamId] - miniTKi;
}


//获取平均值，目的是为了预测未来的k_sync
auto StatisticsManager::getAvgKsync(int streamId) -> int {
    int sumKsynci = 0;
    int R_stat = getRStat(streamId);

    std::vector<int> kSyncList = kSyncMap[streamId];
    if (kSyncList.empty()) {
        return 0;
    }

    //找出R_stat范围内的ksync_i的总和，再取平均值
    for (int i = kSyncList.size() - 1; i >= std::max((int) kSyncList.size() - R_stat, 0); i--) {
        sumKsynci += kSyncMap[streamId][i];
    }
    int avgKsynci = sumKsynci / R_stat;

    return avgKsynci == 0 ? 0 : avgKsynci;
}

//公式见论文page 7
auto StatisticsManager::getFutureKsync(int stream_id) -> int {
    if (kSyncMap.empty()) {
        return 0;
    }

    int avgksynci = getAvgKsync(stream_id);
    int minKsync = INT32_MAX;

    //找到j != i的所有avg_ksync的最小值
    for (int i = 0; i < kSyncMap.size(); i++) {
        if (kSyncMap[i].empty())continue;
        minKsync = std::min(minKsync, getAvgKsync(i));
    }

    if (minKsync == INT32_MAX) {
        minKsync = 0;
    }
    return avgksynci - minKsync;
}


//概率分布函数fD
auto StatisticsManager::fD(int d, int stream_id) -> double {
    int R_stat = getRStat(stream_id);

    if (recordMap[stream_id].empty()) {
        return -1;
    }

    std::vector<TrackTuple> record = recordMap[stream_id];
    if (record.empty()) {
        return 1;
    }

    //取出record里面R_stat大小范围的数据,并计算频率，用频率估计概率
    std::map<int, int> rate_map;
    for (int i = record.size() - 1; i >= std::max((int) record.size() - R_stat, 0); i--) {
        int Di = getD(record[i].delay);
        rate_map[Di] = rate_map.find(Di) == rate_map.end() ? 1 : rate_map[Di] + 1;
    }

    //用直方图模拟
    if (histogramMap[stream_id].empty()) {
        histogramMap[stream_id].resize(maxDelay);
    }

    double sumP = 0;
    for (auto it: rate_map) {
        if (histogramMap[stream_id][it.first] != 0) {
            //原先的直方图已经统计过了该Di的概率，故取平均值
            histogramMap[stream_id][it.first] = (histogramMap[stream_id][it.first] + it.second * 1.0 / R_stat) / 2;
        } else {
            histogramMap[stream_id][it.first] = it.second * 1.0 / R_stat;
            histogramPos[stream_id].push_back(it.first);
        }
    }

    for (int i = 0; i < histogramPos[stream_id].size(); i++) {
        sumP += histogramMap[stream_id][histogramPos[stream_id][i]];
    }

    //如果d已经有现成的概率，则直接返回即可
    if (histogramMap[stream_id][d] != 0) {
        //前面可能更新过了直方图，返回前做一次归一化
        if (sumP != 0) {
            for (int i = 0; i < histogramPos[stream_id].size(); i++) {
                if (histogramMap[stream_id][histogramPos[stream_id][i]] != 0) {
                    histogramMap[stream_id][histogramPos[stream_id][i]] /= sumP;
                }
            }
        }
        return histogramMap[stream_id][d];
    }

    //如果d没有记录，则做折线插值估计 , 双指针中心扩散法
    int hi_size = histogramMap[stream_id].size();

    bool flag = false;
    int left = d - 1, right = d + 1;
    while (left >= 0 && right < hi_size) {
        if (histogramMap[stream_id][left] != 0 && histogramMap[stream_id][right] != 0) {
            flag = true;
            break;
        }
        if (histogramMap[stream_id][left] == 0) {
            left--;
        }
        if (histogramMap[stream_id][right] == 0) {
            right++;
        }
    }

    if (!flag) {
        if (left < 0) {
            left = d + 1, right = d + 2;
            while (right < hi_size) {
                if (histogramMap[stream_id][left] != 0 && histogramMap[stream_id][right] != 0) {
                    break;
                }
                if (histogramMap[stream_id][left] == 0) {
                    left++;
                }
                if (histogramMap[stream_id][right] == 0) {
                    right++;
                }
            }
        } else if (right >= hi_size) {
            if (d - 1 < 0 || d - 2 < 0) {
                //说明此时左边已经没有元素了,直接break
                right = left;
            } else {
                left = d - 2, right = d - 1;
                while (left >= 0) {
                    if (histogramMap[stream_id][left] != 0 && histogramMap[stream_id][right] != 0) {
                        break;
                    }
                    if (histogramMap[stream_id][left] == 0) {
                        left--;
                    }
                    if (histogramMap[stream_id][right] == 0) {
                        right--;
                    }
                }
            }
        }
    }

    if (left == right) {
        histogramMap[stream_id][left] /= 2;
        histogramMap[stream_id][d] = histogramMap[stream_id][left];
        histogramPos[stream_id].push_back(d);
        return histogramMap[stream_id][d];
    }

    double pL = histogramMap[stream_id][left];
    double pR = histogramMap[stream_id][right];

    //此时left和right分别指向了有实际数据的点，可用折线估计概率
    //直线方程： y =  (pR - pL)/(right - left)(x - left) + pL
    double pD = (pR - pL) / (right - left) * (d - left) + pL;

    if (pD < 0) {
        pD = 1;
    }
    //更新直方图
    histogramMap[stream_id][d] = pD;
    histogramPos[stream_id].push_back(d);

    //归一化
    sumP += pD;
    for (int i = 0; i < histogramPos[stream_id].size(); i++) {
        if (histogramMap[stream_id][histogramPos[stream_id][i]] != 0) {
            histogramMap[stream_id][histogramPos[stream_id][i]] /= sumP;
        }
    }

    return histogramMap[stream_id][d];
}

auto StatisticsManager::fDk(int d, int streamId, int K) -> double {
    int kSync = getFutureKsync(streamId);
    double res = 0;

    if (d == 0) {
        for (int i = 0; i <= (kSync + K) / g; i++) {
            res += fD(i, streamId);
        }
    } else {
        res = fD(d + (K + kSync) / g, streamId);
    }

    return std::abs(res);
}

auto StatisticsManager::wil(int l, int streamId, int K) -> int {
    std::string key = "Stream_" + std::to_string(streamId);
    int wi = cfg->getU64(key);;
    int ni = wi / b;
    double res = 0;
    double ri = productivityProfiler->getJoinRecordMap()[streamId] * 1.0 / wi;

    if (l <= ni - 1 && l >= 1) {
        for (int i = 0; i <= (l - 1) * b / g; i++) {
            res += fDk(i, streamId, K);
        }
        res = std::ceil(ri * b * res);
    } else if (l == ni) {
        for (int i = 0; i <= (ni - 1) * b / g; i++) {
            res += fDk(i, streamId, K);
        }
        res = std::ceil(ri * (wi - (ni - 1) * b) * res);
    }

    return std::abs(res);
}

auto StatisticsManager::setConfig(INTELLI::ConfigMapPtr config) -> void {
    cfg = std::move(config);
}



