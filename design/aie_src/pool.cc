#include <limits>

#include "pool.h"
#include "kernel_utils.h"


#define POOL_PROFILE_FOOTER(filter_name) \
  PROFILE_FOOTER2("%s<%d,%d,%d,%d,%d,%d,%d,%d,%d,%d>", \
    filter_name, INP_H, INP_W, OUT_H, OUT_W, B, C, KH, KW, STEP_H, STEP_W);

template <typename TT, int INP_H, int INP_W, int OUT_H, int OUT_W, int B, int C, int KH, int KW, int STEP_H, int STEP_W>
void MaxpoolScalarBHWC<TT, INP_H, INP_W, OUT_H, OUT_W, B, C, KH, KW, STEP_H, STEP_W>::filter(
  input_window<TT>* in,      // BHWC (1x24x24x6)
  output_window<TT>* out     // BPQC (1x12x12x6)
) {
  PROFILE_HEADER2;

  const TT min = std::numeric_limits<TT>::lowest();

  for (int b = 0; b < B; b++) {
    for (int h = 0; h < OUT_H; h++) {
      for (int w = 0; w < OUT_W; w++) {

        TT arr[C] = {min};
        for (int p = 0; p < KH; p++) {
          for (int q = 0; q < KW; q++) {
            for (int c = 0; c < C; c++) {
              TT a = window_readincr(in);
              arr[c] = (arr[c] < a) ? a : arr[c];
            }
          }
          window_incr(in, C*(-KW+INP_W)); // go back KW, go down 1
        }
        
        for (int c = 0; c < C; c++)
          window_writeincr(out, arr[c]);

        window_incr(in, C*(-KH*INP_W + STEP_W)); // go up KH, go right STEP_W (next pos)
      }
      window_incr(in, C*(-OUT_W*STEP_W + STEP_H*INP_W)); // go down STEP_H, go left OUT_W*STEP_W, account for padding
    }
  }

  POOL_PROFILE_FOOTER("MaxpoolScalarBHWC");
}


template <typename TT, int INP_H, int INP_W, int OUT_H, int OUT_W, int B, int C, int KH, int KW, int STEP_H, int STEP_W>
void MaxpoolScalarBCHW<TT, INP_H, INP_W, OUT_H, OUT_W, B, C, KH, KW, STEP_H, STEP_W>::filter(
  input_window<TT>* in,      // BCHW (1x6x24x24)
  output_window<TT>* out     // BCPQ (1x6x12x12)
) {
  PROFILE_HEADER2;

  const TT min = std::numeric_limits<TT>::lowest();

  for (int b = 0; b < B; b++) {
    for (int c = 0; c < C; c++) {
      for (int h = 0; h < OUT_H; h++) {
        for (int w = 0; w < OUT_W; w++) {

          TT c = min;
          for (int p = 0; p < KH; p++) {
            for (int q = 0; q < KW; q++) {
              TT a = window_readincr(in);
              c = (a > c) ? a : c;
            }
            window_incr(in, -KW+INP_W); // left KW, down 1
          }
          window_incr(in, -KH*INP_W + STEP_W); // up KH, right STEP_W
          window_writeincr(out, c);
        } // W
        window_incr(in, -OUT_W*STEP_W + STEP_H*INP_W); // left OUT_W*STEP_W, down STEP_H
      } // H
      window_incr(in, (INP_H - OUT_H*STEP_H)*INP_W);
    } // C
  } // B

  POOL_PROFILE_FOOTER("MaxpoolScalarBCHW");
}


