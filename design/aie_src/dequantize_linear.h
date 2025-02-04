#ifndef DEQUANTIZE_LINEAR_H
#define DEQUANTIZE_LINEAR_H

#include <adf.h>


/** 
 * @defgroup DequantizeLinearKernels
 * @ingroup DequantizeLinear
 * 
 * https://github.com/onnx/onnx/blob/main/docs/Operators.md#DequantizeLinear
 * y = (x - x_zero_point) * x_scale
 * 
 * @{
 */


/**
 * @brief Scalar implementation,
 * DequantizeLinearScalar<96,84> takes 296 cycles
 */
template <typename TT, int B, int INP_W, int OUT_W>
class DequantizeLinearScalar {
  
  private:
    float scale;
    TT zero; // same type as output
	
  public:
    DequantizeLinearScalar (
      float scale,
      TT zero
    ): scale(scale), zero(zero) {};

		void filter(
			input_window<TT>* in,
			output_window<float>* out
		);

		static void registerKernelClass() {
			REGISTER_FUNCTION(DequantizeLinearScalar::filter);
		}
};


/**
 * @brief Vector implementation,
 * DequantizeLinear<96,84> takes 154 cycles
 */
template <typename TT, int B, int INP_W, int OUT_W>
class DequantizeLinear {
  
  private:
    float scale;
    TT zero; // same type as output

    // precompute
    int32_t iscale;
    int32_t ishift;
    int bitshift;
	
  public:
    DequantizeLinear (
      float scale,
      TT zero
    );

		void filter(
			input_window<TT>* in,
			output_window<float>* out
		);

		static void registerKernelClass() {
			REGISTER_FUNCTION(DequantizeLinear::filter);
		}
};
/** @}*/


#endif // DEQUANTIZE_LINEAR_H
