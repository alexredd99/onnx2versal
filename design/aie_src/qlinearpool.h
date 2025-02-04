#ifndef QLINEARPOOL_H_
#define QLINEARPOOL_H_

#include <adf.h>
#include <assert.h>


/** 
 * @defgroup Pool2DKernels
 * @ingroup Pool2D
 * 
 * @details
 * Design Notes
 * - Bandwidth limited
 * - 2 accs causes spilling
 * - All kernels assume INP_W divisible by OUT_W
 * 
 * @{
 */

/**
 * @brief Scalar implementation for BCHW avgpool,
 * requires B*C*OUT_H*OUT_W % 16 == 0
 * QLinearAvgpoolScalarBCHW::filter<24,32,16,1,6> total = 17365 (output_window 15766)
 */
template <typename TT, int INP_H, int INP_W, int OUT_H, int OUT_W, int B, int C, int KH, int KW>
class QLinearAvgpoolScalarBCHW {
  private:
    float in_scale;
    float out_scale;
    int8_t in_zero;
    int8_t out_zero;
    
  public:
    QLinearAvgpoolScalarBCHW (
      float in_scale,
      float out_scale,
      int8_t in_zero,
      int8_t out_zero
    ): in_scale(in_scale), out_scale(out_scale), in_zero(in_zero), out_zero(out_zero) {}

    void filter(
      input_window<TT>* in,
      output_stream<TT>* out
    );
    static void registerKernelClass() {
      REGISTER_FUNCTION(QLinearAvgpoolScalarBCHW::filter);
    }
};


/**
 * @brief Scalar implementation for BCHW avgpool,
 * QLinearGlobalAvgpoolScalarBCHW<7,16,1,1,1,2048,7,7> start = 651,end = 132051,total = 131400
 */
template <typename TT, int INP_H, int INP_W, int OUT_H, int OUT_W, int B, int C, int KH, int KW>
class QLinearGlobalAvgpoolScalarBCHW {
  private:
    float in_scale;
    float out_scale;
    int8_t in_zero;
    int8_t out_zero;

    // precomputation
    int BITSHIFT;
    int16_t scale;
    
  public:
    QLinearGlobalAvgpoolScalarBCHW (
      float in_scale,
      float out_scale,
      int8_t in_zero,
      int8_t out_zero
    );

    void filter(
      input_stream<TT>* in,
      output_stream<TT>* out
    );
    static void registerKernelClass() {
      static_assert(C % 16 == 0);
      static_assert(KW <= 16);
      REGISTER_FUNCTION(QLinearGlobalAvgpoolScalarBCHW::filter);
    }
};
/** @}*/


#endif // QLINEARPOOL_H_
