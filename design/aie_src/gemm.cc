#include "gemm.h"
#include "kernel_utils.h"


/*
xA^T + b as per torch,nn.Linear
8581 cycles for lenet fc1 (broke in 8 sections)
4488 cycles for lenet fc2 (broke in 4 sections)
1956 cycles for lenet fc3
MxK * NxK
weights[N*K] (120x256)
bias[N]      (120)
*/
template <int M, int K, int NCHUNK>
void GemmReluScalarMKNK<M, K, NCHUNK>::filter(
	input_window<float>* in,      // MxK  (1x256)
  output_window<float>* out     // MxN  (1x120)
) {
  PROFILE_HEADER(printf(
    "Running GemmReluScalarMKNK<%d, %d, %d>\n", M, K, NCHUNK));

  int weightIdx = 0;

  for (int i = 0; i < M; i++) {
    for (int j = 0; j < NCHUNK; j++) {
      float res = bias[j];
      for (int k = 0; k < K; k++) {
        float a = window_readincr(in);
        float b = weights[weightIdx];
        weightIdx++;
        res += a * b;
      }    
      
      if (res < 0) res = 0;
      window_writeincr(out, res);
      window_incr(in, -K); // repeat same in row for next j
    }
    window_incr(in, K); // next in row for next NCHUNK
  }

  PROFILE_FOOTER;
}


template <int M, int K, int NCHUNK>
void GemmReluScalarMKKN<M, K, NCHUNK>::filter(
	input_window<float>* in,      // MxK  (1x256)   inputs
                                // KxN  (256x120) weights
  output_window<float>* out     // MxN  (1x120)   outputs
) {
  PROFILE_HEADER(printf(
    "Running GemmReluScalarMKKN<%d, %d, %d>\n", M, K, NCHUNK));

  int weightIdx = 0;

  for (int i = 0; i < M; i++) {
    for (int j = 0; j < NCHUNK; j++) {
      float res = bias[j];
      weightIdx = j;
      for (int k = 0; k < K; k++) {
        float a = window_readincr(in);
        float b = weights[weightIdx];
        weightIdx += NCHUNK;
        res += a * b;
      }    
      if (res < 0) res = 0;
      window_writeincr(out, res);
      window_incr(in, -K); // repeat same in row for next j
    }
    window_incr(in, K); // next in row for next NCHUNK
  }

  PROFILE_FOOTER;
}
