/*! \file   AIOperator.h
.h*/
//
// Created by tony on 03/12/22.
//

#ifndef INTELLISTREAM_INCLUDE_OPERATOR_MEANAQPAIOperator_H_
#define INTELLISTREAM_INCLUDE_OPERATOR_MEANAQPAIOperator_H_

#include <Operator/MeanAQPIAWJOperator.h>
#include <Common/Window.h>
#include <atomic>
#include <WaterMarker/LatenessWM.h>
#include <Common/StateOfKey.h>
#include <Common/LinearVAE.h>
#include <optional>
#include <filesystem>
using std::nullopt;
namespace OoOJoin {

/**
 * @ingroup ADB_OPERATORS
 * @class AIOperator Operator/AIOperator.h
 * @brief The IAWJ operator working under AI and use variational inference to proactively capture the unobserved
 * @note require configurations:
 * - "windowLen" U64: The length of window
 * - "slideLen" U64: The length of slide
 * - "sLen" U64: The length of S buffer
 * - "rLen" U64: The length of R buffer
 * - "wmTag" String: The tag of watermarker, default is arrival for @ref ArrivalWM
 * - "aiMode" String: The tag to indicate working mode of ai, can be pretrain (0), continual_learning (1) or inference (2)
 * = "ptPrefix" String: The prefix of vae *.pt, such as linearVAE
 * - "appendSel", U64, whether append sel observations to stored tensor, 0
 * - "appendSkew", U64, whether append skew (of r and s) observations to stored tensor, 0
 * - "appendRate", U64, whether append rate (of r and s) observations to stored tensor, 0
 * - "exitAfterPretrain", U64, whether exit after a pretrain, U64,1
 * @warning This implementation is putting rotten, just to explore a basic idea of AQP by using historical mean to predict future
 * @warning The predictor and watermarker are currently NOT seperated in this operator, split them in the future!
 * @note In current version, the computation will block feeding
 * @note follows the assumption of linear independent arrival and skewness
 * @note operator tag = "AI"
 */
class AIOperator : public MeanAQPIAWJOperator {
 protected:
  void conductComputation();
  std::string aiMode;
  uint8_t aiModeEnum = 0;
  uint64_t appendSel = 0, appendSkew = 0, appendRate, exitAfterPretrain;
  std::string ptPrefix;
  /**
   * @brief The pre-allocated length of seletivity observations, only valid for pretrain
   */
  uint64_t selLen = 0;
  /**
   * @class ObservationGroup Operator/AIOperator.h
   * @brief THe class of tensors, buffere management to keep observations
   */
  class ObservationGroup {
   public:
    ObservationGroup() = default;
    ~ObservationGroup() = default;
    float finalObservation = 0.0;
    float scalingFactor = 0.0;
    uint64_t xCols, xRows = 0;
    /**
     * @brief xTensor is the observation, yTensor is the label.
     */
    torch::Tensor xTensor, yTensor;
    uint64_t bufferLen = 0, observationCnt = 0;
    void initObservationBuffer(uint64_t _bufferLen) {
      bufferLen = _bufferLen;
      xTensor = torch::zeros({1, (long) bufferLen});
    }
    /**
     * @brief to append a new observation to tensor X
     * @param newX
     */
    void appendX(float newX) {
      if (observationCnt >= bufferLen) {
        observationCnt = 0;
        xRows++;
      }
      xTensor[0][observationCnt] = newX;
      observationCnt++;
    }
    /**
     * @brief narrow down the xTensor and ignore all unused elements, and then reshape x to specific number of cols
     */
    void narrowAndReshapeX(uint64_t _xCols) {
      xCols = _xCols;
      //auto b=a.reshape({1,(long)(rows*cols)});
      uint64_t elementsSelected = observationCnt / xCols;
      elementsSelected = elementsSelected * xCols;
      auto b = xTensor.narrow(1, 0, elementsSelected);
      xRows = elementsSelected / xCols;
      b = b.reshape({(long) (elementsSelected / xCols), (long) xCols});
      xTensor = b;
    }
    /**
     * @brief To normalize the x and y tensor
     */
    void normalizeXY() {
      xTensor = xTensor / scalingFactor;
      yTensor = yTensor / scalingFactor;
    }
    /**
     * @brief Generate the Y tensor as labels
     * @param yLabel the label
     */
    void generateY(float yLabel) {
      yTensor = torch::ones({(long) (observationCnt / xCols), 1})
          * yLabel;
    }
    void setFinalObservation(float obs) {
      finalObservation = obs;

    }
    torch::Tensor tryTensor(std::string fileName) {
      if (std::filesystem::exists(fileName)) {
        try {
          torch::Tensor load_tensor;
          // Load the tensor from the file
          torch::load(load_tensor, fileName);
          // If we get here, the file was loaded successfully
          return load_tensor;

        } catch (const std::runtime_error &error) {
          // Handle the error
          //  std::cerr << "Error loading tensor: " << error.what() << std::endl;
          INTELLI_ERROR("the tensor can not be loaded");
          // Return an error code
          return torch::empty({1, 0});;
        }
      }
      INTELLI_WARNING("the tensor DOES NOT exist");
      return torch::empty({1, 0});;
    }
    torch::Tensor saveTensor2File(torch::Tensor ts, std::string ptName) {
      auto oldSelectivityTensorX = tryTensor(ptName);
      torch::Tensor ru;
      if (oldSelectivityTensorX.size(1) != 0) {
        ru = torch::cat({oldSelectivityTensorX, ts}, /*dim=*/0);
      } else {
        ru = ts;
      }
      torch::save({ru}, ptName);
      return ru;
    }
    float tryScalingFactor(std::string ptPrefix) {
      scalingFactor = 1.0;
      auto ts = tryTensor(ptPrefix + "_s.pt");
      if (ts.size(1) != 0) {
        scalingFactor = ts[0][0].item<float>();
      }
      INTELLI_INFO("scaling factor is read as " + to_string(scalingFactor));
      return scalingFactor;
    }
    void tryAndSaveScalingFactor(std::string ptPrefix) {
      auto ts = tryTensor(ptPrefix + "_s.pt");
      if (ts.size(1) != 0) {
        scalingFactor = ts[0][0].item<float>();
        INTELLI_INFO("READ scaling factor as " + to_string(scalingFactor));
      } else {
        // Get the size of tensor A
        //torch::IntArrayRef size = xTensor.sizes();
        int numElements = torch::numel(xTensor);
        float sum = torch::sum(xTensor).item<float>(); // Calculate the sum of all elements in the tensor
        scalingFactor = sum / numElements;
        auto saveScaling = torch::ones({1, 1}) * scalingFactor;
        torch::save({saveScaling}, ptPrefix + "_s.pt");
        INTELLI_WARNING("create scaling factor " + to_string(scalingFactor));
      }

    }

