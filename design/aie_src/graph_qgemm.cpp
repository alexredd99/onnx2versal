#include "graph_qgemm.h"
#include "graph_utils.h"


template <template<int, int, int, int> class QGEMM, int M, int K, int N, int NPAD>
class QGemmGraphTest : public adf::graph {

  private:
    QGemmGraph<QGEMM, M, K, N, NPAD> g;

  public:
    adf::input_plio plin[1];
    adf::output_plio plout[1];

    QGemmGraphTest(
      const std::string& id,
      std::vector<int8_t> weights,
      std::vector<int32_t> bias,
      float x_scale,
      float w_scale,
      float y_scale,
      int8_t x_zero_point,
      int8_t w_zero_point,
      int8_t y_zero_point,
      const std::string& INP_TXT,
      const std::string& OUT_TXT = "qgemm_out.txt"
    ): g(weights, bias, x_scale, w_scale, y_scale, x_zero_point, w_zero_point, y_zero_point) { 
      plin[0] = adf::input_plio::create("plin0_qgemm"+id+"_input", PLIO64_ARG(INP_TXT));
      plout[0] = adf::output_plio::create("plout0_qgemm"+id+"_output", PLIO64_ARG(OUT_TXT));
      adf::connect<adf::window<M*K>> (plin[0].out[0], g.pin[0]);
      adf::connect<adf::window<M*NPAD>> (g.pout[0], plout[0].in[0]);
    }

};

