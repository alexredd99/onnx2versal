#include "graph_conv.h"
#include "graph_utils.h"


template <
  template<int, int, int, int, int, int, int, int, int, int, int, int, int> class CONV, 
  template<typename, int, int, int, int> class CONCAT, 
  int IS_BCHW, int MCHUNK, 
  int INP_H, int INP_W, int OUT_W, int OUT_W_PAD, int STEP_H, int STEP_W,
  int B, int C, int M, int KH, int KW, int GROUP, int IS_RELU,
  int H0 = 0, int H1 = 0, int W0 = 0, int W1 = 0>
class ConvReluChunkMGraphTest : public adf::graph {

  private:
    typedef ConvReluChunkMGraph<CONV, CONCAT, IS_BCHW, MCHUNK, 
                                INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W,
                                B, C, M, KH, KW, GROUP, IS_RELU,
                                H0, H1, W0, W1> Graph;
    Graph g;
    static constexpr int OUT_H = Graph::OUT_H;

  public:
    adf::input_plio plin[1];
    adf::output_plio plout[1];

    ConvReluChunkMGraphTest(
      const std::string& id,
      std::vector<float> weights,
      std::vector<float> bias,
      const std::string& INP_TXT,
      const std::string& OUT_TXT = "conv_out.txt"
    ): g(weights, bias) { 
      plin[0] = adf::input_plio::create("plin0_conv"+id+"_input", PLIO64_ARG(INP_TXT));
      plout[0] = adf::output_plio::create("plout0_conv"+id+"_output", PLIO64_ARG(OUT_TXT));
      adf::connect<adf::window<B*C*INP_H*INP_W*4>> (plin[0].out[0], g.pin[0]);
      adf::connect<adf::stream> (g.pout[0], plout[0].in[0]);
    }
};


template <
  template<typename, int, int, int, int> class SPLIT,
  template<int, int, int, int, int, int, int, int, int, int, int, int, int> class CONV, 
  template<typename, int, int, int, int> class CONCAT, 
  int HCHUNK,
  int INP_H, int INP_W, int OUT_W, int OUT_W_PAD, int STEP_H, int STEP_W,
  int B, int C, int M, int KH, int KW, int GROUP, int IS_RELU,
  int H0 = 0, int H1 = 0, int W0 = 0, int W1 = 0>
class ConvReluChunkHGraphTest : public adf::graph {

  private:
    typedef ConvReluChunkHGraph<SPLIT, CONV, CONCAT, HCHUNK, 
                                INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W,
                                B, C, M, KH, KW, GROUP, IS_RELU,
                                H0, H1, W0, W1> Graph;
    Graph g;

  public:
    adf::input_plio plin[1];
    adf::output_plio plout[1];
    adf::input_gmio gmio_w;

    ConvReluChunkHGraphTest(
      const std::string& id,
      std::vector<float> bias,
      const std::string& INP_TXT,
      const std::string& OUT_TXT = "conv_out.txt"
    ): g(bias) { 
      plin[0] = adf::input_plio::create("plin0_conv"+id+"_input", PLIO64_ARG(INP_TXT));
      plout[0] = adf::output_plio::create("plout0_conv"+id+"_output", PLIO64_ARG(OUT_TXT));
      gmio_w = adf::input_gmio::create("gmio0_gemm"+id+"_w", 64, 1000);
      adf::connect<adf::stream> (plin[0].out[0], g.pin[0]);
      adf::connect<adf::stream> (gmio_w.out[0], g.pin[1]);
      adf::connect<adf::stream> (g.pout[0], plout[0].in[0]);
    }
};


template <
  template<typename, int, int, int, int> class SPLIT,
  template<int, int, int, int, int, int, int, int, int, int, int, int, int> class CONV, 
  template<typename, int, int, int, int> class CONCAT, 
  int HCHUNK,
  int INP_H, int INP_W, int OUT_W, int OUT_W_PAD, int STEP_H, int STEP_W,
  int B, int C, int M, int KH, int KW, int GROUP, int IS_RELU,
  int H0 = 0, int H1 = 0, int W0 = 0, int W1 = 0>