    void saveXYTensors2Files(std::string ptPrefix, uint64_t _xCols) {
      /**
       * @brief 1. generate x
       */
      narrowAndReshapeX(_xCols);

      /**
       * @brief 2. generate y
       *
       */
      generateY(finalObservation);
      /**
       * @brief 3. normalize x and y
       */
      tryAndSaveScalingFactor(ptPrefix);
      normalizeXY();
      /***
       * @brief 4. sava x and y
       */
      auto tx = saveTensor2File(xTensor, ptPrefix + "_x.pt");

      uint64_t xRows = tx.size(0);
      uint64_t xCols = tx.size(1);
      INTELLI_INFO(
          "Now we have [" + to_string(xRows) + "x" + to_string(xCols) + "] at " + ptPrefix + "_x.pt");
      auto ty = saveTensor2File(yTensor, ptPrefix + "_y.pt");
      uint64_t yRows = ty.size(0);
      uint64_t yCols = ty.size(1);
      INTELLI_INFO(
          "Now we have [" + to_string(yRows) + "x" + to_string(yCols) + "] at " + ptPrefix + "_y.pt");
    }

  };

  class AIStateOfKey : public MeanStateOfKey {
   public:
    double lastUnarrivedTuples = 0;

    AIStateOfKey() = default;

    ~AIStateOfKey() = default;
  };
  class AIStateOfStreams {
   public:
    uint64_t sCnt = 0, rCnt = 0;
    /**
     * @brief estimate and track the selectivity
     */
    ObservationGroup selObservations;
    /**
     * @brief estimate and track the skew of s and r
     */
    ObservationGroup sSkewObservations, rSkewObservations;
    /**
     * @brief estimate and track the rate of s and r
     */
    ObservationGroup sRateObservations, rRateObservations;
    double selectivity = 0.0;

