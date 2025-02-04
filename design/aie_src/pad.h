#ifndef PAD_H_
#define PAD_H_

#include <type_traits>
#include <adf.h>
#include <assert.h>


/** 
 * @defgroup PadKernels
 * @ingroup Pad
 * 
 * @brief See padding at https://github.com/onnx/onnx/blob/main/docs/Operators.md#Conv
 * 
 * @{
 */

/**
 * @brief Vector implementation for Float Pad2D
 * Pad2DStreamFloat<f,2,30,30,32,1,1,1,1> total = 2304
 */
template <typename TT, int B, int INP_H, int INP_W, int INP_W_PAD, int H0, int H1, int W0, int W1>
class Pad2DStreamFloat {
  private:
    static constexpr int OUT_H = INP_H + H0 + H1;
    static constexpr int OUT_W = INP_W + W0 + W1;
    TT pad_value;
    
  public:
    Pad2DStreamFloat(
      TT pad_value = 0
    ): pad_value(pad_value) {};

    void filter(
      input_stream<TT>* in,
      output_stream<TT>* out
    );

    static void registerKernelClass() {
      static_assert((std::is_same<TT, float>::value));
      REGISTER_FUNCTION(Pad2DStreamFloat::filter);
    }
};


/**
 * @brief Vector implementation for Int8 Pad2D
 * Pad2DStreamInt8<a,2,30,30,32,1,1,1,1> total = 1885 for v64int16
 */
template <typename TT, int B, int INP_H, int INP_W, int INP_W_PAD, int H0, int H1, int W0, int W1>
class Pad2DStreamInt8 {
  private:
    static constexpr int OUT_H = INP_H + H0 + H1;
    static constexpr int OUT_W = INP_W + W0 + W1;
    TT pad_value;
    
  public:
    Pad2DStreamInt8(
      TT pad_value = 0
    ): pad_value(pad_value) {};

    void filter(
      input_stream<TT>* in,
      output_stream<TT>* out
    );

    static void registerKernelClass() {
      static_assert((std::is_same<TT, int8_t>::value) || (std::is_same<TT, uint8_t>::value));
      static_assert((B*OUT_H*OUT_W) % 4 == 0);
      static_assert(INP_W_PAD % 16 == 0);
      REGISTER_FUNCTION(Pad2DStreamInt8::filter);
    }
};


/**
 * @brief Vector implementation for Pad2D using windows
 * Pad2DWindowScalar<f,2,30,30,32,1,1,1,1> total = 1549 * 2
 * Pad2DWindowScalar<a,2,30,30,32,1,1,1,1> total = 6924 * 2
 */
template <typename TT, int B, int INP_H, int INP_W, int INP_W_PAD, int H0, int H1, int W0, int W1>
class Pad2DWindowScalar {
  private:
    static constexpr int OUT_H = INP_H + H0 + H1;
    static constexpr int OUT_W = INP_W + W0 + W1;
    TT pad_value;
    
  public:
    Pad2DWindowScalar(
      TT pad_value = 0
    ): pad_value(pad_value) {};

    void filter(
      input_window<TT>* in,
      output_window<TT>* out
    );

    static void registerKernelClass() {
      REGISTER_FUNCTION(Pad2DWindowScalar::filter);
    }
};


/**
 * @brief Vector implementation for Pad2D using input stream and output window
 * Pad2DStream2WindowInt8<a,2,30,30,32,1,1,1,1> start = 32390,end = 39864,total = 7474 * 2
 */
template <typename TT, int B, int INP_H, int INP_W, int INP_W_PAD, int H0, int H1, int W0, int W1>
class Pad2DStream2WindowInt8 {
  private:
    static constexpr int OUT_H = INP_H + H0 + H1;
    static constexpr int OUT_W = INP_W + W0 + W1;
    TT pad_value;
    
  public:
    Pad2DStream2WindowInt8(
      TT pad_value = 0
    ): pad_value(pad_value) {};

    void filter(
      input_stream<TT>* in,
      output_window<TT>* out
    );

    static void registerKernelClass() {
      static_assert((std::is_same<TT, int8_t>::value) || (std::is_same<TT, uint8_t>::value));
      static_assert(INP_W_PAD % 16 == 0);
      REGISTER_FUNCTION(Pad2DStream2WindowInt8::filter);
    }
};
/** @}*/


#endif // PAD_H_