class ConvReluChunkHStreamGraphTest : public adf::graph {

  private:
    typedef ConvReluChunkHStreamGraph<SPLIT, CONV, CONCAT, HCHUNK, 
                                      INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W,
                                      B, C, M, KH, KW, GROUP, IS_RELU,
                                      H0, H1, W0, W1> Graph;
    Graph g;

  public:
    adf::input_plio plin[1];
    adf::output_plio plout[1];
    adf::input_gmio gmio_w;

    ConvReluChunkHStreamGraphTest(
      const std::string& id,
      std::vector<float> bias,
      const std::string& INP_TXT,
      const std::string& OUT_TXT = "conv_out.txt"
    ): g(bias) { 
      plin[0] = adf::input_plio::create("plin0_conv"+id+"_input", PLIO64_ARG(INP_TXT));
      plout[0] = adf::output_plio::create("plout0_conv"+id+"_output", PLIO64_ARG(OUT_TXT));

      gmio_w = adf::input_gmio::create("gmio0_gemm"+id+"_w", 64, 1000);
      adf::connect<adf::stream> (plin[0].out[0], g.pin[0]);
      adf::connect<adf::stream> (gmio_w.out[0], g.pin[1]);
      adf::connect<adf::stream> (g.pout[0], plout[0].in[0]);
    }
};


template <
  template<typename, int, int, int, int> class SPLIT,
  template<int, int, int, int, int, int, int, int, int, int, int, int, int> class CONV, 
  template<typename, int, int, int, int> class CONCAT, 
  int HCHUNK,
  int INP_H, int INP_W, int OUT_W, int OUT_W_PAD, int STEP_H, int STEP_W,
  int B, int C, int M, int KH, int KW, int GROUP, int IS_RELU,
  int H0 = 0, int H1 = 0, int W0 = 0, int W1 = 0>
class ConvReluChunkHPktStreamGraphTest : public adf::graph {

  private:
    typedef ConvReluChunkHPktStreamGraph<SPLIT, CONV, CONCAT, HCHUNK, 
                                      INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W,
                                      B, C, M, KH, KW, GROUP, IS_RELU,
                                      H0, H1, W0, W1> Graph;
    Graph g;

  public:
    adf::input_plio plin[1];
    adf::output_plio plout[1];
    adf::input_gmio gmio_w;

    ConvReluChunkHPktStreamGraphTest(
      const std::string& id,
      std::vector<float> bias,
      const std::string& INP_TXT,
      const std::string& OUT_TXT = "conv_out.txt"
    ): g(bias) { 
      plin[0] = adf::input_plio::create("plin0_conv"+id+"_input", PLIO64_ARG(INP_TXT));
      plout[0] = adf::output_plio::create("plout0_conv"+id+"_output", PLIO64_ARG(OUT_TXT));

      gmio_w = adf::input_gmio::create("gmio0_gemm"+id+"_w", 64, 1000);
      adf::connect<adf::stream> (plin[0].out[0], g.pin[0]);
      adf::connect<adf::stream> (gmio_w.out[0], g.pin[1]);
      adf::connect<adf::stream> (g.pout[0], plout[0].in[0]);
    }
};


// instance to be compiled and used in host within xclbin
const int INP_H = 24;
const int INP_W = 24;
const int OUT_W = INP_W;
const int OUT_W_PAD = (INP_W + 3)/4*4;
const int STEP_H = 1;
const int STEP_W = 1;
const int B = 1;
const int C = 2; // loop dependency missed issue occurs at C=1
const int M = 4;
const int KH = 5;
const int KW = 5;
const int GROUP = 1;
const int IS_RELU = 1;
const int MCHUNK = 2; // aiecompiler have issues when this is 1 (deadlock?) or 3 (invalid seg)
const int HCHUNK = 8; // OUT_H' * strides + overlap, overlap = KH - strides
const int PADH = KH/2;
const int PADW = KW/2;