// instance to be compiled and used in host within xclbin
// mknk has no padding, mkkn pads N in 128-bit chunks for vector kernel
std::vector<int8_t> weights {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 0, 0, 0, 0, 0, 0, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 0, 0, 0, 0, 0, 0, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 0, 0, 0, 0, 0, 0, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 0, 0, 0, 0, 0, 0, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 0, 0, 0, 0, 0, 0, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 0, 0, 0, 0, 0, 0, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 0, 0, 0, 0, 0, 0, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 0, 0, 0, 0, 0, 0, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 0, 0, 0, 0, 0, 0, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 0, 0, 0, 0, 0, 0, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 0, 0, 0, 0, 0, 0, 120, 121, 122, 123, 124, 125, 126, 127, -128, -127, 0, 0, 0, 0, 0, 0, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, 0, 0, 0, 0, 0, 0, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, 0, 0, 0, 0, 0, 0, -106, -105, -104, -103, -102, -101, -100, -99, -98, -97, 0, 0, 0, 0, 0, 0, -96, -95, -94, -93, -92, -91, -90, -89, -88, -87, 0, 0, 0, 0, 0, 0, -86, -85, -84, -83, -82, -81, -80, -79, -78, -77, 0, 0, 0, 0, 0, 0, -76, -75, -74, -73, -72, -71, -70, -69, -68, -67, 0, 0, 0, 0, 0, 0, -66, -65, -64, -63, -62, -61, -60, -59, -58, -57, 0, 0, 0, 0, 0, 0, -56, -55, -54, -53, -52, -51, -50, -49, -48, -47, 0, 0, 0, 0, 0, 0, -46, -45, -44, -43, -42, -41, -40, -39, -38, -37, 0, 0, 0, 0, 0, 0, -36, -35, -34, -33, -32, -31, -30, -29, -28, -27, 0, 0, 0, 0, 0, 0, -26, -25, -24, -23, -22, -21, -20, -19, -18, -17, 0, 0, 0, 0, 0, 0, -16, -15, -14, -13, -12, -11, -10, -9, -8, -7, 0, 0, 0, 0, 0, 0, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 0, 0, 0, 0, 0, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0, 0, 0, 0, 0, 0, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 0, 0, 0, 0, 0, 0, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 0, 0, 0, 0, 0, 0, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 0, 0, 0, 0, 0, 0, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 0, 0, 0, 0, 0, 0, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 0, 0, 0, 0, 0, 0, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 0, 0, 0, 0, 0, 0, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 0, 0, 0, 0, 0, 0, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 0, 0, 0, 0, 0, 0, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 0, 0, 0, 0, 0, 0, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 0, 0, 0, 0, 0, 0, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 0, 0, 0, 0, 0, 0, 124, 125, 126, 127, -128, -127, -126, -125, -124, -123, 0, 0, 0, 0, 0, 0, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, 0, 0, 0, 0, 0, 0, -112, -111, -110, -109, -108, -107, -106, -105, -104, -103, 0, 0, 0, 0, 0, 0, -102, -101, -100, -99, -98, -97, -96, -95, -94, -93, 0, 0, 0, 0, 0, 0, -92, -91, -90, -89, -88, -87, -86, -85, -84, -83, 0, 0, 0, 0, 0, 0, -82, -81, -80, -79, -78, -77, -76, -75, -74, -73, 0, 0, 0, 0, 0, 0, -72, -71, -70, -69, -68, -67, -66, -65, -64, -63, 0, 0, 0, 0, 0, 0, -62, -61, -60, -59, -58, -57, -56, -55, -54, -53, 0, 0, 0, 0, 0, 0, -52, -51, -50, -49, -48, -47, -46, -45, -44, -43, 0, 0, 0, 0, 0, 0, -42, -41, -40, -39, -38, -37, -36, -35, -34, -33, 0, 0, 0, 0, 0, 0, -32, -31, -30, -29, -28, -27, -26, -25, -24, -23, 0, 0, 0, 0, 0, 0, -22, -21, -20, -19, -18, -17, -16, -15, -14, -13, 0, 0, 0, 0, 0, 0, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, 0, 0, 0, 0, 0, 0, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 0, 0, 0, 0, 0, 0, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 0, 0, 0, 0, 0, 0, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 0, 0, 0, 0, 0, 0, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 0, 0, 0, 0, 0, 0, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 0, 0, 0, 0, 0, 0, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 0, 0, 0, 0, 0, 0, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 0, 0, 0, 0, 0, 0, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 0, 0, 0, 0, 0, 0, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 0, 0, 0, 0, 0, 0, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 0, 0, 0, 0, 0, 0, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 0, 0, 0, 0, 0, 0, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 0, 0, 0, 0, 0, 0, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, 0, 0, 0, 0, 0, 0, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, 0, 0, 0, 0, 0, 0, -108, -107, -106, -105, -104, -103, -102, -101, -100, -99, 0, 0, 0, 0, 0, 0, -98, -97, -96, -95, -94, -93, -92, -91, -90, -89, 0, 0, 0, 0, 0, 0, -88, -87, -86, -85, -84, -83, -82, -81, -80, -79, 0, 0, 0, 0, 0, 0, -78, -77, -76, -75, -74, -73, -72, -71, -70, -69, 0, 0, 0, 0, 0, 0, -68, -67, -66, -65, -64, -63, -62, -61, -60, -59, 0, 0, 0, 0, 0, 0, -58, -57, -56, -55, -54, -53, -52, -51, -50, -49, 0, 0, 0, 0, 0, 0, -48, -47, -46, -45, -44, -43, -42, -41, -40, -39, 0, 0, 0, 0, 0, 0, -38, -37, -36, -35, -34, -33, -32, -31, -30, -29, 0, 0, 0, 0, 0, 0, -28, -27, -26, -25, -24, -23, -22, -21, -20, -19, 0, 0, 0, 0, 0, 0, -18, -17, -16, -15, -14, -13, -12, -11, -10, -9, 0, 0, 0, 0, 0, 0, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 0, 0, 0, 0, 0, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 0, 0, 0, 0, 0, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0, 0, 0, 0, 0, 0, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 0, 0, 0, 0, 0, 0, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 0, 0, 0, 0, 0, 0, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 0, 0, 0, 0, 0, 0, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 0, 0, 0, 0, 0, 0};
std::vector<int32_t> bias {0, 166, 333, 500, 666, 833, 1000, 1166, 1333, 1500, 0, 0, 0, 0, 0, 0};

