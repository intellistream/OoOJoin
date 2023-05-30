#include <Common/LinearSVI.h>
#include <torch/optim/adam.h>
using namespace TROCHPACK_SVI;
using namespace std;
using namespace torch;
//
// Created by tony on 29/05/23.
//
void TROCHPACK_SVI::LinearSVI::initSVIParameters(uint64_t latentD) {
  latentDimension=latentD;
  /**
   * @brief register parameters
   */
  mu = torch::rand({1});
  tau =  torch::rand({1});
 // logTau = register_parameter("logtau", torch::zeros({1}));
  auto tz=torch::rand({1,(long )latentD});
  auto sumZ=tz.sum();
  latentZ= register_parameter("latentZ",((int)latentD)*tz/sumZ);
  std::cout<<latentZ<<std::endl;
  std::cout<<latentZ.sum()<<std::endl;
  auto parameters = this->parameters();
  //create the optimiser
  vector<torch::Tensor> params_vec;
  for (auto it = parameters.begin(); it != parameters.end(); ++it) {
    params_vec.push_back(*it);
  }
  myOpt = newtorchOptimiser(params_vec, torch::optim::AdamOptions(0.1));
}
torch::Tensor TROCHPACK_SVI::LinearSVI::minusELBO(torch::Tensor& x)
{ int64_t n=x.size(1);
  torch::Tensor elbo=torch::zeros({1});
  auto logPmu0=torch::log(mu);
  auto logPtau0= torch::log(tau);
  auto logPz0=torch::log(latentZ).sum();
  for (int64_t i = 0; i < x.size(0); i++) {

    // Get the current row
    torch::Tensor rowX = x[i];
    /**
     * @brief log likelyhood of x|A
     */
    auto xz=torch::mul(latentZ,rowX);
    auto xzPmu=xz-mu;
    auto xzPmu2=torch::mul(xzPmu,xzPmu);
    auto logLikelyHood=-tau/2+torch::sum(xzPmu2)+n/2*torch::log(tau);
    irMu=(torch::sum(xz)+mu)/(1+n);
    /**
     * @brief E_q(ln(q(mu))), in mean filed, the expectation of the mu-only pdf, consider others as const
     */
     auto logEqmu=torch::log(irMu);
    /**
    * @brief E_q(ln(q(tau))), in mean filed, the expectation of the mu-only pdf, consider others as const
    */
    irTau=(tau+n/2)*torch::reciprocal((1+0.5*torch::sum(xzPmu2)+torch::mul(irMu-mu,irMu-mu)));
    auto logEqtau= torch::log(irTau);
    /**
   * @brief sum(E_q(ln(q(z)))), in mean filed, the expectation of the mu-only pdf, consider others as const
   */
    auto muRecX=mu*torch::reciprocal(rowX);
    auto logMuRecX=torch::log(muRecX);
    auto logEqz=torch::sum(logMuRecX);
    elbo+=logLikelyHood+logPmu0+logPz0-(logEqmu+logEqtau+logEqz);

  }
  auto ru=-elbo;
  //std::cout<<"elbo="+to_string(ru.item<float>())+" ,mu="+to_string(irMu.item<float>())+ " ,tau="+to_string(irTau.item<float>())<<std::endl;
  auto sigma2=torch::reciprocal(irTau);
  auto tSum=ru+sigma2;
  return 2*sigma2*(-elbo)/tSum;
}
void TROCHPACK_SVI::LinearSVI::loadPriorDist(float pmu,float psigma)
{
  mu=torch::tensor(pmu);
  tau=torch::tensor(1/psigma);
  irMu=mu;
  irTau=tau;
}
void TROCHPACK_SVI::LinearSVI::updateEstimations() {
 mu=irMu;
 tau=irTau;
}
void TROCHPACK_SVI::LinearSVI::learnStep(torch::Tensor data) {
 //
  myOpt->zero_grad();
  auto mElboLoss= minusELBO(data);
  mElboLoss.backward({},true);
  myOpt->step();
  updateEstimations();
  //
}
void TROCHPACK_SVI::LinearSVI::resetLearningRate(double lr)
{
  myOpt->defaults()=torch::optim::AdamOptions(lr);
 // std::cout<<"z="<<latentZ<<std::endl;
}
void TROCHPACK_SVI::LinearSVI::runForward(torch::Tensor data)
{
  int64_t n=data.size(1);
  int64_t rows=data.size(0);
  torch::Tensor rowX ;
  //std::cout<<mu<<endl;
  auto tempMu=torch::zeros({rows,1});
  auto tempTau=torch::zeros({rows,1});
  for (int64_t i = 0; i < data.size(0); i++) {
    rowX=data[i];
    auto xz=torch::mul(latentZ,rowX);
    auto xzPmu=xz-mu;
    auto xzPmu2=torch::mul(xzPmu,xzPmu);
    irMu=(torch::sum(xz)+mu)/(1+n);
    tempMu[i][0]=irMu;
    irTau=(tau+n/2)*torch::reciprocal((1+0.5*torch::sum(xzPmu2)+torch::mul(irMu-mu,irMu-mu)));
    tempTau[i][0]=irTau;
  }
  auto tempSima=torch::sqrt(torch::reciprocal(tempTau));
  resultMu=torch::mean(tempMu).item<float>();
  resultSigma=torch::mean(tempSima).item<float>();

}
torch::Tensor TROCHPACK_SVI::LinearSVI::forwardMu(torch::Tensor data)
{   std::cout<<latentZ<<std::endl;
  int64_t n=data.size(1);
  auto zt=latentZ.t();
  auto ru=(torch::matmul(data,zt)+mu)/(1+n);
  return ru;
}
void  TROCHPACK_SVI::LinearSVI::pretrainStep(torch::Tensor data,torch::Tensor label)
{ myOpt->zero_grad();
   //std::cout<<label<<","<<data;
   auto muPredict= forwardMu(data);
   auto mse= torch::mse_loss(muPredict, label);
   mse.backward({},true);
   myOpt->step();
   updateEstimations();
   //std::cout<<"mse= "<<mse.sum().item<float>()<<std::endl;
}