std::vector<float> fpweights {-0.26942873001098633, -0.23129096627235413, 0.30025559663772583, 0.4555683732032776, -0.18344980478286743, 0.3268052935600281, -0.3960091471672058, 0.1339816451072693, 0.251032292842865, -0.34402206540107727, -0.0739976167678833, 0.3927071690559387, -0.39642155170440674, -0.48190364241600037, 0.09058535099029541, -0.06446847319602966, 0.29868924617767334, 0.42345553636550903, -0.2008463442325592, -0.11159586906433105, -0.013727903366088867, 0.08815145492553711, 0.483853816986084, 0.19733023643493652, -0.11045148968696594, -0.23623231053352356, 0.44462573528289795, -0.3644515872001648, 0.22026586532592773, 0.42539501190185547, 0.1646655797958374, -0.07694557309150696, -0.30100905895233154, -0.13252466917037964, 0.20687180757522583, 0.1495342254638672, 0.42797619104385376, 0.36686092615127563, 0.3161507248878479, 0.4114508628845215, -0.2236628532409668, -0.1304764449596405, -0.12010610103607178, 0.060450613498687744, 0.16821825504302979, -0.21328333020210266, -0.4805375337600708, -0.10077762603759766, -0.19147205352783203, 0.4421847462654114, 0.3882650136947632, 0.3603106737136841, 0.15299975872039795, -0.1557108461856842, 0.048849284648895264, 0.3152250647544861, -0.40138962864875793, 0.3010748624801636, -0.4588202238082886, 0.3164210319519043, 0.30756378173828125, -0.44899269938468933, 0.12716072797775269, 0.0024530887603759766, -0.33018049597740173, -0.3516210615634918, 0.27325910329818726, 0.06769275665283203, 0.48299914598464966, 0.4822477698326111, 0.492667019367218, -0.3813844919204712, 0.4382561445236206, -0.2554304003715515, -0.04178774356842041, 0.2574065327644348, -0.29637908935546875, 0.06631159782409668, -0.31418323516845703, -0.3952639102935791, -0.38344138860702515, -0.14236095547676086, -0.49534517526626587, -0.07514607906341553, 0.16419708728790283, -0.09831181168556213, -0.414205402135849, -0.4373111426830292, -0.22188347578048706, -0.3306873142719269, 0.46509498357772827, -0.34876978397369385, 0.30546241998672485, 0.08610796928405762, 0.06928694248199463, 0.012080729007720947, 0.47176307439804077, -0.13615521788597107, 0.28791576623916626, 0.05529409646987915, -0.10436633229255676, 0.4554659128189087, 0.09831595420837402, -0.38108307123184204, -0.08246079087257385, 0.28158169984817505, 0.19374704360961914, 0.41634035110473633, -0.2406226098537445, 0.2581937313079834, -0.0401248037815094, 0.07360976934432983, 0.4550466537475586, 0.4792863130569458, 0.3615909814834595, -0.14090290665626526, 0.3877008557319641, 0.1386091709136963, -0.0700032114982605, -0.4642573297023773, 0.27012813091278076, 0.0021055936813354492, 0.2861884832382202, 0.24802279472351074, 0.29356735944747925, -0.1993488371372223, 0.30079859495162964, 0.048846304416656494, -0.02667379379272461, 0.17512589693069458, -0.4786413311958313, -0.39768317341804504, -0.20782262086868286, 0.4829900860786438, -0.3602542281150818, -0.16940370202064514, -0.44894692301750183, -0.16873112320899963, -0.1796737015247345, 0.4468071460723877, 0.3451541066169739, -0.11723577976226807, -0.4752309322357178, 0.3310311436653137, 0.16053617000579834, -0.34763550758361816, 0.4960712790489197, -0.39976656436920166, 0.3671145439147949, -0.20573383569717407, -0.06464654207229614, 0.29545652866363525, 0.17750835418701172, 0.43786436319351196, 0.12114030122756958, -0.40218985080718994, 0.3843603730201721, 0.26915550231933594, 0.21187043190002441, -0.44626644253730774, -0.10377725958824158, -0.33256417512893677, 0.32190388441085815, 0.20052862167358398, 0.38307762145996094, 0.46657508611679077, 0.2747476100921631, 0.49423307180404663, 0.11476987600326538, -0.46287038922309875, -0.4857484698295593, -0.15789613127708435, 0.32347172498703003, 0.3661347031593323, 0.460812509059906, -0.43487852811813354, -0.4554288983345032, 0.4132835865020752, -0.1949532926082611, 0.05798739194869995, 0.4824448823928833, -0.09955146908760071, 0.16587138175964355, -0.09912043809890747, 0.26819467544555664, 0.02771472930908203, -0.2624768614768982, -0.2286939024925232, -0.24194079637527466, 0.03232032060623169, 0.20318901538848877, 0.44927990436553955, 0.19408738613128662, 0.2811928391456604, -0.3310738801956177, -0.12593737244606018, -0.08621978759765625, 0.18638020753860474, -0.20410802960395813, -0.196708083152771};
std::vector<float> fpweights_pad {-0.26942873001098633, -0.23129096627235413, 0.30025559663772583, 0.4555683732032776, -0.18344980478286743, 0.0, 0.0, 0.0, 0.3268052935600281, -0.3960091471672058, 0.1339816451072693, 0.251032292842865, -0.34402206540107727, 0.0, 0.0, 0.0, -0.0739976167678833, 0.3927071690559387, -0.39642155170440674, -0.48190364241600037, 0.09058535099029541, 0.0, 0.0, 0.0, -0.06446847319602966, 0.29868924617767334, 0.42345553636550903, -0.2008463442325592, -0.11159586906433105, 0.0, 0.0, 0.0, -0.013727903366088867, 0.08815145492553711, 0.483853816986084, 0.19733023643493652, -0.11045148968696594, 0.0, 0.0, 0.0, -0.23623231053352356, 0.44462573528289795, -0.3644515872001648, 0.22026586532592773, 0.42539501190185547, 0.0, 0.0, 0.0, 0.1646655797958374, -0.07694557309150696, -0.30100905895233154, -0.13252466917037964, 0.20687180757522583, 0.0, 0.0, 0.0, 0.1495342254638672, 0.42797619104385376, 0.36686092615127563, 0.3161507248878479, 0.4114508628845215, 0.0, 0.0, 0.0, -0.2236628532409668, -0.1304764449596405, -0.12010610103607178, 0.060450613498687744, 0.16821825504302979, 0.0, 0.0, 0.0, -0.21328333020210266, -0.4805375337600708, -0.10077762603759766, -0.19147205352783203, 0.4421847462654114, 0.0, 0.0, 0.0, 0.3882650136947632, 0.3603106737136841, 0.15299975872039795, -0.1557108461856842, 0.048849284648895264, 0.0, 0.0, 0.0, 0.3152250647544861, -0.40138962864875793, 0.3010748624801636, -0.4588202238082886, 0.3164210319519043, 0.0, 0.0, 0.0, 0.30756378173828125, -0.44899269938468933, 0.12716072797775269, 0.0024530887603759766, -0.33018049597740173, 0.0, 0.0, 0.0, -0.3516210615634918, 0.27325910329818726, 0.06769275665283203, 0.48299914598464966, 0.4822477698326111, 0.0, 0.0, 0.0, 0.492667019367218, -0.3813844919204712, 0.4382561445236206, -0.2554304003715515, -0.04178774356842041, 0.0, 0.0, 0.0, 0.2574065327644348, -0.29637908935546875, 0.06631159782409668, -0.31418323516845703, -0.3952639102935791, 0.0, 0.0, 0.0, -0.38344138860702515, -0.14236095547676086, -0.49534517526626587, -0.07514607906341553, 0.16419708728790283, 0.0, 0.0, 0.0, -0.09831181168556213, -0.414205402135849, -0.4373111426830292, -0.22188347578048706, -0.3306873142719269, 0.0, 0.0, 0.0, 0.46509498357772827, -0.34876978397369385, 0.30546241998672485, 0.08610796928405762, 0.06928694248199463, 0.0, 0.0, 0.0, 0.012080729007720947, 0.47176307439804077, -0.13615521788597107, 0.28791576623916626, 0.05529409646987915, 0.0, 0.0, 0.0, -0.10436633229255676, 0.4554659128189087, 0.09831595420837402, -0.38108307123184204, -0.08246079087257385, 0.0, 0.0, 0.0, 0.28158169984817505, 0.19374704360961914, 0.41634035110473633, -0.2406226098537445, 0.2581937313079834, 0.0, 0.0, 0.0, -0.0401248037815094, 0.07360976934432983, 0.4550466537475586, 0.4792863130569458, 0.3615909814834595, 0.0, 0.0, 0.0, -0.14090290665626526, 0.3877008557319641, 0.1386091709136963, -0.0700032114982605, -0.4642573297023773, 0.0, 0.0, 0.0, 0.27012813091278076, 0.0021055936813354492, 0.2861884832382202, 0.24802279472351074, 0.29356735944747925, 0.0, 0.0, 0.0, -0.1993488371372223, 0.30079859495162964, 0.048846304416656494, -0.02667379379272461, 0.17512589693069458, 0.0, 0.0, 0.0, -0.4786413311958313, -0.39768317341804504, -0.20782262086868286, 0.4829900860786438, -0.3602542281150818, 0.0, 0.0, 0.0, -0.16940370202064514, -0.44894692301750183, -0.16873112320899963, -0.1796737015247345, 0.4468071460723877, 0.0, 0.0, 0.0, 0.3451541066169739, -0.11723577976226807, -0.4752309322357178, 0.3310311436653137, 0.16053617000579834, 0.0, 0.0, 0.0, -0.34763550758361816, 0.4960712790489197, -0.39976656436920166, 0.3671145439147949, -0.20573383569717407, 0.0, 0.0, 0.0, -0.06464654207229614, 0.29545652866363525, 0.17750835418701172, 0.43786436319351196, 0.12114030122756958, 0.0, 0.0, 0.0, -0.40218985080718994, 0.3843603730201721, 0.26915550231933594, 0.21187043190002441, -0.44626644253730774, 0.0, 0.0, 0.0, -0.10377725958824158, -0.33256417512893677, 0.32190388441085815, 0.20052862167358398, 0.38307762145996094, 0.0, 0.0, 0.0, 0.46657508611679077, 0.2747476100921631, 0.49423307180404663, 0.11476987600326538, -0.46287038922309875, 0.0, 0.0, 0.0, -0.4857484698295593, -0.15789613127708435, 0.32347172498703003, 0.3661347031593323, 0.460812509059906, 0.0, 0.0, 0.0, -0.43487852811813354, -0.4554288983345032, 0.4132835865020752, -0.1949532926082611, 0.05798739194869995, 0.0, 0.0, 0.0, 0.4824448823928833, -0.09955146908760071, 0.16587138175964355, -0.09912043809890747, 0.26819467544555664, 0.0, 0.0, 0.0, 0.02771472930908203, -0.2624768614768982, -0.2286939024925232, -0.24194079637527466, 0.03232032060623169, 0.0, 0.0, 0.0, 0.20318901538848877, 0.44927990436553955, 0.19408738613128662, 0.2811928391456604, -0.3310738801956177, 0.0, 0.0, 0.0, -0.12593737244606018, -0.08621978759765625, 0.18638020753860474, -0.20410802960395813, -0.196708083152771, 0.0, 0.0, 0.0};
std::vector<float> fpbias {-0.14411085844039917, 0.31030207872390747, 0.07759010791778564, -0.42472273111343384};