    uint64_t sEventTime = 0, rEventTime = 0;
    double sRate = 0, rRate = 0;
    double sSkew = 0, rSkew = 0;
    void updateSelectivity(uint64_t joinResults) {
      double crossCnt = rCnt * sCnt;
      selectivity = joinResults / crossCnt;

    }
    void encounterSTuple(TrackTuplePtr ts) {
      sCnt++;
      if (ts->eventTime > sEventTime) {
        sEventTime = ts->eventTime;
      }
      sRate = sCnt;
      sRate = sRate * 1e3 / sEventTime;
      sSkew = (sSkew * (sCnt - 1) + ts->arrivalTime - ts->eventTime) / sCnt;
      //sSkewObservations.appendX(sSkew);
      // sRateObservations.appendX(sRate);
    }
    void encounterRTuple(TrackTuplePtr tr) {
      rCnt++;
      if (tr->eventTime > rEventTime) {
        rEventTime = tr->eventTime;
      }
      rRate = rCnt;
      rRate = rRate * 1e3 / rEventTime;
      rSkew = (rSkew * (rCnt - 1) + tr->arrivalTime - tr->eventTime) / rCnt;
      // rSkewObservations.appendX(rSkew);
      // rRateObservations.appendX(rRate);
    }
    void reset() {
      sCnt = 0;
      rCnt = 0;
      selectivity = 0.0;
      sEventTime = 0;
      rEventTime = 0;
      sRate = 0;
      rRate = 0;
      sSkew = 0;
      rSkew = 0;
    }
    std::string reportStr() {
      std::string ru = "sRate," + to_string(sRate) + "\r\n";
      ru += "rRate," + to_string(rRate) + "\r\n";
      ru += "sSkew," + to_string(sSkew) + "\r\n";
      ru += "rSkew," + to_string(rSkew) + "\r\n";
      ru += "selectivity," + to_string(selectivity) + "\r\n";
      ru += "sCnt," + to_string(sCnt) + "\r\n";
      ru += "rCnt," + to_string(rCnt) + "\r\n";
      return ru;
    }
    TROCHPACK_VAE::LinearVAE vaeSelectivity, vaeSRate, vaeRRate;
    AIStateOfStreams() = default;
    ~AIStateOfStreams() = default;
  };
  AIStateOfStreams streamStatisics;
#define newAIStateOfKey std::make_shared<AIStateOfKey>
  using AIStateOfKeyPtr = std::shared_ptr<AIStateOfKey>;
  /**
   * @brief inline function to be called at the end of window
   */
  void endOfWindow();
  /**
   * @brief to prepare data structures for a pretrain
   */
  void preparePretrain();
  /**
 * @brief to prepare data structures for a inference
 */
  void prepareInference();
  /**
   * @brief save all tensors to file
   */
  void saveAllTensors();

 public:
  AIOperator() = default;

  ~AIOperator() = default;

  /**
   *
  * @brief Set the config map related to this operator
  * @param cfg The config map
   * @return bool whether the config is successfully set
  */
  bool setConfig(ConfigMapPtr cfg) override;

  /**
 * @brief feed a tuple s into the Operator
 * @param ts The tuple
  * @warning The current version is simplified and assuming only used in SINGLE THREAD!
  * @return bool, whether tuple is fed.
 */
  bool feedTupleS(TrackTuplePtr ts) override;

  /**
    * @brief feed a tuple R into the Operator
    * @param tr The tuple
    * @warning The current version is simplified and assuming only used in SINGLE THREAD!
    *  @return bool, whether tuple is fed.
    */
  bool feedTupleR(TrackTuplePtr tr) override;

  /**
   * @brief start this operator
   * @return bool, whether start successfully
   */
  bool start() override;

  /**
   * @brief stop this operator
   * @return bool, whether start successfully
   */
  bool stop() override;

  /**
   * @brief get the joined sum result
   * @return The result
   */
  size_t getResult() override;

  /**
   * @brief get the joined sum result under AQP
   * @return The result
   */
  size_t getAQPResult() override;

};

/**
 * @ingroup ADB_OPERATORS
 * @typedef AIOperatorPtr
 * @brief The class to describe a shared pointer to @ref AIOperator

 */
typedef std::shared_ptr<class AIOperator> AIOperatorPtr;
/**
 * @ingroup ADB_OPERATORS
 * @def newAIOperator

 * @brief (Macro) To creat a new @ref AIOperator
 under shared pointer.
 */
#define newAIOperator std::make_shared<OoOJoin::AIOperator>

}
#endif //INTELLISTREAM_INCLUDE_OPERATOR_MEANAQPAIOperator_H_
