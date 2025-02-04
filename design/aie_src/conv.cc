#include <limits>

#include "conv.h"
#include "kernel_utils.h"
#include "aie_api/aie.hpp"


#define CONV_PROFILE_FOOTER(filter_name) \
  PROFILE_FOOTER2("%s<%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d>", \
    filter_name, INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W, B, C, M, KH, KW, GROUP, IS_RELU);


template <int INP_H, int INP_W, int OUT_W, int OUT_W_PAD, int STEP_H, int STEP_W, 
          int B, int C, int M, int KH, int KW, int GROUP, int IS_RELU>
void ConvReluScalar<INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W, B, C, M, KH, KW, GROUP, IS_RELU>::filter(
	input_window<float>* in,      // BCHW (1x1x28x28)
  output_window<float>* out     // BMHW (1x6x24x24)
) {
  PROFILE_HEADER2;

  int weightIdx = 0;

  // BHWM
  for (int b = 0; b < B; b++) chess_prepare_for_pipelining chess_loop_range(B,) {
    for (int m = 0; m < M; m++) chess_prepare_for_pipelining chess_loop_range(M,) { 
      
      for (int h = 0; h < OUT_H; h++) chess_prepare_for_pipelining chess_loop_range(OUT_H,) {
        for (int w = 0; w < OUT_W; w++) chess_prepare_for_pipelining chess_loop_range(OUT_W_PAD,) {
        
          float res = bias[m];
          
          for (int c = 0; c < C_PER_M; c++) chess_prepare_for_pipelining chess_loop_range(C_PER_M,) {
            for (int p = 0; p < KH; p++) chess_flatten_loop {
              for (int q = 0; q < KW; q++) chess_flatten_loop {
                float a = window_readincr(in);
                res += a * weights[weightIdx];
                weightIdx++;
              }
              window_incr(in, -KW+INP_W); // go left KW, down 1
            }
            window_incr(in, -KH*INP_W + INP_H*INP_W); // go up KH, channel 1
          } // C

          if (IS_RELU) {
            res = std::max(res, 0.0f);
          }
          window_writeincr(out, res);
          window_incr(in, -C_PER_M*INP_H*INP_W + STEP_W); // go channel -C_PER_M, right STEP_W
          weightIdx -= C_PER_M*KH*KW;
        } // W

        window_incr(out, OUT_W_PAD - OUT_W);
        window_incr(in, -OUT_W*STEP_W + INP_W*STEP_H); // go left OUT_W*STEP_W, go down STEP_H
      } // H
      window_incr(in, -INP_W*OUT_H*STEP_H); // go up OUT_H*STEP_H
      weightIdx += C_PER_M*KH*KW;
      if (m % (M/GROUP) == M/GROUP - 1) {
        window_incr(in, C_PER_M*INP_H*INP_W); // next C_PER_M channels
      }
    } // M
  } // B

  CONV_PROFILE_FOOTER("ConvReluScalar");
}


template <int INP_H, int INP_W, int OUT_W, int OUT_W_PAD, int STEP_H, int STEP_W, 
          int B, int C, int M, int KH, int KW, int GROUP, int IS_RELU>