// BCHW
ConvReluChunkMGraphTest<ConvReluScalar, ConcatFloat, 1, MCHUNK, 
                        INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W, 
                        B, C, M, KH, KW, GROUP, IS_RELU,
                        PADH, PADH, PADW, PADW> convReluScalarBCHW(
  "convReluScalarBCHW", fpweights, fpbias, 
  "conv_fpin.txt", "convbchw_fpout_shape1x4x24x24_ConvReluScalar.txt");

ConvReluChunkMGraphTest<Conv5x5ReluBCHW, ConcatFloat, 1, MCHUNK, 
                        INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W, 
                        B, C, M, KH, KW, GROUP, IS_RELU,
                        PADH, PADH, PADW, PADW> conv5x5ReluBCHW(
  "conv5x5ReluBCHW", fpweights, fpbias, 
  "conv_fpin.txt", "convbchw_fpout_shape1x4x24x24_Conv5x5ReluBCHW.txt");

ConvReluChunkMGraphTest<Conv5x5on8Relu, ConcatFloat, 1, MCHUNK, 
                        INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W, 
                        B, C, M, KH, KW, GROUP, IS_RELU,
                        PADH, PADH, PADW, PADW> conv5x5on8ReluBCHW(
  "conv5x5on8ReluBCHW", fpweights_pad, fpbias, 
  "conv_fpin.txt", "convbchw_fpout_shape1x4x24x24_Conv5x5on8Relu.txt");

