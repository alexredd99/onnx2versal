#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <adf.h>
#include "conv.h"


template <int INP_W, int OUT_W, int B, int C, int M, int K, 
  const char* INP_TXT, const char* WEIGHT_TXT, const char* BIAS_TXT, const char* OUT_TXT>
class ConvReluScalar : public adf::graph {

  private:
    adf::kernel k[1];
    std::string id;

  public:
    adf::input_plio plin[3];
    adf::output_plio plout[1];

    ConvReluScalar(const std::string& id) { 
      this->id = id;

      k[0] = adf::kernel::create(conv_relu_scalar<INP_W, OUT_W, B, C, M, K>);
      adf::source(k[0]) = "conv.cc";

#ifdef EXTERNAL_IO
      plin[0] = adf::input_plio::create("plin0_conv"+id+"_input", adf::plio_64_bits);
      plin[1] = adf::input_plio::create("plin1_conv"+id+"_weight", adf::plio_64_bits);
      plin[2] = adf::input_plio::create("plin2_conv"+id+"_bias", adf::plio_64_bits);
      plout[0] = adf::output_plio::create("plout0_conv"+id+"_output", adf::plio_64_bits);
#else
      plin[0] = adf::input_plio::create("plin0_conv"+id+"_input", adf::plio_64_bits, INP_TXT);
      plin[1] = adf::input_plio::create("plin1_conv"+id+"_weight", adf::plio_64_bits, WEIGHT_TXT);
      plin[2] = adf::input_plio::create("plin2_conv"+id+"_bias", adf::plio_64_bits, BIAS_TXT);
      plout[0] = adf::output_plio::create("plout0_conv"+id+"_output", adf::plio_64_bits, OUT_TXT);
#endif
      
      adf::connect<adf::window<B*INP_W*INP_W*C*4>> (plin[0].out[0], k[0].in[0]);
      adf::connect<adf::window<M*K*K*C*4>> (plin[1].out[0], k[0].in[1]);
      adf::connect<adf::window<M*4>> (plin[2].out[0], k[0].in[2]);
      adf::connect<adf::window<B*OUT_W*OUT_W*M*4>> (k[0].out[0], plout[0].in[0]);

      adf::runtime<ratio>(k[0]) = 0.6;
    }

};


#endif // __GRAPH_H__