void Conv5x5on8Relu<INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W, B, C, M, KH, KW, GROUP, IS_RELU>::filter(
	input_window<float>* in,      // BCHW
  output_window<float>* out     // BMHW
) {
  PROFILE_HEADER2;

  v16float data = null_v16float();
  v8float zeros = null_v8float();
  v8float* wvec = (v8float *) weights;

#define MAC_ROW(acc) \
  acc = fpmac(acc, data, 0, 0x76543210, *wvec, 0, 0x00000000); \
  acc = fpmac(acc, data, 1, 0x76543210, *wvec, 1, 0x00000000); \
  acc = fpmac(acc, data, 2, 0x76543210, *wvec, 2, 0x00000000); \
  acc = fpmac(acc, data, 3, 0x76543210, *wvec, 3, 0x00000000); \
  acc = fpmac(acc, data, 4, 0x76543210, *wvec, 4, 0x00000000);

  // BHWM
  for (int b = 0; b < B; b++) chess_prepare_for_pipelining chess_loop_range(B,) {
    for (int m = 0; m < M; m++) chess_prepare_for_pipelining chess_loop_range(M,) {// computes one output channel
      for (int h = 0; h < OUT_H; h+=2) chess_prepare_for_pipelining chess_loop_range(OUT_H/2,) {
        for (int w = 0; w < OUT_W_PAD; w+=8) chess_prepare_for_pipelining chess_loop_range(OUT_W_PAD/8,) { // computes 8 output channel pixels
          
          v8float acc1 = aie::broadcast<float, 8>(bias[m]);
          v8float acc2 = aie::broadcast<float, 8>(bias[m]);

          for (int c = 0; c < C; c++) chess_prepare_for_pipelining chess_loop_range(C,) { // computes 8 partial products over 5x5 kernel
            data = upd_w(data, 0, window_readincr_v8(in));
            data = upd_w(data, 1, window_readincr_v8(in));
            window_incr(in, INP_W-16);
            MAC_ROW(acc1);
            data = upd_w(data, 0, window_readincr_v8(in));
            data = upd_w(data, 1, window_readincr_v8(in));
            window_incr(in, INP_W-16);
            MAC_ROW(acc2);
            wvec++;
            
            MAC_ROW(acc1);
            data = upd_w(data, 0, window_readincr_v8(in));
            data = upd_w(data, 1, window_readincr_v8(in));
            window_incr(in, INP_W-16);
            MAC_ROW(acc2);
            wvec++;
            
            MAC_ROW(acc1);
            data = upd_w(data, 0, window_readincr_v8(in));
            data = upd_w(data, 1, window_readincr_v8(in));
            window_incr(in, INP_W-16);
            MAC_ROW(acc2);
            wvec++;
            
            MAC_ROW(acc1);
            data = upd_w(data, 0, window_readincr_v8(in));
            data = upd_w(data, 1, window_readincr_v8(in));
            window_incr(in, INP_W-16);
            MAC_ROW(acc2);
            wvec++;
            
            MAC_ROW(acc1);
            data = upd_w(data, 0, window_readincr_v8(in));
            data = upd_w(data, 1, window_readincr_v8(in));
            window_incr(in, INP_H*INP_W - 5*INP_W - 16);
            MAC_ROW(acc2);
            wvec++;
          }
          window_incr(in, -C*INP_H*INP_W + 8); // data go channel -C, right 8
                    
          if (IS_RELU) {
            acc1 = fpmax(acc1, zeros, 0, 0x76543210);
            acc2 = fpmax(acc2, zeros, 0, 0x76543210);
          }
          window_write(out, acc1);
          window_incr(out, OUT_W_PAD);
          window_write(out, acc2);
          window_incr(out, -OUT_W_PAD+8);
          wvec -= C*5;

        } // W
        window_incr(in, 2*INP_W-OUT_W_PAD); // go left OUT_W_PAD, go down 1
        window_incr(out, OUT_W_PAD);
        chess_separator_scheduler(); // uncomment if compiler cannot detect out dependency
      } // H
      window_incr(in, -OUT_H*INP_W); // go up OUT_H
      wvec += C*5;
    } // M
  } // B

#undef MAC_ROW

  CONV_PROFILE_FOOTER("Conv5x5on8Relu");
}


template <int INP_H, int INP_W, int OUT_W, int OUT_W_PAD, int STEP_H, int STEP_W, 
          int B, int C, int M, int KH, int KW, int GROUP, int IS_RELU>
void ConvHx4Relu<INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W, B, C, M, KH, KW, GROUP, IS_RELU>::filter(
	input_window<float>* in,      // BCHW
  output_window<float>* out     // BMHW
) {
  PROFILE_HEADER2;

  v16float data = null_v16float();
  v8float zeros = null_v8float();
  float *w_ptr = (float *) weights;
  float *b_ptr = (float *) bias;

#define MAC_ROW(acc) \
  for (int i = 0; i < KW; i++) { \
    acc = fpmac(acc, data, i, 0x76543210, *(v8float *) w_ptr, i, 0); \
  }

#define UPD_DATA \
  data = upd_w(data, 0, window_readincr_v8(in)); \
  data = upd_v(data, 2, window_readincr_v4(in)); \
  window_incr(in, INP_W - 12);

  // BHWM
  for (int b = 0; b < B; b++) chess_prepare_for_pipelining chess_loop_range(B,) {
    for (int m = 0; m < M; m++) chess_prepare_for_pipelining chess_loop_range(M,) { // computes one output channel
      for (int h = 0; h < OUT_H; h+=2) chess_prepare_for_pipelining chess_loop_range(OUT_H/2,) {
        for (int w = 0; w < OUT_W_PAD; w+=8) chess_prepare_for_pipelining chess_loop_range(OUT_W_PAD/8,) { // computes 8 output channel pixels
          
          v8float acc1 = aie::broadcast<float, 8>(*b_ptr);
          v8float acc2 = aie::broadcast<float, 8>(*b_ptr);

          for (int c = 0; c < C; c++) chess_prepare_for_pipelining chess_loop_range(C,) { // computes 8 partial products over 5x5 kernel
            UPD_DATA
            MAC_ROW(acc1);
            UPD_DATA
            MAC_ROW(acc2);
            w_ptr += 4;

            MAC_ROW(acc1);            
            UPD_DATA
            MAC_ROW(acc2);
            w_ptr += 4;

            MAC_ROW(acc1);
            UPD_DATA;
            MAC_ROW(acc2);
            w_ptr += 4;
            window_incr(in, INP_H*INP_W - (KH+1)*INP_W);
          }
          window_incr(in, -C*INP_H*INP_W + 8); // data go channel -C, right 8
          w_ptr -= CKK_ROW_SIZE;
                    
          if (IS_RELU) {
            acc1 = fpmax(acc1, zeros, 0, 0x76543210);
            acc2 = fpmax(acc2, zeros, 0, 0x76543210);
          }
          window_write(out, acc1);
          window_incr(out, OUT_W_PAD);
          window_write(out, acc2);
          window_incr(out, -OUT_W_PAD+8);

        } // W
        window_incr(in, -OUT_W_PAD*STEP_W + 2*INP_W*STEP_H); // go left OUT_W_PAD*STEP_W, go down 2*STEP_H
        window_incr(out, OUT_W_PAD);
        chess_separator_scheduler(); // uncomment if compiler cannot detect out dependency
      } // H
      window_incr(in, -INP_W*OUT_H*STEP_H); // go up OUT_H * STEP_H
      w_ptr += CKK_ROW_SIZE;
      b_ptr ++;
    } // M
  } // B

#undef UPD_DATA
#undef MAC_ROW

  CONV_PROFILE_FOOTER("ConvHx4Relu");
}


