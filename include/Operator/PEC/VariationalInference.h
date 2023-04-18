//
// Created by 86183 on 2023/4/18.
//

#ifndef INTELLISTREAM_VARIATIONALINFERENCE_H
#define INTELLISTREAM_VARIATIONALINFERENCE_H

#include <vector>

class VariationalInference {
public:
    //global variable U, format: key-μ
    std::vector<std::vector<double>> U;
    //P(U) format: key-P(μ)
    std::vector<std::vector<double>> pU;
    //local variable Z
    std::vector<double> z;


    auto getE(std::vector<double> pX) -> double;


};


#endif //INTELLISTREAM_VARIATIONALINFERENCE_H
