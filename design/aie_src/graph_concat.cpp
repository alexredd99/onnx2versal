#include "graph_concat.h"
#include "graph_utils.h"

#define ITER_CNT 1


template <template<int, int, int, int> class CONCAT,
  int LCNT, int WINDOW_SIZE, int CHUNK_SIZE, int BLOCK_SIZE>
class ConcatGraphTest : public adf::graph {

  private:
    ConcatGraph<CONCAT, LCNT, WINDOW_SIZE, CHUNK_SIZE, BLOCK_SIZE> g;

  public:
    adf::input_plio plin[NLANES];
    adf::output_plio plout[1];

    ConcatGraphTest(
      const std::string& id,
      const std::string& INP0_TXT,
      const std::string& INP1_TXT,
      const std::string& INP2_TXT,
      const std::string& INP3_TXT,
      const std::string& INP4_TXT,
      const std::string& INP5_TXT,
      const std::string& INP6_TXT,
      const std::string& INP7_TXT,
      const std::string& OUT_TXT = "concat_out.txt"
    ) {
      plin[0] = adf::input_plio::create("plin0_concat"+id+"_input", adf::plio_64_bits, TXT_ARG(INP0_TXT));
      plin[1] = adf::input_plio::create("plin1_concat"+id+"_input", adf::plio_64_bits, TXT_ARG(INP1_TXT));
      plin[2] = adf::input_plio::create("plin2_concat"+id+"_input", adf::plio_64_bits, TXT_ARG(INP2_TXT));
      plin[3] = adf::input_plio::create("plin3_concat"+id+"_input", adf::plio_64_bits, TXT_ARG(INP3_TXT));
      plin[4] = adf::input_plio::create("plin4_concat"+id+"_input", adf::plio_64_bits, TXT_ARG(INP4_TXT));
      plin[5] = adf::input_plio::create("plin5_concat"+id+"_input", adf::plio_64_bits, TXT_ARG(INP5_TXT));
      plin[6] = adf::input_plio::create("plin6_concat"+id+"_input", adf::plio_64_bits, TXT_ARG(INP6_TXT));
      plin[7] = adf::input_plio::create("plin7_concat"+id+"_input", adf::plio_64_bits, TXT_ARG(INP7_TXT));
      plout[0] = adf::output_plio::create("plout0_concat"+id+"_output", adf::plio_64_bits, TXT_ARG(OUT_TXT));

      for (int i = 0; i < NLANES; i++) {  
        if (LCNT > i) {
          adf::connect<adf::window<WINDOW_SIZE*4>> (plin[i].out[0], g.pin[i]);
        } else {
          adf::connect<adf::window<4>> (plin[i].out[0], g.pin[i]);
        }
      }
      // BLOCK_SIZE <= WINDOW_SIZE
      adf::connect<adf::window<WINDOW_SIZE/CHUNK_SIZE*BLOCK_SIZE*4>> (g.pout[0], plout[0].in[0]);
    }

};


// instance to be compiled and used in host within xclbin
ConcatGraphTest<ConcatScalar, 5, 8, 8, 4*8+4> fpscalar1("fpscalar1",
  "concat_fpin.txt", "concat_fpin.txt",
  "concat_fpin.txt", "concat_fpin.txt",
  "concat_fpin.txt", "empty.txt",
  "empty.txt", "empty.txt",
  "concat_fpout1_ConcatScalar.txt"
);

ConcatGraphTest<ConcatScalar, 5, 8, 4, 4*4+2> fpscalar2("fpscalar2",
  "concat_fpin.txt", "concat_fpin.txt",
  "concat_fpin.txt", "concat_fpin.txt",
  "concat_fpin.txt", "empty.txt",
  "empty.txt", "empty.txt",
  "concat_fpout2_ConcatScalar.txt"
);


#ifdef __X86SIM__
int main(int argc, char ** argv) {
  adfCheck(fpscalar1.init(), "init fpscalar1");
  adfCheck(fpscalar1.run(1), "run fpscalar1");
	adfCheck(fpscalar1.end(), "end fpscalar1");

  adfCheck(fpscalar2.init(), "init fpscalar2");
  adfCheck(fpscalar2.run(1), "run fpscalar2");
	adfCheck(fpscalar2.end(), "end fpscalar2");
  return 0;
}
#endif


#ifdef __AIESIM__
int main(int argc, char ** argv) {
	adfCheck(fpscalar1.init(), "init fpscalar1");
  get_graph_throughput_by_port(fpscalar1, "plout[0]", fpscalar1.plout[0], 36, sizeof(float), ITER_CNT);
	adfCheck(fpscalar1.end(), "end fpscalar1");

  adfCheck(fpscalar2.init(), "init fpscalar2");
  get_graph_throughput_by_port(fpscalar2, "plout[0]", fpscalar2.plout[0], 36, sizeof(float), ITER_CNT);
	adfCheck(fpscalar2.end(), "end fpscalar2");
  return 0;
}
#endif