template <int INP_H, int INP_W, int OUT_W, int OUT_W_PAD, int STEP_H, int STEP_W, 
          int B, int C, int M, int KH, int KW, int GROUP, int IS_RELU>
void Conv1x1Relu<INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W, B, C, M, KH, KW, GROUP, IS_RELU>::filter(
	input_window<float>* in,      // BCHW
  output_window<float>* out     // BMHW
) {
  PROFILE_HEADER2;
  
  int weightIdx = 0;
  aie::vector<float, 8> data = null_v8float();
  v16float res = null_v16float();

  for (int b = 0; b < B; b++) chess_prepare_for_pipelining chess_loop_range(B,) {
    for (int m = 0; m < M; m++) chess_prepare_for_pipelining chess_loop_range(M,) { 
      
      for (int h = 0; h < OUT_H; h++) chess_prepare_for_pipelining chess_loop_range(OUT_H,) {
        for (int w = 0; w < OUT_W_PAD; w+=8/STEP_W) {
        
          aie::accum<accfloat, 8> acc1;
          acc1.from_vector(aie::broadcast<float, 8>(bias[m]), 0);
          
          for (int c = 0; c < C; c++) chess_prepare_for_pipelining chess_loop_range(C,) {
            data = window_read_v8(in);
            window_incr(in, INP_H*INP_W);
            acc1 = aie::mac(acc1, data, weights[weightIdx]); weightIdx++;
          } // C
          window_incr(in, -C*INP_H*INP_W + 8);
          weightIdx -= C;

          if (IS_RELU) {
            acc1 = fpmax(acc1, null_v8float(), 0, 0x76543210);
          }

          if (STEP_W == 2) {
            acc1 = fpshuffle(acc1, 0, 0x00006420);
            window_writeincr(out, ext_v(acc1, 0));
          } else {
            window_writeincr(out, acc1);
          }
        } // W

        window_incr(in, -OUT_W_PAD*STEP_W+INP_W*STEP_H); // go left OUT_W_PAD*STEP_W, go down STEP_H
        chess_separator_scheduler(); // uncomment if compiler cannot detect out dependency
      } // H
      window_incr(in, -INP_W*OUT_H*STEP_H); // go up OUT_H * STEP_H
      weightIdx += C;
    } // M
  } // B

#undef MAC_ROW

  CONV_PROFILE_FOOTER("Conv1x1Relu");
}


template <int INP_H, int INP_W, int OUT_W, int OUT_W_PAD, int STEP_H, int STEP_W, 
          int B, int C, int M, int KH, int KW, int GROUP, int IS_RELU>
void ConvReluScalarStream<INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W, B, C, M, KH, KW, GROUP, IS_RELU>::filter(
	input_window<float>* in,      // BCHW
  input_stream<float>* restrict weights, // MCKK
  output_stream<float>* restrict out     // BMHW
) {
  PROFILE_HEADER2;
  
  int weightIdx;

  for (int b = 0; b < B; b++) chess_prepare_for_pipelining chess_loop_range(B,) {
    for (int m = 0; m < M; m++) chess_prepare_for_pipelining chess_loop_range(M,) { 

      for (int i = 0; i < CKK_ROW_SIZE; i++) chess_prepare_for_pipelining chess_loop_range(CKK_ROW_SIZE,) {
        ckk_row[i] = readincr(weights);
      }
      
      for (int h = 0; h < OUT_H; h++) chess_prepare_for_pipelining chess_loop_range(OUT_H,) {
        for (int w = 0; w < OUT_W; w++) {
        
          float res = bias[m];
          weightIdx = 0;
          
          for (int c = 0; c < C_PER_M; c++) {
            for (int p = 0; p < KH; p++) chess_flatten_loop {
              for (int q = 0; q < KW; q++) chess_flatten_loop {
                float a = window_readincr(in);
                res += a * ckk_row[weightIdx];
                weightIdx++;
              }
              window_incr(in, -KW+INP_W); // go left KW, down 1
            }
            window_incr(in, -KH*INP_W + INP_H*INP_W); // go up KH, channel 1
          } // C

          if (IS_RELU) {
            if (res < 0) res = 0;
          }
          writeincr(out, res);
          window_incr(in, -C_PER_M*INP_H*INP_W + STEP_W); // go channel -C_PER_M, right STEP_W
        } // W

        for (int w = 0; w < OUT_W_PAD - OUT_W; w++) {
          writeincr(out, 0);
        }
        window_incr(in, -OUT_W*STEP_W + INP_W*STEP_H); // go left OUT_W*STEP_W, go down STEP_H
        chess_separator_scheduler(); // uncomment if compiler cannot detect out dependency
      } // H
      window_incr(in, -INP_W*OUT_H*STEP_H); // go up OUT_H*STEP_H
      if (m % (M/GROUP) == M/GROUP - 1) {
        window_incr(in, C_PER_M*INP_H*INP_W); // next C_PER_M channels
      }
    } // M
  } // B

  CONV_PROFILE_FOOTER("ConvReluScalarStream");
}