ConvReluChunkHGraphTest<SplitScalar, ConvReluScalarStream, ConcatFloatStream, HCHUNK, 
                        INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W, 
                        B, C, M, KH, KW, GROUP, IS_RELU,
                        PADH, PADH, PADW, PADW> convReluScalarStreamCacheCKK(
  "convReluScalarStreamCacheCKK", fpbias, 
  "conv_fpin.txt", "convbchw_fpout_shape1x4x24x24_ConvReluScalarStream.txt");

ConvReluChunkHStreamGraphTest<SplitFilterFloatStream,ConvReluScalarStream,ConcatFloatStream, HCHUNK, 
                              INP_H, INP_W, OUT_W, OUT_W_PAD, STEP_H, STEP_W, 
                              B, C, M, KH, KW, GROUP, IS_RELU,
                              PADH, PADH, PADW, PADW> convReluScalarStreamCacheCKK_splitstream(
  "convReluScalarStreamCacheCKK_splitstream", fpbias, 
  "conv_fpin.txt", "convbchw_fpout_shape1x4x24x24_ConvReluScalarStreamSplitstream.txt");


#if defined(__X86SIM__) || defined(__AIESIM__)
int main(int argc, char ** argv) {
  // init gmio
  int mckk_bufsize = M*C*KH*KW * sizeof(float_t);
  float_t* mckk_buf = (float_t *) adf::GMIO::malloc(mckk_bufsize);
  memcpy(mckk_buf, fpweights.data(), mckk_bufsize);

  // BCHW
  adfCheck(convReluScalarBCHW.init(), "init convReluScalarBCHW");
  adfCheck(convReluScalarBCHW.run(ITER_CNT), "run convReluScalarBCHW");
	adfCheck(convReluScalarBCHW.end(), "end convReluScalarBCHW");

  adfCheck(conv5x5ReluBCHW.init(), "init conv5x5ReluBCHW");
  adfCheck(conv5x5ReluBCHW.run(ITER_CNT), "run conv5x5ReluBCHW");
	adfCheck(conv5x5ReluBCHW.end(), "end conv5x5ReluBCHW");

  adfCheck(conv5x5on8ReluBCHW.init(), "init conv5x5on8ReluBCHW");
  adfCheck(conv5x5on8ReluBCHW.run(ITER_CNT), "run conv5x5on8ReluBCHW");
	adfCheck(conv5x5on8ReluBCHW.end(), "end conv5x5on8ReluBCHW");

  adfCheck(convReluScalarStreamCacheCKK.init(), "init convReluScalarStreamCacheCKK");
  convReluScalarStreamCacheCKK.gmio_w.gm2aie_nb(mckk_buf, mckk_bufsize);
  adfCheck(convReluScalarStreamCacheCKK.run(ITER_CNT), "run convReluScalarStreamCacheCKK");
	adfCheck(convReluScalarStreamCacheCKK.end(), "end convReluScalarStreamCacheCKK");
  
  adfCheck(convReluScalarStreamCacheCKK_splitstream.init(), "init convReluScalarStreamCacheCKK_splitstream");
  convReluScalarStreamCacheCKK_splitstream.gmio_w.gm2aie_nb(mckk_buf, mckk_bufsize);
  adfCheck(convReluScalarStreamCacheCKK_splitstream.run(ITER_CNT), "run convReluScalarStreamCacheCKK_splitstream");
	adfCheck(convReluScalarStreamCacheCKK_splitstream.end(), "end convReluScalarStreamCacheCKK_splitstream");

  // cleanup gmio
  adf::GMIO::free(mckk_buf);
  return 0;
}
#endif
