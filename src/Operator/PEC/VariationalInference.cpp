//
// Created by 86183 on 2023/4/18.
//

#include "Operator/PEC/VariationalInference.h"

auto VariationalInference::getE(std::vector<double> pX) -> double {
    double E = 0.0;
    for (int i = 0; i < pX.size(); i++) {
        E += pX[i] * i;
    }
    return E;
}