template <int INP_H, int INP_W, int OUT_W, int OUT_W_PAD, int STEP_H, int STEP_W, 
          int B, int C, int M, int KH, int KW, int GROUP, int IS_RELU>
void ConvHx8ReluStream<INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W, B, C, M, KH, KW, GROUP, IS_RELU>::filter(
	input_window<float>* in,
  input_stream<float>* restrict weights,
  output_stream<float>* restrict out
) {
  PROFILE_HEADER2;

  float* restrict w_ptr = (float *) ckk_row;
  float* restrict width_ptr = (float *) width_row;
  v16float data = null_v16float();

#define UPD_DATA \
  data = upd_w(data, 0, window_readincr_v8(in)); \
  data = upd_w(data, 1, window_readincr_v8(in)); \
  window_incr(in, INP_W-16);

#define MAC_ROW(acc) \
  for (int q = 0; q < KW; q++) { \
    acc = fpmac(acc, data, q, 0x76543210, *(v8float *) w_ptr, q, 0x00000000); \
  }

  // BHWM
  for (int b = 0; b < B; b++) chess_prepare_for_pipelining chess_loop_range(B,) {
    for (int m = 0; m < M; m++) chess_prepare_for_pipelining chess_loop_range(M,) {// computes one output channel

      for (int i = 0; i < CKK_ROW_SIZE; i+=4) chess_prepare_for_pipelining chess_loop_range(CKK_ROW_SIZE/4,) {
        *(v4float *) w_ptr = readincr_v4(weights); w_ptr += 4;
      }
      w_ptr -= CKK_ROW_SIZE;

      for (int h = 0; h < OUT_H; h+=2) chess_prepare_for_pipelining chess_loop_range(OUT_H/2,) {
        for (int w = 0; w < OUT_W_PAD; w+=8) chess_prepare_for_pipelining chess_loop_range(OUT_W_PAD/8,) { // computes 8 output channel pixels
          
          v8float acc1 = aie::broadcast<float, 8>(bias[m]);
          v8float acc2 = aie::broadcast<float, 8>(bias[m]);

          for (int c = 0; c < C; c++) chess_prepare_for_pipelining chess_loop_range(C,) { // computes 8 partial products over 5x5 kernel
            UPD_DATA;
            MAC_ROW(acc1);
            
            for (int p = 0; p < KH-1; p++) {
              UPD_DATA;
              MAC_ROW(acc2);
              w_ptr += 8;
              MAC_ROW(acc1);
            }
            
            UPD_DATA;
            MAC_ROW(acc2);
            w_ptr += 8;
            window_incr(in, INP_H*INP_W - 6*INP_W);
          }
          window_incr(in, -C*INP_H*INP_W + 8); // data go channel -C, right 8
          w_ptr -= CKK_ROW_SIZE;

          if (IS_RELU) {
            acc1 = fpmax(acc1, null_v8float(), 0, 0x76543210);
            acc2 = fpmax(acc2, null_v8float(), 0, 0x76543210);
          }
          writeincr_v4(out, ext_v(acc1, 0));
          writeincr_v4(out, ext_v(acc1, 1));
          *(v4float *) width_ptr = ext_v(acc2, 0); width_ptr += 4;
          *(v4float *) width_ptr = ext_v(acc2, 1); width_ptr += 4;
        } // W
        width_ptr -= OUT_W_PAD;

        for (int w = 0; w < OUT_W_PAD; w+=4) {
          writeincr_v4(out, *(v4float *) width_ptr); width_ptr += 4;
        }
        width_ptr -= OUT_W_PAD;

        window_incr(in, 2*INP_W*STEP_W - OUT_W_PAD*STEP_H); // go left OUT_W_PAD, go down STEP_H
        chess_separator_scheduler(); // uncomment if compiler cannot detect out dependency
      } // H
      
      window_incr(in, -OUT_H*INP_W*STEP_H); // go up OUT_H*STEP_H
    } // M
  } // B

#undef UPD_DATA
#undef MAC_ROW

  CONV_PROFILE_FOOTER("ConvHx8ReluStream");
}


#define MAC_ROW(acc) \
  for (int i = 0; i < KW; i++) { \
    acc = fpmac(acc, data, i, X_OFFSET, *(v8float *) w_ptr, i, 0); \
  }

#define UPD_DATA \
  data = upd_w(data, 0, window_readincr_v8(in)); \
  data = upd_w(data, 1, window_readincr_v8(in)); \
  window_incr(in, INP_W - 16);

// double acc require store in cache and write where VLIW underutilized
template <int INP_H, int INP_W, int OUT_W, int OUT_W_PAD, int STEP_H, int STEP_W, 
          int B, int C, int M, int KH, int KW, int GROUP, int IS_RELU>