template <typename TT, int INP_H, int INP_W, int OUT_H, int OUT_W, int B, int C, int KH, int KW, int STEP_H, int STEP_W>
void Maxpool2x2FloatBCHW<TT, INP_H, INP_W, OUT_H, OUT_W, B, C, KH, KW, STEP_H, STEP_W>::filter(
  input_window<float>* in_window,      // BCHW (1x6x24x24)
  output_window<float>* out_window     // BCPQ (1x6x12x12)
) {
  PROFILE_HEADER2;

  const float min = std::numeric_limits<float>::lowest();

  v8float *in0 = (v8float *) in_window->ptr + 0 * INP_W/8;
  v8float *in1 = (v8float *) in_window->ptr + 1 * INP_W/8;
  v8float *in2 = (v8float *) in_window->ptr + 2 * INP_W/8;
  v8float *in3 = (v8float *) in_window->ptr + 3 * INP_W/8;
  v16float v = null_v16float();

  for (int b = 0; b < B; b++) {
    for (int c = 0; c < C; c++) {
      for (int h = 0; h < INP_H; h+=4) {
        for (int w = 0; w < INP_W; w+=8) {  // computes 2x4 cells with 4x8 cells

          v8float res = aie::broadcast<float, 8>(min);
          v = upd_w(v, 0, *in0);
          v = upd_w(v, 1, *in2);
          res = fpmax(res, v, 0, 0xeca86420);
          res = fpmax(res, v, 0, 0xfdb97531);
          
          v = upd_w(v, 0, *in1);
          v = upd_w(v, 1, *in3);
          res = fpmax(res, v, 0, 0xeca86420);
          res = fpmax(res, v, 0, 0xfdb97531);

          window_write(out_window, ext_v(res, 0));
          window_incr(out_window, OUT_W);
          window_write(out_window, ext_v(res, 1));
          window_incr(out_window, -OUT_W+4);
          
          in0++;
          in1++;
          in2++;
          in3++;
        } // W
        in0 += 4*INP_W/8 - INP_W/8; // account for padding
        in1 += 4*INP_W/8 - INP_W/8;
        in2 += 4*INP_W/8 - INP_W/8;
        in3 += 4*INP_W/8 - INP_W/8;
        window_incr(out_window, 2*OUT_W - INP_W/KW);
      } // H
    } // C
  } // B

  POOL_PROFILE_FOOTER("Maxpool2x2FloatBCHW");
}


/**
 * max32 (v64int16 xbuff, 
 *  int xstart, unsigned int xoffsets, unsigned int xoffsets_hi, unsigned int xsquare, 
 *  int ystart, unsigned int yoffsets, unsigned int yoffsets_hi, unsigned int ysquare)
 * 
 * 0x06...00, 0x0e...08 => 0 1 2 3 ... 12 13 14 15, 16 17 18 19 ... 28 29 30 31
 * max32(v, 0, 0x06040200, 0x0e0c0a08, 0x3210, 32, 0x06040200, 0x0e0c0a08, 0x3210); // first 32 with next 32
 * problem: offsets index <= 32, each 4b selects 2 adjacent lanes
 * 
 * 128 int16 max
 */
