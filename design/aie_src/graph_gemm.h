#ifndef __GEMM_GRAPH_H_
#define __GEMM_GRAPH_H_

#include <adf.h>
#include "gemm.h"
#include "concat.h"

#define CHUNK_COUNT   N / NCHUNK + 1
#define NCUTCHUNK     N % NCHUNK
#define CONCAT_NLANES 8


/*
Chunks NxK weight params into ~16384B chunks by N dimension
Assumes weight <=16384B, bias <=4096B, input <=4096B per chunk
Places a maximum of 3x3 tiles, 8 gemm tiles surrounding a concat tile (max AIE DMA input = 8)
*/
template <template<int, int, int> class GEMM, int NCHUNK, int M, int K, int N>
class GemmReluChunkGraph : public adf::graph {

  private:
    adf::kernel gemms[CHUNK_COUNT];
    adf::kernel concat;
    adf::relative_coordinate tileOffsets[8] = {
      {.col_offset = -1, .row_offset = -1},
      {.col_offset = 0, .row_offset = -1},
      {.col_offset = 1, .row_offset = -1},
      {.col_offset = -1, .row_offset = 0},
      {.col_offset = 1, .row_offset = 0},
      {.col_offset = -1, .row_offset = 1},
      {.col_offset = 0, .row_offset = 1},
      {.col_offset = 1, .row_offset = 1},
    };

  public:
    adf::port<input> pin[CONCAT_NLANES];
    adf::port<output> pout[1];

    // constructor fails with INTERNAL ERROR: 'Linting::CheckKernel::check': the number of argument types and the number of ports are inconsistent
    void construct(
      std::vector<float> weights,
      std::vector<float> bias
    ) { 

      concat = adf::kernel::create_object<ConcatScalar<CHUNK_COUNT, NCHUNK, NCHUNK, N>>();
      adf::source(concat) = "concat.cc";
      adf::runtime<ratio>(concat) = 0.6;

      std::vector<float> wChunk;
      std::vector<float> bChunk;

      for (int i = 0; i < CHUNK_COUNT; i++) {
        int chunkSize = (i*NCHUNK + NCHUNK > N) ? NCUTCHUNK : NCHUNK;
        wChunk = std::vector<float>(weights.begin()+i*NCHUNK*K, weights.begin()+(i*NCHUNK+chunkSize)*K); 
        wChunk.resize(NCHUNK*K, 0);
        bChunk = std::vector<float>(bias.begin()+i*NCHUNK, bias.begin()+i*NCHUNK+chunkSize);
        bChunk.resize(NCHUNK, 0);
        gemms[i] = adf::kernel::create_object<GEMM<M, K, NCHUNK>>(wChunk, bChunk);
        adf::source(gemms[i]) = "gemm.cc";
        adf::runtime<ratio>(gemms[i]) = 0.6;

        adf::location<adf::kernel>(gemms[i]) = adf::location<adf::kernel>(concat) + 
          adf::relative_offset(tileOffsets[i]);
        adf::location_constraint tilePos = adf::location<adf::kernel>(gemms[i]);
        adf::location<adf::parameter>(gemms[i].param[0]) = tilePos; // weight (<= 16384B)
        adf::location<adf::parameter>(gemms[i].param[0]) = adf::offset(0x0000);
        adf::location<adf::parameter>(gemms[i].param[1]) = tilePos; // bias   (<= 4096B)
        adf::location<adf::parameter>(gemms[i].param[1]) = adf::offset(0x4000); 
        adf::location<adf::buffer>(gemms[i].in[0]) = tilePos;  // input window (<= 4096B)
        adf::location<adf::buffer>(gemms[i].in[0]) = {adf::offset(0x5000), adf::offset(0x6000)};
      }

      for (int i = 0; i < CONCAT_NLANES; i++) {
        if (i < CHUNK_COUNT) {
          adf::connect<adf::window<M*K*4>> (pin[i], gemms[i].in[0]);
          adf::connect<adf::window<M*NCHUNK*4>> (gemms[i].out[0], concat.in[i]);
        } else {
          adf::connect<adf::window<4>> (pin[i], concat.in[i]);
        }
      }

      adf::connect<adf::window<M*N*4>> (concat.out[0], pout[0]);
    }

};


#endif // __GEMM_GRAPH_H_