void ConvHx4ReluStream<INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W, B, C, M, KH, KW, GROUP, IS_RELU>::filter(
	input_window<float>* in,      // BCHW
  input_stream<float>* restrict weights, // MCKK
  output_stream<float>* restrict out     // BMHW
) {
  PROFILE_HEADER2;
  
  float* restrict w_ptr = (float *) ckk_row;
  v16float data = null_v16float();

  for (int b = 0; b < B; b++) chess_prepare_for_pipelining chess_loop_range(B,) {
    for (int m = 0; m < M; m++) chess_prepare_for_pipelining chess_loop_range(M,) { 

      for (int i = 0; i < CKK_ROW_SIZE; i+=4) chess_prepare_for_pipelining chess_loop_range(CKK_ROW_SIZE/4,) {
        *(v4float *) w_ptr = readincr_v4(weights); w_ptr += 4;
      }
      w_ptr -= CKK_ROW_SIZE;
      
      for (int h = 0; h < OUT_H; h++) chess_prepare_for_pipelining chess_loop_range(OUT_H,) {
        for (int w = 0; w < OUT_W_PAD; w+=W_LOOP_STEP) {
        
          v8float acc1 = aie::broadcast<float, 8>(bias[m]);
          
          for (int c = 0; c < C_PER_M; c++) chess_prepare_for_pipelining chess_loop_range(C_PER_M,) {
            for (int p = 0; p < KH; p++) chess_flatten_loop {
              UPD_DATA;
              MAC_ROW(acc1);
              w_ptr += 4;
            }
            window_incr(in, INP_H*INP_W - KH*INP_W);
          } // C_PER_M
          window_incr(in, -C_PER_M*INP_H*INP_W + W_LOOP_IN_STEP);
          w_ptr -= CKK_ROW_SIZE;

          if (IS_RELU) {
            acc1 = fpmax(acc1, null_v8float(), 0, 0x76543210);
          }

          if (STEP_W > 1) {
            writeincr_v4(out, ext_v(acc1, 0));
          } else {
            writeincr_v4(out, ext_v(acc1, 0));
            writeincr_v4(out, ext_v(acc1, 1));
          }
        } // W

        window_incr(in, -OUT_W_PAD*STEP_W+INP_W*STEP_H); // go left OUT_W_PAD*STEP_W, go down STEP_H
        chess_separator_scheduler(); // uncomment if compiler cannot detect out dependency
      } // H
      window_incr(in, -INP_W*OUT_H*STEP_H); // go up OUT_H * STEP_H
      if (m % (M/GROUP) == M/GROUP - 1) {
        window_incr(in, C_PER_M*INP_H*INP_W);
      }
    } // M
  } // B

  CONV_PROFILE_FOOTER("ConvHx4ReluStream");
}


// stride > 1 have no reuse of data down the row
template <int INP_H, int INP_W, int OUT_W, int OUT_W_PAD, int STEP_H, int STEP_W, 
          int B, int C, int M, int KH, int KW, int GROUP, int IS_RELU>
void ConvHx4ReluStreamMultiRow<INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W, B, C, M, KH, KW, GROUP, IS_RELU>::filter(
	input_window<float>* in,      // BCHW
  input_stream<float>* restrict weights, // MCKK
  output_stream<float>* restrict out     // BMHW
) {
  PROFILE_HEADER2;
  
  float* restrict w_ptr = (float *) ckk_row;
  
  v16float data = null_v16float();

  for (int b = 0; b < B; b++) chess_prepare_for_pipelining chess_loop_range(B,) {
    for (int m = 0; m < M; m++) chess_prepare_for_pipelining chess_loop_range(M,) { 

      for (int i = 0; i < CKK_ROW_SIZE; i+=4) chess_prepare_for_pipelining chess_loop_range(CKK_ROW_SIZE/4,) {
        *(v4float *) w_ptr = readincr_v4(weights); w_ptr += 4;
      }
      w_ptr -= CKK_ROW_SIZE;
      
      for (int h = 0; h < OUT_H; h+=2) chess_prepare_for_pipelining chess_loop_range(OUT_H/2,) {
        
        v8float *out_row_ptr = (v8float *) out_row;
        
        for (int w = 0; w < OUT_W_PAD; w+=8/STEP_W) {
        
          v8float acc1 = aie::broadcast<float, 8>(bias[m]);
          v8float acc2 = aie::broadcast<float, 8>(bias[m]);
          
          for (int c = 0; c < C; c++) chess_prepare_for_pipelining chess_loop_range(C,) {
            UPD_DATA;
            MAC_ROW(acc1);
            UPD_DATA;
            MAC_ROW(acc2);
            w_ptr += 4;

            MAC_ROW(acc1);
            UPD_DATA;
            MAC_ROW(acc2);
            w_ptr += 4;

            MAC_ROW(acc1);
            UPD_DATA;
            MAC_ROW(acc2);
            w_ptr += 4;
            
            window_incr(in, INP_H*INP_W - (KH+1)*INP_W);
          } // C
          window_incr(in, -C*INP_H*INP_W + 8);
          w_ptr -= CKK_ROW_SIZE;

          if (IS_RELU) {
            acc1 = fpmax(acc1, null_v8float(), 0, 0x76543210);
            acc2 = fpmax(acc2, null_v8float(), 0, 0x76543210);
          }

          writeincr_v4(out, ext_v(acc1, 0));
          writeincr_v4(out, ext_v(acc1, 1));
          *out_row_ptr = acc2; out_row_ptr++;
        } // W

        window_incr(in, -OUT_W_PAD*STEP_W+2*INP_W*STEP_H); // go left OUT_W_PAD*STEP_W, go down 2*STEP_H
        
        v4float *_out_row_ptr = (v4float *) out_row;
        for (int i = 0; i < OUT_W_PAD; i+=4) chess_prepare_for_pipelining chess_loop_range(OUT_W_PAD/4,) {
          writeincr_v4(out, *_out_row_ptr); _out_row_ptr++;
        }

        chess_separator_scheduler(); // uncomment if compiler cannot detect out dependency

      } // H
      window_incr(in, -INP_W*OUT_H*STEP_H); // go up OUT_H * STEP_H
    } // M
  } // B


  CONV_PROFILE_FOOTER("ConvHx4ReluStreamMultiRow");
}
#undef UPD_DATA
#undef MAC_ROW