template <typename TT, int INP_H, int INP_W, int OUT_H, int OUT_W, int B, int C, int KH, int KW, int STEP_H, int STEP_W>
void Maxpool2x2Int8BCHW<TT, INP_H, INP_W, OUT_H, OUT_W, B, C, KH, KW, STEP_H, STEP_W>::filter(
  input_window<TT>* in_window,      // BCHW (1x6x24x24)
  output_window<TT>* out_window     // BCPQ (1x6x12x12)
) {
  PROFILE_HEADER2;

  using v16 = typename std::conditional<(std::is_same<TT, int8_t>::value), v16int8, v16uint8>::type;
  const TT min = std::numeric_limits<TT>::lowest();
  TT *out_ptr = (TT *) out_window->ptr;

  v16 *in0 = (v16 *) in_window->ptr + 0 * INP_W/16;
  v16 *in1 = (v16 *) in_window->ptr + 1 * INP_W/16;
  v64int16 v = null_v64int16();

  for (int b = 0; b < B; b++) {
    for (int c = 0; c < C; c++) {
      for (int h = 0; h < INP_H; h+=2) {
        for (int w = 0; w <= INP_W-16; w+=32) {  // computes 1x16 cells with 2x32 cells, stop at 16 left since INP_W%16=0

          v32int16 res = aie::broadcast<int16_t, 32>(min);
          v = upd_w(v, 0, unpack(*in0)); in0++;
          v = upd_w(v, 1, unpack(*in0)); in0++;
          v = upd_w(v, 2, unpack(*in1)); in1++;
          v = upd_w(v, 3, unpack(*in1)); in1++;
          
          // against row+1: 0 1 2 3 ... 28 29 30 31 x 32 33 34 35 ... 60 61 62 63
          res = max32(v, 0, 0x06040200, 0x0e0c0a08, 0x3210, 32, 0x06040200, 0x0e0c0a08, 0x3210);
          // against col+1: 0 2 4 6 ... 24 26 28 30 x 1 3 5 7 ... 25 27 29 31, (shuffle first for 16-bit addressing)
          res = shuffle32(res, 0, 0x06040200, 0x0e0c0a08, 0x3120); // 0213 4657 ... 28302931
          res = max32(res, 0, 0x1c181410, 0x00000000, 0x3210, 0, 0x1d191511, 0x00000000, 0x3210); // 0 1 4 5 ... 24 25 28 29 x 2 3 6 7 ... 26 27 30 31

          window_writeincr(out_window, ((aie::vector<int16_t,32>) res).extract<16>(0).pack<TT>());
        } // W

        if (RUN_16CHUNK) {  // computes 1x8 cells with 2x16 cells, handle last 16
          v32int16 res = aie::broadcast<int16_t, 32>(min);
          v = upd_w(v, 0, unpack(*in0)); in0++;
          v = upd_w(v, 2, unpack(*in1)); in1++;
          res = max32(v, 0, 0x06040200, 0x0e0c0a08, 0x3210, 32, 0x06040200, 0x0e0c0a08, 0x3210);
          res = shuffle32(res, 0, 0x06040200, 0x0e0c0a08, 0x3120);
          res = max32(res, 0, 0x1c181410, 0x00000000, 0x3210, 0, 0x1d191511, 0x00000000, 0x3210);

          // against col+1: 0 2 4 6 ... 24 26 28 30 x 1 3 5 7 ... 25 27 29 31, (shuffle first for 16-bit addressing)
          res = shuffle32(res, 0, 0x06040200, 0x0e0c0a08, 0x3120); // 0213 4657 ... 28302931
          res = max32(res, 0, 0x1c181410, 0x00000000, 0x3210, 0, 0x1d191511, 0x00000000, 0x3210); // 0 1 4 5 ... 24 25 28 29 x 2 3 6 7 ... 26 27 30 31

          window_writeincr(out_window, ((aie::vector<int16_t,32>) res).extract<16>(0).pack<TT>());
        } // W
        
        in0 += 2*INP_W/16 - (INP_W+15)/16; // account for padding
        in1 += 2*INP_W/16 - (INP_W+15)/16;
      } // H
    } // C
  } // B

  POOL_PROFILE_FOOTER("Maxpool2x2Int8BCHW");
}


