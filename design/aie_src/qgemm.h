#ifndef QGEMM_H_
#define QGEMM_H_

#include <adf.h>
#include <assert.h>


/** 
 * @defgroup QgemmKernels
 * @ingroup Qgemm
 * - y = saturate ((x / y_scale) + y_zero_point)
 * - Bias must be quantized using scale = x_scale * w_scale and zero_point = 0
 * 
 * Computation
 * - x    = (qx - qx_zero) * qx_scale
 * - bias = qbias * x_scale * w_scale
 * - y    = x*w + bias
 * 
 * (qy-qy_zero)*qy_scale 
 * - = (qx-qx_zero)*qx_scale * (qw-qw_zero)*qw_scale + qbias*qx_scale*qw_scale
 * - = [(qx-qx_zero) * (qw-qw_zero) + qbias] * qx_scale*qw_scale
 * 
 * @{
 */

/**
 * @brief Scalar implementation for MK*KN, stores weights and biases,
 * QgemmScalar<a,a,1,80,32> takes 17340 cycles
 */
template <typename TT, typename TTPARAM, int M, int K, int N>
class QgemmScalar {
  
  private:
    alignas(32) TTPARAM (&weights)[N*K]; // KxN (256x120)
    alignas(32) int32_t (&bias)[N];     // N   (120)
    float x_scale;
    float w_scale;
    float y_scale;
    TT x_zero;
    TTPARAM w_zero;
    TT y_zero;

    float scale;
    alignas(32) TT in_row[K];
  
  public:
    QgemmScalar (
      TTPARAM (&w)[K*N],
      int32_t (&b)[N],
      float x_scale,
      float w_scale,
      float y_scale,
      TT x_zero,
      TTPARAM w_zero,
      TT y_zero
    ): weights(w), bias(b), x_scale(x_scale), w_scale(w_scale), y_scale(y_scale), x_zero(x_zero), w_zero(w_zero), y_zero(y_zero) {
      scale = x_scale*w_scale/y_scale;
    };

    void filter(
      input_stream<TT>* in,   // MxK  (1x256)
      output_stream<TT>* out  // MxN  (1x120)
    );
    
    static void registerKernelClass() {
      static_assert((std::is_same<TT, int8_t>::value) || (std::is_same<TT, uint8_t>::value));
      REGISTER_FUNCTION(QgemmScalar::filter);
      REGISTER_PARAMETER(weights);
      REGISTER_PARAMETER(bias);
    };
};


/**
 * @brief Vector implementation for MK*KN, stores weights and biases, requires N%16=0
 * Qgemm<a,a,1,80,32> takes 227 cycles
 */
template <typename TT, typename TTPARAM, int M, int K, int N>
class Qgemm {
  
  private:
    alignas(32) TTPARAM (&weights)[N*K]; // KxN (256x120)
    alignas(32) int32_t (&bias)[N];     // N   (120)
    float x_scale;
    float w_scale;
    float y_scale;
    TT x_zero;
    TTPARAM w_zero;
    TT y_zero;

    // precomputation
    int scalebits;
    int16_t scale;
    int32_t shift;

    alignas(32) TT in_row[K];

  public:
    Qgemm (
      TTPARAM (&w)[K*N],
      int32_t (&b)[N],
      float x_scale,
      float w_scale,
      float y_scale,
      TT x_zero,
      TTPARAM w_zero,
      TT y_zero
    );

    void filter(
      input_stream<TT>* in,   // MxK
      output_stream<TT>* out  // MxN
    );
    
    static void registerKernelClass() {
      static_assert((std::is_same<TT, int8_t>::value) || (std::is_same<TT, uint8_t>::value));
      static_assert(K % 16 == 0);
      static_assert(N % 16 == 0);
      REGISTER_FUNCTION(Qgemm::filter);
      REGISTER_PARAMETER(weights);
      REGISTER_PARAMETER(bias);
    };
};


/**
 * @brief Vector implementation for MK*KN, stores weights and biases, requires N%16=0
 * Qgemm<a,a,1,80,32> takes cycles
 */
template <typename TT, typename TTPARAM, int M, int K, int N>
class QgemmStream {
  
  private:
    alignas(32) int32_t (&bias)[N];     // N   (120)
    float x_scale;
    float w_scale;
    float y_scale;
    TT x_zero;
    TTPARAM w_zero;
    TT y_zero;

    // precomputation
    int scalebits;
    int16_t scale;
    int32_t shift;

    alignas(32) TT in_row[K];

  public:
    QgemmStream (
      int32_t (&b)[N],
      float x_scale,
      float w_scale,
      float y_scale,
      TT x_zero,
      TTPARAM w_zero,
      TT y_zero
    );

    void filter(
      input_stream<TT>* in,          // MxK
      input_stream<TTPARAM>* weight, // KxN
      output_stream<TT>* out         // MxN
    );
    
    static void registerKernelClass() {
      static_assert((std::is_same<TT, int8_t>::value) || (std::is_same<TT, uint8_t>::value));
      static_assert((std::is_same<TTPARAM, int8_t>::value) || (std::is_same<TTPARAM, uint8_t>::value));
      static_assert(K % 16 == 0);
      static_assert(N % 16 == 0);
      REGISTER_FUNCTION(QgemmStream::filter);
      REGISTER_PARAMETER(bias);
    };
};
/** @}*/


#endif // QGEMM_H_