// half of mac lanes are wasted, use two lanes
template <int INP_H, int INP_W, int OUT_W, int OUT_W_PAD, int STEP_H, int STEP_W, 
          int B, int C, int M, int KH, int KW, int GROUP, int IS_RELU>
void ConvHx4Out4ReluStream<INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W, B, C, M, KH, KW, GROUP, IS_RELU>::filter(
	input_window<float>* in,      // BCHW
  input_stream<float>* restrict weights, // MCKK
  output_stream<float>* restrict out     // BMHW
) {
  PROFILE_HEADER2;
  
  float* restrict w_ptr = (float *) ckk_row;
  
  v16float data = null_v16float();

  for (int b = 0; b < B; b++) chess_prepare_for_pipelining chess_loop_range(B,) {
    for (int m = 0; m < M; m++) chess_prepare_for_pipelining chess_loop_range(M,) { 

      for (int i = 0; i < CKK_ROW_SIZE; i+=4) chess_prepare_for_pipelining chess_loop_range(CKK_ROW_SIZE/4,) {
        *(v4float *) w_ptr = readincr_v4(weights); w_ptr += 4;
      }
      w_ptr -= CKK_ROW_SIZE;
      
      for (int h = 0; h < OUT_H; h++) chess_prepare_for_pipelining chess_loop_range(OUT_H,) {
        
        v8float acc1 = aie::broadcast<float, 8>(bias[m]);
        
        for (int c = 0; c < C_PER_M; c++) chess_prepare_for_pipelining chess_loop_range(C_PER_M,) {
          for (int p = 0; p < KH; p++) chess_flatten_loop {
            data = upd_w(data, 0, window_read_v8(in));
            window_incr(in, INP_W);
            for (int i = 0; i < KW; i++)
              acc1 = fpmac(acc1, data, i, 0x76543210, *(v8float *) w_ptr, i, 0); // half of macs are wasted
            w_ptr += 4;
          }
          window_incr(in, INP_H*INP_W - KH*INP_W);
        } // C_PER_M
        window_incr(in, -C_PER_M*INP_H*INP_W);
        w_ptr -= CKK_ROW_SIZE;

        if (IS_RELU) {
          acc1 = fpmax(acc1, null_v8float(), 0, 0x76543210);
        }

        writeincr_v4(out, ext_v(acc1, 0));

        window_incr(in, INP_W); // go down 1
        chess_separator_scheduler(); // uncomment if compiler cannot detect out dependency
      } // H
      window_incr(in, -INP_W*OUT_H); // go up OUT_H
      if (m % (M/GROUP) == M/GROUP - 1) {
        window_incr(in, C_PER_M*INP_H*INP_W);
      }
    } // M
  } // B
#undef UPD_DATA
#undef MAC_ROW

  CONV_PROFILE_FOOTER("ConvHx4Out4ReluStream");
}


template <int INP_H, int INP_W, int OUT_W, int OUT_W_PAD, int STEP_H, int STEP_W, 
          int B, int C, int M, int KH, int KW, int GROUP, int IS_RELU>
void Conv1x1ReluStream<INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W, B, C, M, KH, KW, GROUP, IS_RELU>::filter(
	input_window<float>* in,      // BCHW
  input_stream<float>* restrict weights, // MCKK
  output_stream<float>* restrict out     // BMHW
) {
  PROFILE_HEADER2;
  
  float* w_ptr;
  aie::vector<float, 8> data = null_v8float();
  v8float zeros = null_v8float();
  v16float res = null_v16float();

  for (int b = 0; b < B; b++) chess_prepare_for_pipelining chess_loop_range(B,) {
    for (int m = 0; m < M; m++) chess_prepare_for_pipelining chess_loop_range(M,) { 

      w_ptr = (float *) ckk_row;
      for (int i = 0; i < CKK_ROW_SIZE; i+=4) chess_prepare_for_pipelining chess_loop_range(CKK_ROW_SIZE/4,) {
        *(v4float *) w_ptr = readincr_v4(weights); w_ptr += 4;
      }
      
      for (int h = 0; h < OUT_H; h++) chess_prepare_for_pipelining chess_loop_range(OUT_H,) {
        for (int w = 0; w < OUT_W_PAD; w+=8/STEP_W) {
        
          aie::accum<accfloat, 8> acc1;
          acc1.from_vector(aie::broadcast<float, 8>(bias[m]), 0);
          
          for (int c = 0; c < C; c++) chess_prepare_for_pipelining chess_loop_range(C,) {
            data = window_read_v8(in);
            window_incr(in, INP_H*INP_W);
            acc1 = aie::mac(acc1, data, ckk_row[c]);
          } // C
          window_incr(in, -C*INP_H*INP_W + 8);

          if (IS_RELU) {
            acc1 = fpmax(acc1, zeros, 0, 0x76543210);
          }

          if (STEP_W == 2) {
            acc1 = fpshuffle(acc1, 0, 0x00006420);
            writeincr_v4(out, ext_v(acc1, 0));
          } else {
            writeincr_v4(out, ext_v(acc1, 0));
            writeincr_v4(out, ext_v(acc1, 1));
          }
        } // W

        window_incr(in, -OUT_W_PAD*STEP_W+INP_W*STEP_H); // go left OUT_W_PAD*STEP_W, go down STEP_H
        chess_separator_scheduler(); // uncomment if compiler cannot detect out dependency
      } // H
      window_incr(in, -INP_W*OUT_H*STEP_H); // go up OUT_H * STEP_H
    } // M
  } // B

#undef MAC_ROW

  CONV_PROFILE_FOOTER("Conv1x1ReluStream");
}


