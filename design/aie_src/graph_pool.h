#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <adf.h>
#include "pool.h"


template <int INP_W, int OUT_W, int B, int C, const char* INP_TXT, const char* OUT_TXT>
class MaxpoolScalar : public adf::graph {

  private:
    adf::kernel k[1];
    std::string id;

  public:
    adf::input_plio plin[1];
    adf::output_plio plout[1];

    MaxpoolScalar(const std::string& id) { 
      this->id = id;

      k[0] = adf::kernel::create(maxpool_scalar<INP_W, OUT_W, B, C>);
      adf::source(k[0]) = "pool.cc";

#ifdef EXTERNAL_IO
      plin[0] = adf::input_plio::create("plin0_maxpool"+id+"_input", adf::plio_64_bits);
      plout[0] = adf::output_plio::create("plout0_maxpool"+id+"_output", adf::plio_64_bits);
#else
      plin[0] = adf::input_plio::create("plin0_maxpool"+id+"_input", adf::plio_64_bits, INP_TXT);
      plout[0] = adf::output_plio::create("plout0_maxpool"+id+"_output", adf::plio_64_bits, OUT_TXT);
#endif
      
      adf::connect<adf::window<B*INP_W*INP_W*C*4>> (plin[0].out[0], k[0].in[0]);
      adf::connect<adf::window<B*OUT_W*OUT_W*C*4>> (k[0].out[0], plout[0].in[0]);

      adf::runtime<ratio>(k[0]) = 0.6;
    }

};


#endif // __GRAPH_H__