#ifndef __DEQUANTIZE_LINEAR_GRAPH_H__
#define __DEQUANTIZE_LINEAR_GRAPH_H__

#include <adf.h>
#include "dequantize_linear.h"


/**
 * @defgroup DequantizeLinear
 * 
 * @brief Linear dequantization operator. It consumes a quantized tensor, a scale, and 
 * a zero point to compute the full precision tensor. The dequantization formula is 
 * y = (x - x_zero_point) * x_scale. x_scale and x_zero_point must have same shape, 
 * and can be either a scalar for per-tensor / per layer quantization, or a 1-D tensor 
 * for per-axis quantization. x_zero_point and x must have same type. x and y must 
 * have same shape.
 * 
 * @tparam DEQUANTIZE_LINEAR  DequantizeLinear Kernel
 * @tparam INP_SIZE           input size
 * @tparam OUT_SIZE           output size
 * 
 * @{
 */

/**
 * @brief Single instance graph
 * 
 * @connections
 * @connect{pin[0], INP_SIZE}
 * @connect{pout[0], OUT_SIZE*4}
 * @endconnections
 */
template <template<int, int> class DEQUANTIZE_LINEAR, int INP_SIZE, int OUT_SIZE>
class DequantizeLinearGraph : public adf::graph {

  private:
    adf::kernel k[1];
    std::string id;

  public:
    adf::port<input> pin[1];
    adf::port<output> pout[1];

    DequantizeLinearGraph(
      float scale,
      int8_t zero
    ) { 
      k[0] = adf::kernel::create_object<DEQUANTIZE_LINEAR<INP_SIZE, OUT_SIZE>>(scale, zero);
      adf::source(k[0]) = "dequantize_linear.cc";
      adf::headers(k[0]) = {"dequantize_linear.h"};
      adf::runtime<ratio>(k[0]) = 0.6;
      
      adf::connect<adf::window<INP_SIZE>> (pin[0], k[0].in[0]);
      adf::connect<adf::window<OUT_SIZE*4>> (k[0].out[0], pout[0]);
    }

};
/** @} */


#endif // __DEQUANTIZE_LINEAR_GRAPH_H__