template <int INP_H, int INP_W, int OUT_W, int OUT_W_PAD, int STEP_H, int STEP_W, 
          int B, int C, int M, int KH, int KW, int GROUP, int IS_RELU>
void Conv1x1Out4ReluStream<INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W, B, C, M, KH, KW, GROUP, IS_RELU>::filter(
	input_window<float>* in,      // BCHW
  input_stream<float>* restrict weights, // MCKK
  output_stream<float>* restrict out     // BMHW
) {
  PROFILE_HEADER2;
  
  float* w_ptr;
  
  aie::vector<float, 8> data = null_v8float();

  for (int b = 0; b < B; b++) chess_prepare_for_pipelining chess_loop_range(B,) {
    for (int m = 0; m < M; m++) chess_prepare_for_pipelining chess_loop_range(M,) { 

      float* w_ptr = (float *) ckk_row;
      for (int i = 0; i < CKK_ROW_SIZE; i+=4) chess_prepare_for_pipelining chess_loop_range(CKK_ROW_SIZE/4,) {
        *(v4float *) w_ptr = readincr_v4(weights); w_ptr += 4;
      }
      
      for (int h = 0; h < OUT_H; h++) chess_prepare_for_pipelining chess_loop_range(OUT_H,) {
        
        aie::accum<accfloat, 8> acc1;
        acc1.from_vector(aie::broadcast<float, 8>(bias[m]), 0);
        
        for (int c = 0; c < C_PER_M; c++) chess_prepare_for_pipelining chess_loop_range(C_PER_M,) {
          data = window_read_v8(in);
          window_incr(in, INP_H*INP_W);
          acc1 = aie::mac(acc1, data, ckk_row[c]);
        } // C_PER_M
        window_incr(in, -C_PER_M*INP_H*INP_W);

        if (IS_RELU) {
          acc1 = fpmax(acc1, null_v8float(), 0, 0x76543210);
        }

        writeincr_v4(out, ext_v(acc1, 0));

        window_incr(in, INP_W); // go down 1
        chess_separator_scheduler(); // uncomment if compiler cannot detect out dependency
      } // H
      window_incr(in, -INP_W*OUT_H); // go up OUT_H
      if (m % (M/GROUP) == M/GROUP - 1) {
        window_incr(in, C_PER_M*INP_H*INP_W);
      }
    } // M
  } // B

#undef MAC_ROW

  CONV_PROFILE_FOOTER("Conv1x1Out4ReluStream");
}


template <int INP_H, int INP_W, int OUT_W, int OUT_W_PAD, int STEP_H, int STEP_W, 
          int B, int C, int M, int KH, int KW, int GROUP, int IS_RELU>