template <typename TT, int INP_H, int INP_W, int OUT_H, int OUT_W, int B, int C, int KH, int KW, int STEP_H, int STEP_W>
void Maxpool3x3Int8BCHW<TT, INP_H, INP_W, OUT_H, OUT_W, B, C, KH, KW, STEP_H, STEP_W>::filter(
  input_window<TT>* in_window,      // BCHW (1x6x24x24)
  output_window<TT>* out_window     // BCPQ (1x6x12x12)
) {
  PROFILE_HEADER2;

  using v32 = typename std::conditional<(std::is_same<TT, int8_t>::value), v32int8, v32uint8>::type;
  const TT min = std::numeric_limits<TT>::lowest();

  TT *in = (TT *) in_window->ptr;
  v32int16 res = undef_v32int16();

  for (int b = 0; b < B; b++) {
    for (int c = 0; c < C; c++) {
      for (int h = 0; h < OUT_H; h++) {
        for (int w = 0; w < OUT_W; w+=16) {  // computes 1x16 cells with 3x32 cells, stop at 16 left since INP_W%16=0

          res = unpack(*(v32 *) in); in += INP_W;
          res = max32(res, 0, 0x06040200, 0x0e0c0a08, 0x3210, unpack(*(v32 *) in), 0, 0x06040200, 0x0e0c0a08, 0x3210); in += INP_W;   // row+1
          res = max32(res, 0, 0x06040200, 0x0e0c0a08, 0x3210, unpack(*(v32 *) in), 0, 0x06040200, 0x0e0c0a08, 0x3210); in += 32-2*INP_W; // row+2
          
          // against col+1: 0 2 4 6 ... 24 26 28 30 x 1 3 5 7 ... 25 27 29 31, (shuffle first for 16-bit addressing)
          res = shuffle32(res, 0, 0x06040200, 0x0e0c0a08, 0x3120); // 0213 4657 ... 28302931
          res = max32(res, 0, 0x1c181410, 0x1c181410, 0x3210, 0, 0x1d191511, 0x1c181410, 0x3210); // idx 0145 -> orig 0246 x idx 2367 -> orig 1357
          
          // against col+2: 2 4 6 8 ... 26 28 30 32 x 16-long result
          res = upd_w(res, 1, aie::shuffle_down((aie::vector<int16_t,16>) ext_w(res, 1), 1));

          aie::vector<TT,16> last_pool_v = aie::broadcast<TT,16>(min); // handle 33rd element for 3x3 kernel
          TT *last_pool_ptr = (TT *) &last_pool_v;
          *last_pool_ptr = *in; last_pool_ptr++;
          *last_pool_ptr = *(in + INP_W); last_pool_ptr++;
          *last_pool_ptr =  *(in + 2*INP_W);
          TT last_pool = aie::reduce_max(last_pool_v);
          res = upd_elem(res, 31, last_pool);
          res = max32(res, 0, 0x06040200, 0x00000000, 0x3210, 0, 0x0e0c0a08, 0x00000000, 0x3210);

          window_writeincr(out_window, ((aie::vector<int16_t,32>) res).extract<16>(0).pack<TT>());
        } // W
        
        in += INP_W;
      } // H
      in += (INP_H - OUT_H*STEP_H) * INP_W;
    } // C
  } // B

  POOL_PROFILE_FOOTER("Maxpool3x3Int8BCHW");
}


template <typename TT, int INP_H, int INP_W, int OUT_H, int OUT_W, int B, int C, int KH, int KW, int STEP_H, int STEP_W>
void AvgpoolScalarBCHW<TT, INP_H, INP_W, OUT_H, OUT_W, B, C, KH, KW, STEP_H, STEP_W>::filter(
  input_window<TT>* in,      // BCHW (1x6x24x24)
  output_window<TT>* out     // BCPQ (1x6x12x12)
) {
  PROFILE_HEADER2;

  TT div_factor = inv(KH*KW);

  for (int b = 0; b < B; b++) {
    for (int c = 0; c < C; c++) {
      for (int h = 0; h < OUT_H; h++) {
        for (int w = 0; w < OUT_W; w++) {

          TT sum = 0;
          for (int p = 0; p < KH; p++) {
            for (int q = 0; q < KW; q++) {
              TT a = window_readincr(in);
              sum += a;
            }
            window_incr(in, -KW+INP_W); // left KW, down 1
          }
          window_incr(in, -KH*INP_W + KW); // up KH, right KW
          window_writeincr(out, sum * div_factor);
        } // W
        window_incr(in, KH*INP_W - OUT_W*KW); // left OUT_W*KW, down KH
      } // H
    } // C
  } // B

  POOL_PROFILE_FOOTER("AvgpoolScalarBCHW");
}