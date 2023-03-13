//
// Created by 86183 on 2023/1/10.
//
#include <iostream>
#include "Operator/MSMJ/common/define.h"

namespace MSMJ {
//系统参数定义
//搜索粒度
    int g{10000};

//自适应时间间隔
    int L{500000};

//最大延迟
    int maxDelay = INT16_MAX;

//用户期待的recall率
    double userRecall{0.4};

//用户设定的时间P
    int P{600000};

//basic window size
    int b{1000};

//用于估计R的可信度值，范围(0,1)
    double confidenceValue{0.5};

    int streamCount{2};

}