std::vector<int8_t> lenet_w {69, -30, 10, -47, -12, 7, -22, -18, -7, 37, 0, 0, 0, 0, 0, 0, 127, -33, 32, -54, -40, 38, 19, 73, -35, -23, 0, 0, 0, 0, 0, 0, 17, -39, -13, 32, -67, 47, -48, 46, 47, 66, 0, 0, 0, 0, 0, 0, -33, -47, 10, 30, -69, 84, 6, -56, 59, 36, 0, 0, 0, 0, 0, 0, 51, 38, 13, -33, 44, -62, -18, -45, -62, -10, 0, 0, 0, 0, 0, 0, 52, 5, 16, -34, -8, -47, -29, -8, -12, -39, 0, 0, 0, 0, 0, 0, 57, 75, -67, -65, 43, 12, -23, 4, -48, -30, 0, 0, 0, 0, 0, 0, 2, -4, 17, -26, 25, 19, -25, 13, 22, -28, 0, 0, 0, 0, 0, 0, -6, -37, 56, 72, 18, -18, 11, -28, -24, -42, 0, 0, 0, 0, 0, 0, -1, -14, -10, 16, 62, -31, 19, 24, -31, -32, 0, 0, 0, 0, 0, 0, -49, 7, 2, -30, -13, -24, 0, 10, 24, 30, 0, 0, 0, 0, 0, 0, -51, -39, -43, -22, -30, -35, 24, -25, 16, -21, 0, 0, 0, 0, 0, 0, 76, 76, 21, -13, -13, 82, 1, -43, -36, -12, 0, 0, 0, 0, 0, 0, -42, 8, 59, -74, -19, -23, 45, -47, -61, -67, 0, 0, 0, 0, 0, 0, 30, 1, 6, -6, -56, 59, 12, -64, 75, -84, 0, 0, 0, 0, 0, 0, -32, 26, 54, 24, 1, -45, 30, 50, -18, 38, 0, 0, 0, 0, 0, 0, -85, 103, 42, 6, 16, -71, 4, -38, -30, -11, 0, 0, 0, 0, 0, 0, 36, 26, -58, 48, 75, 106, -33, 38, -7, 85, 0, 0, 0, 0, 0, 0, -21, 68, -54, -90, 38, 12, -42, -67, 36, -61, 0, 0, 0, 0, 0, 0, -1, 20, -30, -34, -8, -23, -38, 15, 46, -30, 0, 0, 0, 0, 0, 0, 70, -40, 5, -64, -94, -41, -18, -67, -14, -75, 0, 0, 0, 0, 0, 0, -38, -44, 32, 51, 52, -36, 3, 63, 71, 113, 0, 0, 0, 0, 0, 0, -64, 78, -42, 6, -22, -50, -23, -31, 77, -10, 0, 0, 0, 0, 0, 0, 35, -24, -7, 60, -32, 88, 18, 60, -7, 39, 0, 0, 0, 0, 0, 0, -77, -14, -10, 18, 67, 31, -19, -58, -43, 51, 0, 0, 0, 0, 0, 0, 50, 18, -18, -1, -34, -38, -21, -22, -57, -36, 0, 0, 0, 0, 0, 0, -46, -23, -15, 42, 43, -42, -20, -8, 35, 5, 0, 0, 0, 0, 0, 0, 4, -33, 12, -1, -23, -10, -31, 22, -17, 38, 0, 0, 0, 0, 0, 0, -9, 65, 14, 36, 79, -1, 27, -87, -39, 26, 0, 0, 0, 0, 0, 0, -53, 39, 78, 16, -25, -47, -7, 46, -42, -29, 0, 0, 0, 0, 0, 0, -79, 63, -28, 39, 47, -66, -35, -14, -29, 24, 0, 0, 0, 0, 0, 0, 92, -57, 41, 40, 14, -37, 18, 93, -15, -69, 0, 0, 0, 0, 0, 0, -28, -47, -13, -8, 37, -17, 17, 12, -2, -24, 0, 0, 0, 0, 0, 0, -36, 109, 92, 3, -73, -53, 23, 88, 34, 8, 0, 0, 0, 0, 0, 0, 6, 36, -37, 15, 34, -50, 4, 43, 37, 75, 0, 0, 0, 0, 0, 0, -61, -15, 58, -54, -24, 39, -23, -13, -59, -13, 0, 0, 0, 0, 0, 0, 14, 55, 14, -14, 20, -20, -17, 29, -58, 27, 0, 0, 0, 0, 0, 0, -71, -98, 63, -84, -3, -94, 5, -77, 7, -54, 0, 0, 0, 0, 0, 0, 66, -62, 11, 52, -17, 2, -28, -23, 61, -29, 0, 0, 0, 0, 0, 0, 9, 31, -30, 14, 6, 58, -48, 53, 37, 13, 0, 0, 0, 0, 0, 0, 40, -33, -45, 13, -60, 5, -11, 91, -50, -35, 0, 0, 0, 0, 0, 0, -11, -54, 24, -12, 40, -2, -30, 36, 41, 70, 0, 0, 0, 0, 0, 0, 6, -38, -55, -35, 82, 15, 26, 35, -34, 40, 0, 0, 0, 0, 0, 0, 39, -22, -47, -45, 23, 36, 34, 12, -44, -21, 0, 0, 0, 0, 0, 0, -46, 102, -4, 68, -21, -58, 5, 18, 115, -53, 0, 0, 0, 0, 0, 0, 10, -79, -24, 85, 71, 79, -1, -20, -57, 58, 0, 0, 0, 0, 0, 0, 59, 2, -8, 35, -36, 25, -47, 86, -34, -64, 0, 0, 0, 0, 0, 0, -46, 0, 75, 23, -32, -11, 18, 10, -40, 33, 0, 0, 0, 0, 0, 0, -105, 52, -43, 58, 13, 22, 15, -36, 108, -16, 0, 0, 0, 0, 0, 0, -86, 35, 5, 92, -43, 15, -23, -78, -43, -35, 0, 0, 0, 0, 0, 0, -34, -26, 20, 12, 31, -45, 45, 14, 29, -30, 0, 0, 0, 0, 0, 0, -86, 108, -30, -3, -13, -71, -16, 87, -10, 75, 0, 0, 0, 0, 0, 0, 76, -48, 67, 105, -71, 75, -19, 17, 34, 19, 0, 0, 0, 0, 0, 0, 42, 39, 2, 37, -13, 41, -35, -12, 38, -28, 0, 0, 0, 0, 0, 0, -64, 117, -42, 35, 89, -2, 34, 38, -55, 97, 0, 0, 0, 0, 0, 0, 33, -93, 8, 8, -7, 39, -31, -40, 32, 30, 0, 0, 0, 0, 0, 0, -10, -69, 68, -33, 7, 14, 43, 33, 81, 78, 0, 0, 0, 0, 0, 0, 32, -32, 24, 27, 29, 24, -16, 39, 38, 23, 0, 0, 0, 0, 0, 0, -35, 3, -3, 49, 16, 0, -11, 25, -16, 4, 0, 0, 0, 0, 0, 0, -15, -110, -8, -10, -19, -6, -48, -59, -35, -25, 0, 0, 0, 0, 0, 0, 44, -36, 13, 37, -86, 69, 49, -52, 5, 32, 0, 0, 0, 0, 0, 0, -33, 10, -4, 29, 27, -21, 18, -45, 0, -8, 0, 0, 0, 0, 0, 0, 29, -4, 4, 100, -97, 33, -10, -73, -34, -73, 0, 0, 0, 0, 0, 0, -70, 43, -4, -17, 21, -29, 5, -67, 30, 63, 0, 0, 0, 0, 0, 0, -11, -76, -52, 4, -15, 77, -41, -91, 84, 34, 0, 0, 0, 0, 0, 0, -13, -29, -48, 81, -25, -9, 2, 30, -55, -12, 0, 0, 0, 0, 0, 0, 16, 51, -10, -48, -16, 34, 27, 46, -13, 32, 0, 0, 0, 0, 0, 0, -49, -99, 9, -15, 93, 36, -36, -31, -40, -1, 0, 0, 0, 0, 0, 0, 3, -44, 32, -75, -16, -89, -17, 20, 54, 15, 0, 0, 0, 0, 0, 0, -52, -48, 45, -29, 46, 23, -13, 82, 25, 106, 0, 0, 0, 0, 0, 0, 37, -30, 8, 40, -16, -31, -14, -16, 0, -15, 0, 0, 0, 0, 0, 0, 4, -42, -56, -3, 24, -49, 24, -21, -5, -54, 0, 0, 0, 0, 0, 0, -43, 48, 14, 2, 27, 41, -25, 19, 12, -19, 0, 0, 0, 0, 0, 0, -61, 61, -41, -28, -4, 57, 42, 74, 14, -29, 0, 0, 0, 0, 0, 0, -49, -14, -41, -74, -56, -29, 4, 49, 73, 63, 0, 0, 0, 0, 0, 0, 11, 12, 11, 30, -28, 46, 10, 5, 38, 43, 0, 0, 0, 0, 0, 0, -66, -31, -28, -67, 96, -63, 0, 15, 25, 20, 0, 0, 0, 0, 0, 0, -43, -15, 35, -45, -23, -24, -5, -40, 29, 45, 0, 0, 0, 0, 0, 0, -19, -63, 31, 17, -36, -37, -10, -72, -5, -105, 0, 0, 0, 0, 0, 0, 20, -75, -12, -1, -36, -17, 9, -59, -55, -74, 0, 0, 0, 0, 0, 0, 63, -58, 66, 57, -94, -22, -9, 20, -4, 29, 0, 0, 0, 0, 0, 0, -19, 89, 21, 76, -34, 103, -46, 3, 99, -38, 0, 0, 0, 0, 0, 0, 82, -81, -39, -94, 70, -23, 38, 49, -51, 48, 0, 0, 0, 0, 0, 0, -17, -17, 23, -37, -35, -50, -29, 20, 34, -28, 0, 0, 0, 0, 0, 0};
std::vector<int32_t> lenet_b {-353, 2846, -1128, -672, -636, 613, 44, 854, 508, 620, 0, 0, 0, 0, 0, 0};