void ConvHx4ReluPktStream<INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W, B, C, M, KH, KW, GROUP, IS_RELU>::filter(
	input_pktstream* restrict in_s,      // BCHW
  input_stream<float>* restrict weights, // MCKK
  output_stream<float>* restrict out     // BMHW
) {

  PROFILE_HEADER2;

#define MAC_ROW(acc) \
  for (int i = 0; i < KW; i++) { \
    acc = fpmac(acc, data, i, X_OFFSET, *(v8float *) w_ptr, i, 0); \
  }

#define UPD_DATA \
  data = upd_w(data, 0, *(v8float *) in_ptr); in_ptr += 8; \
  data = upd_w(data, 1, *(v8float *) in_ptr); in_ptr += INP_W - 8;
  
  float* restrict w_ptr = (float *) ckk_row;
  float* restrict in_ptr = (float*) in;
  
  v16float data = null_v16float();

  // fill window
  for (int bc = 0; bc < B*C; bc++) {
    get_ss(0); // discard header
    for (int hw = 0; hw < INP_H*INP_W; hw++) {
      *in_ptr = getf_ss(0); in_ptr++;
    }
  }
  in_ptr = (float*) in;

  for (int b = 0; b < B; b++) chess_prepare_for_pipelining chess_loop_range(B,) {
    for (int m = 0; m < M; m++) chess_prepare_for_pipelining chess_loop_range(M,) { 

      for (int i = 0; i < CKK_ROW_SIZE; i+=4) {
        *(v4float *) w_ptr = readincr_v4(weights); w_ptr += 4;
      }
      w_ptr -= CKK_ROW_SIZE;
      
      for (int h = 0; h < OUT_H; h++) chess_prepare_for_pipelining chess_loop_range(OUT_H,) {
        for (int w = 0; w < OUT_W_PAD; w+=W_LOOP_STEP) {
        
          v8float acc1 = aie::broadcast<float, 8>(bias[m]);
          
          for (int c = 0; c < C_PER_M; c++) {
            for (int p = 0; p < KH; p++) chess_flatten_loop {
              UPD_DATA;
              MAC_ROW(acc1);
              w_ptr += 4;
            }
            in_ptr += INP_H*INP_W - KH*INP_W;
          } // C_PER_M
          in_ptr += -C_PER_M*INP_H*INP_W + W_LOOP_IN_STEP;
          w_ptr -= CKK_ROW_SIZE;

          if (IS_RELU) {
            acc1 = fpmax(acc1, null_v8float(), 0, 0x76543210);
          }

          if (STEP_W > 1) {
            put_wms(0, ext_v(acc1, 0));
          } else {
            put_wms(0, ext_v(acc1, 0));
            put_wms(0, ext_v(acc1, 1));
          }
        } // W

        in_ptr += -OUT_W_PAD*STEP_W+INP_W*STEP_H; // go left OUT_W_PAD*STEP_W, go down STEP_H
        chess_separator_scheduler(); // uncomment if compiler cannot detect out dependency
      } // H
      in_ptr += -INP_W*OUT_H*STEP_H; // go up OUT_H * STEP_H
      if (m % (M/GROUP) == M/GROUP - 1) {
        in_ptr += C_PER_M*INP_H*INP_W;
      }
    } // M
  } // B


#undef UPD_DATA
#undef MAC_ROW

  CONV_PROFILE_FOOTER("ConvHx4ReluPktStream");
}


template <int INP_H, int INP_W, int OUT_W, int OUT_W_PAD, int STEP_H, int STEP_W, 
          int B, int C, int M, int KH, int KW, int GROUP, int IS_RELU>
void Conv1x1ReluPktStream<INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W, B, C, M, KH, KW, GROUP, IS_RELU>::filter(
  input_pktstream* in_s,        // BCHW
  input_stream<float>* weights, // MCKK
  output_stream<float>* out     // BMHW
) {
  PROFILE_HEADER2;
  
  float* w_ptr;
  float* in_ptr = (float*) in;

  aie::vector<float, 8> data = null_v8float();
  v8float zeros = null_v8float();
  v16float res = null_v16float();

  // fill window
  for (int bc = 0; bc < B*C; bc++) {
    get_ss(0); // discard header
    for (int hw = 0; hw < INP_H*INP_W; hw++) {
      *in_ptr = getf_ss(0); in_ptr++;
    }
  }
  in_ptr = (float*) in;

  for (int b = 0; b < B; b++) chess_prepare_for_pipelining chess_loop_range(B,) {
    for (int m = 0; m < M; m++) chess_prepare_for_pipelining chess_loop_range(M,) { 

      float* w_ptr = (float *) ckk_row;
      for (int i = 0; i < CKK_ROW_SIZE; i+=4) chess_prepare_for_pipelining chess_loop_range(CKK_ROW_SIZE/4,) {
        *(v4float *) w_ptr = readincr_v4(weights); w_ptr += 4;
      }
      
      for (int h = 0; h < OUT_H; h++) chess_prepare_for_pipelining chess_loop_range(OUT_H,) {
        for (int w = 0; w < OUT_W_PAD; w+=8/STEP_W) {
        
          aie::accum<accfloat, 8> acc1;
          acc1.from_vector(aie::broadcast<float, 8>(bias[m]), 0);
          
          for (int c = 0; c < C; c++) chess_prepare_for_pipelining chess_loop_range(C,) {
            data = *(v8float *) in_ptr; in_ptr += INP_H*INP_W;
            acc1 = aie::mac(acc1, data, ckk_row[c]);
          } // C
          in_ptr += -C*INP_H*INP_W + 8;

          if (IS_RELU) {
            acc1 = fpmax(acc1, zeros, 0, 0x76543210);
          }

          if (STEP_W == 2) {
            acc1 = fpshuffle(acc1, 0, 0x00006420);
            writeincr_v4(out, ext_v(acc1, 0));
          } else {
            writeincr_v4(out, ext_v(acc1, 0));
            writeincr_v4(out, ext_v(acc1, 1));
          }
        } // W

        in_ptr += -OUT_W_PAD*STEP_W+INP_W*STEP_H; // go left OUT_W_PAD*STEP_W, go down STEP_H
        chess_separator_scheduler(); // uncomment if compiler cannot detect out dependency
      } // H
      in_ptr += -INP_W*OUT_H*STEP_H; // go up OUT_H * STEP_H
    } // M
  } // B

#undef MAC_ROW

  CONV_PROFILE_FOOTER("Conv1x1ReluPktStream");
}