QGemmGraphTest<QgemmScalar, 1, 84, 10, 16> qgemmScalar(
  "qgemmScalar", weights, bias, 0.004, 0.003, 0.002, 25, 0, 19,
  "qgemm_int8in.txt", "qgemm_int8out_qgemmScalar.txt");

QGemmGraphTest<QgemmVector, 1, 84, 10, 16> qgemmVector(
  "qgemmVector", weights, bias, 0.004, 0.003, 0.002, 25, 0, 19,
  "qgemm_int8in.txt", "qgemm_int8out_qgemmVector.txt");

QGemmGraphTest<QgemmVector, 1, 84, 10, 16> lenet(
  "lenet", lenet_w, lenet_b, 0.04296031594276428, 0.0022037059534341097, 0.058117881417274475, -128, 0, -128,
  "k8qgemm_in.txt", "k8qgemm_goldenout.txt");


#ifdef __X86SIM__
int main(int argc, char ** argv) {
  adfCheck(qgemmScalar.init(), "init qgemmScalar");
  adfCheck(qgemmScalar.run(ITER_CNT), "run qgemmScalar");
	adfCheck(qgemmScalar.end(), "end qgemmScalar");

  adfCheck(qgemmVector.init(), "init qgemmVector");
  adfCheck(qgemmVector.run(ITER_CNT), "run qgemmVector");
	adfCheck(qgemmVector.end(), "end qgemmVector");

  adfCheck(lenet.init(), "init lenet");
  adfCheck(lenet.run(ITER_CNT), "run lenet");
	adfCheck(lenet.end(), "end lenet");
  return 0;
}
#endif


#ifdef __AIESIM__
int main(int argc, char ** argv) {
  adfCheck(qgemmScalar.init(), "init qgemmScalar");
  get_graph_throughput_by_port(qgemmScalar, "plout[0]", qgemmScalar.plout[0], 1*16, sizeof(int8_t), ITER_CNT);
	adfCheck(qgemmScalar.end(), "end qgemmScalar");

  adfCheck(qgemmVector.init(), "init qgemmVector");
  get_graph_throughput_by_port(qgemmVector, "plout[0]", qgemmVector.plout[0], 1*16, sizeof(int8_t), ITER_CNT);
	adfCheck(qgemmVector.end(), "end qgemmVector");

  adfCheck(lenet.init(), "init lenet");
  get_graph_throughput_by_port(lenet, "plout[0]", lenet.plout[0], 1*16, sizeof(int8_t), ITER_CNT);
	adfCheck(lenet.end(), "end lenet");
  return 0;
}
#endif
