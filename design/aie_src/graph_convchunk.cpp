#include "graph_conv.h"
#include "graph_utils.h"

#define ITER_CNT 1


template <template<int, int, int, int, int, int> class CONV, int IS_BCHW,
  int MCHUNK, int INP_W, int OUT_W, int B, int C, int M, int K>
class ConvReluChunkGraphTest : public adf::graph {

  private:
    ConvReluChunkGraph<CONV, IS_BCHW, MCHUNK, INP_W, OUT_W, B, C, M, K> g;

  public:
    adf::input_plio plin[CONCAT_NLANES];
    adf::output_plio plout[1];

    ConvReluChunkGraphTest(
      const std::string& id,
      std::vector<float> weights,
      std::vector<float> bias,
      const std::string& INP_TXT,
      const std::string& EMPTY_TXT,
      const std::string& OUT_TXT = "conv_out.txt"
    ) { 
      g.construct(weights, bias);
      for (int i = 0; i < CONCAT_NLANES; i++) {
        std::string plio_name = "plin"+std::to_string(i)+"_conv"+id+"_input";
        if (i < CHUNK_COUNT) {
          plin[i] = adf::input_plio::create(plio_name, adf::plio_64_bits, TXT_ARG(INP_TXT));
          adf::connect<adf::window<B*INP_W*INP_W*C*4>> (plin[i].out[0], g.pin[i]);
        } else {
          plin[i] = adf::input_plio::create(plio_name, adf::plio_64_bits, TXT_ARG(EMPTY_TXT));
          adf::connect<adf::window<4>> (plin[i].out[0], g.pin[i]);
        }
      }
      plout[0] = adf::output_plio::create("plout0_conv"+id+"_output", adf::plio_64_bits, TXT_ARG(OUT_TXT));
      adf::connect<adf::window<B*OUT_W*OUT_W*M*4>> (g.pout[0], plout[0].in[0]);
    }
};


// instance to be compiled and used in host within xclbin
char empty_input[]  = "empty.txt";
const int mchunk = 4;
std::vector<float> weights {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 425, 426, 427, 428, 429, 430, 431, 432, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455, 456, 457, 458, 459, 460, 461, 462, 463, 464, 465, 466, 467, 468, 469, 470, 471, 472, 473, 474, 475, 476, 477, 478, 479, 480, 481, 482, 483, 484, 485, 486, 487, 488, 489, 490, 491, 492, 493, 494, 495, 496, 497, 498, 499, 500, 501, 502, 503, 504, 505, 506, 507, 508, 509, 510, 511, 512, 513, 514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 525, 526, 527, 528, 529, 530, 531, 532, 533, 534, 535, 536, 537, 538, 539, 540, 541, 542, 543, 544, 545, 546, 547, 548, 549, 550, 551, 552, 553, 554, 555, 556, 557, 558, 559, 560, 561, 562, 563, 564, 565, 566, 567, 568, 569, 570, 571, 572, 573, 574, 575, 576, 577, 578, 579, 580, 581, 582, 583, 584, 585, 586, 587, 588, 589, 590, 591, 592, 593, 594, 595, 596, 597, 598, 599, 600, 601, 602, 603, 604, 605, 606, 607, 608, 609, 610, 611, 612, 613, 614, 615, 616, 617, 618, 619, 620, 621, 622, 623, 624, 625, 626, 627, 628, 629, 630, 631, 632, 633, 634, 635, 636, 637, 638, 639, 640, 641, 642, 643, 644, 645, 646, 647, 648, 649, 650, 651, 652, 653, 654, 655, 656, 657, 658, 659, 660, 661, 662, 663, 664, 665, 666, 667, 668, 669, 670, 671, 672, 673, 674, 675, 676, 677, 678, 679, 680, 681, 682, 683, 684, 685, 686, 687, 688, 689, 690, 691, 692, 693, 694, 695, 696, 697, 698, 699, 700, 701, 702, 703, 704, 705, 706, 707, 708, 709, 710, 711, 712, 713, 714, 715, 716, 717, 718, 719, 720, 721, 722, 723, 724, 725, 726, 727, 728, 729, 730, 731, 732, 733, 734, 735, 736, 737, 738, 739, 740, 741, 742, 743, 744, 745, 746, 747, 748, 749, 750, 751, 752, 753, 754, 755, 756, 757, 758, 759, 760, 761, 762, 763, 764, 765, 766, 767, 768, 769, 770, 771, 772, 773, 774, 775, 776, 777, 778, 779, 780, 781, 782, 783, 784, 785, 786, 787, 788, 789, 790, 791, 792, 793, 794, 795, 796, 797, 798, 799, 800, 801, 802, 803, 804, 805, 806, 807, 808, 809, 810, 811, 812, 813, 814, 815, 816, 817, 818, 819, 820, 821, 822, 823, 824, 825, 826, 827, 828, 829, 830, 831, 832, 833, 834, 835, 836, 837, 838, 839, 840, 841, 842, 843, 844, 845, 846, 847, 848, 849, 850, 851, 852, 853, 854, 855, 856, 857, 858, 859, 860, 861, 862, 863, 864, 865, 866, 867, 868, 869, 870, 871, 872, 873, 874, 875, 876, 877, 878, 879, 880, 881, 882, 883, 884, 885, 886, 887, 888, 889, 890, 891, 892, 893, 894, 895, 896, 897, 898, 899, 900, 901, 902, 903, 904, 905, 906, 907, 908, 909, 910, 911, 912, 913, 914, 915, 916, 917, 918, 919, 920, 921, 922, 923, 924, 925, 926, 927, 928, 929, 930, 931, 932, 933, 934, 935, 936, 937, 938, 939, 940, 941, 942, 943, 944, 945, 946, 947, 948, 949, 950, 951, 952, 953, 954, 955, 956, 957, 958, 959, 960, 961, 962, 963, 964, 965, 966, 967, 968, 969, 970, 971, 972, 973, 974, 975, 976, 977, 978, 979, 980, 981, 982, 983, 984, 985, 986, 987, 988, 989, 990, 991, 992, 993, 994, 995, 996, 997, 998, 999, 1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023, 1024, 1025, 1026, 1027, 1028, 1029, 1030, 1031, 1032, 1033, 1034, 1035, 1036, 1037, 1038, 1039, 1040, 1041, 1042, 1043, 1044, 1045, 1046, 1047, 1048, 1049, 1050, 1051, 1052, 1053, 1054, 1055, 1056, 1057, 1058, 1059, 1060, 1061, 1062, 1063, 1064, 1065, 1066, 1067, 1068, 1069, 1070, 1071, 1072, 1073, 1074, 1075, 1076, 1077, 1078, 1079, 1080, 1081, 1082, 1083, 1084, 1085, 1086, 1087, 1088, 1089, 1090, 1091, 1092, 1093, 1094, 1095, 1096, 1097, 1098, 1099, 1100, 1101, 1102, 1103, 1104, 1105, 1106, 1107, 1108, 1109, 1110, 1111, 1112, 1113, 1114, 1115, 1116, 1117, 1118, 1119, 1120, 1121, 1122, 1123, 1124, 1125, 1126, 1127, 1128, 1129, 1130, 1131, 1132, 1133, 1134, 1135, 1136, 1137, 1138, 1139, 1140, 1141, 1142, 1143, 1144, 1145, 1146, 1147, 1148, 1149, 1150, 1151, 1152, 1153, 1154, 1155, 1156, 1157, 1158, 1159, 1160, 1161, 1162, 1163, 1164, 1165, 1166, 1167, 1168, 1169, 1170, 1171, 1172, 1173, 1174, 1175, 1176, 1177, 1178, 1179, 1180, 1181, 1182, 1183, 1184, 1185, 1186, 1187, 1188, 1189, 1190, 1191, 1192, 1193, 1194, 1195, 1196, 1197, 1198, 1199};
std::vector<float> bias {1, 1, 1, 1, 1, 1, 1, 1};
std::vector<float> weights_rand {0.6002128681312939, 0.96319729526038, 0.14780133406539042, 0.2569166436866691, 0.8735568272907714, 0.4918922317083445, 0.8989610922270317, 0.18551789752317627, 0.5326685874713607, 0.32626963264937237, 0.31654255989247604, 0.44687696394619913, 0.43307744910126844, 0.3573468796779544, 0.9149707703156186, 0.7317441854328928, 0.7275469913315297, 0.2899134495919554, 0.5777094243168404, 0.779179433301834, 0.7955903685432131, 0.34453046075431226, 0.7708727565686478, 0.735893896807733, 0.14150648562190027, 0.8659454685664772, 0.4413214701804108, 0.48641044888866547, 0.4483691788979973, 0.5678460014775075, 0.6211692473670547, 0.4981795657629434, 0.8667885432590956, 0.6277347561952844, 0.40142794930551995, 0.41669175690871096, 0.8108386151289514, 0.3481919427465201, 0.21145479578241355, 0.059383188005789234, 0.8760268479205742, 0.9185464511903499, 0.12012018216347597, 0.33447374149611486, 0.17537206951524387, 0.11589846882587973, 0.8998667430000302, 0.05687725914535546, 0.9804856634690068, 0.09645086069738418, 0.8634706491935857, 0.5665061069891627, 0.36791748781787337, 0.3423423766251579, 0.7573641432377087, 0.3145732950042872, 0.6573189166171418, 0.5173260835160801, 0.4849656451580705, 0.9011621706491616, 0.5546450586202596, 0.8268616030486949, 0.7255735341014894, 0.03855724605899835, 0.7731100525054192, 0.21687025009104066, 0.9031496468515715, 0.042924190608832014, 0.33307203447431877, 0.09973294723475401, 0.47558911708484375, 0.8200224358697518, 0.2981873596630641, 0.1509348973110416, 0.3302670356968992, 0.813880141920636, 0.14038395779934687, 0.2273624490775018, 0.06885196449337394, 0.7057100439896077, 0.3952332435363368, 0.310839977143316, 0.7186263903411519, 0.33597754234025523, 0.727771273214418, 0.8151993953143135, 0.21766284345773845, 0.9738186968459833, 0.16235794791266678, 0.29084090665674256, 0.17979529083354162, 0.34550565635633446, 0.4800608878207556, 0.5221758690021191, 0.853606042296272, 0.8894479088158666, 0.22010386078090638, 0.6228940321857525, 0.11149605729871559, 0.4589698601820683, 0.32233353804661813, 0.3165007454536064, 0.482584241712101, 0.7298276355292675, 0.0691826587923895, 0.8791733376874045, 0.7348137746305611, 0.17649938888906014, 0.9391609090873704, 0.5063122240233886, 0.9998085781169653, 0.19725947430073765, 0.5349081983832014, 0.2902480425599284, 0.30417355738924656, 0.5910653808339902, 0.9217190668708333, 0.805263855579175, 0.7239413985013259, 0.5591737821017022, 0.9222985036674477, 0.49236140669088413, 0.8738321783347182, 0.8339816438766318, 0.21383534680158656, 0.7712254629659102, 0.012171156943258765, 0.3228295375480875, 0.22956744469194978, 0.5068629584873134, 0.7368531616807855, 0.09767636744772412, 0.5149222019367684, 0.9384120216943856, 0.2286465509829455, 0.6771411441114241, 0.5928802707811576, 0.01006369565609333, 0.47582619585722274, 0.7087703909410487, 0.0439754320240906, 0.8795214830237301, 0.5200814166356731, 0.03066104832571792, 0.2244136119208402, 0.953675696427313, 0.5823197330520553, 0.10747256776880454, 0.2875445022805615, 0.4567036258604842, 0.020950069267730465, 0.4116155136137588, 0.48945863543469814, 0.2436778752812322, 0.5886390002919866, 0.7532401195921768, 0.23583422410563548, 0.6204999002799877, 0.6396222429637416, 0.948540301128841, 0.7782761672964438, 0.848345269790006, 0.49041990843627514, 0.18534858697938283, 0.995815292959635, 0.1293557610386996, 0.4714573193542654, 0.0680930992421066, 0.9438508573508195, 0.9649249408484396, 0.7193890620039736, 0.3499928436301689, 0.25438240111762433, 0.2653033245370474, 0.12729402542007473, 0.5258089530767229, 0.1418172757014775, 0.3167306665534563, 0.6267064759591049, 0.7275436095907498, 0.024272704622061214, 0.4301159843767399, 0.652124594842139, 0.8532459761585128, 0.4753247822120824, 0.9692058717176852, 0.2656325475414473, 0.01350870662671888, 0.4837528647017362, 0.25611379502001796, 0.8237176720231694, 0.23277267218111208, 0.3106292182913063, 0.7912274310191123, 0.71514325202536, 0.5580512366572625, 0.7049480619227595, 0.4186368635700265, 0.005310047614667801, 0.011355128512192558, 0.5112217875736866, 0.08329097971899924, 0.05107548016925678, 0.9655166391374733, 0.8590026396580586, 0.15202722720956952, 0.0006642185902044906, 0.9416677953897564, 0.2783252983201143, 0.1858976028554975, 0.6915081078315353, 0.1089037388413262, 0.2646495980028003, 0.9750946802120465, 0.6394627744740102, 0.5206777914825536, 0.39791861483493773, 0.774500954884202, 0.14095747652369373, 0.9673378020369822, 0.8611230080657867, 0.6176569825692433, 0.042906190402558164, 0.7008556494477377, 0.9132843408854808, 0.524577067478626, 0.3542248218272628, 0.12027734497946962, 0.7549011041375683, 0.8850218512118062, 0.10025174407584858, 0.7589845547523419, 0.017060486259097507, 0.9670549180772195, 0.6150580206350962, 0.5524390589916331, 0.2959498335889278, 0.9292916715697834, 0.265905627336791, 0.8281466132166949, 0.985108679337132, 0.7833966455148058, 0.5189899203864318, 0.06607426385167192, 0.472413789171073, 0.43825594697170356, 0.20279604118699524, 0.42358763671430444, 0.3577578840803387, 0.16368426115164847, 0.44137414333248615, 0.26279995632448905, 0.5220624206822644, 0.03516005971619418, 0.9062314197872842, 0.8163643055198561, 0.5525813325105046, 0.8518085827572431, 0.9623950738108005, 0.11052229405266056, 0.6308318084098038, 0.9979940009356586, 0.9878891693335261, 0.6033229922907446, 0.12802087045081667, 0.5831928309905282, 0.0020646355744217137, 0.19891133466685962, 0.95612315955996, 0.3304405726028439, 0.6383901057769485, 0.2808594946322427, 0.9478218871115895, 0.7285587299494868, 0.3296511575814126, 0.7917614211803709, 0.10816552447633765, 0.3923189400654594, 0.22121812773159233, 0.6837264472760463, 0.10244628177193038, 0.3970258322826741, 0.2766497302091896, 0.5063429193238042, 0.3498976805038789, 0.706410577667011, 0.024577024306966067, 0.6339869213059075, 0.23057128969382001, 0.2687090287568493, 0.8002556035835235, 0.9555683939804208, 0.3165502100270866, 0.8268052703690757, 0.1039908379712331, 0.6339816530073832, 0.7510322995349188, 0.15597792803445654, 0.42600238766956056, 0.8927071642489816, 0.10357846344837218, 0.01809635820147515, 0.5905853790479337, 0.43553154053093346, 0.7986892488839027, 0.9234555382041066, 0.2991536445438161, 0.3884041171691053, 0.48627208632931485, 0.5881514604352993, 0.9838538296816167, 0.6973302508300515, 0.3895485073455688, 0.2637676864504621, 0.9446257184221241, 0.1355484331085064, 0.720265852504758, 0.9253950252924407, 0.6646655865344386, 0.42305444016612115, 0.198990939925484, 0.36747532225192725, 0.7068718094201221, 0.6495342241752968, 0.9279761665983579, 0.8668609136593749, 0.8161507522663684, 0.9114508753507623, 0.2763371527370494, 0.3695235401396495, 0.3798939037860174, 0.5604505887097803, 0.668218229542771, 0.2867166830403943, 0.019462467309300346, 0.3992223836579093, 0.3085279595544005, 0.9421847190180479, 0.8882650405440435, 0.8603106783434196, 0.652999760945243, 0.3442891646989389, 0.5488492673993047, 0.8152250407163727, 0.09861036872281104, 0.8010748802553047, 0.04117979132812166, 0.8164210312143557, 0.8075638041598735, 0.051007308828755815, 0.6271607114618583, 0.5024530743128972, 0.16981950317171468, 0.14837893767737564, 0.7732591260670232, 0.5676927488368441, 0.9829991349165325, 0.9822477772883441, 0.9926669934642663, 0.11861551836576589, 0.9382561369639623, 0.2445696090205628, 0.458212259758612, 0.7574065558217259, 0.20362093207723198, 0.5663116055543299, 0.1858167482259957, 0.1047361066812933, 0.11655861242959853, 0.3576390348482188, 0.004654836837183485, 0.4248539212016895, 0.664197105081149, 0.40168818500398873, 0.08579460051983512, 0.06268886202364055, 0.2781165127648617, 0.16931269054360076, 0.9650949732179694, 0.151230224974085, 0.8054624374049966, 0.5861079417411448, 0.5692869199842944, 0.5120807159237846, 0.9717630761139511, 0.3638447750916378, 0.7879157509538762, 0.5552941074669596, 0.39563366762572616, 0.9554659333057809, 0.5983159693646871, 0.11891694214601356, 0.4175392009968566, 0.7815817276816076, 0.6937470232392138, 0.9163403298365742, 0.25937738414152023, 0.7581937163538238, 0.4598752073945209, 0.5736097469668154, 0.955046681006283, 0.9792863199891774, 0.8615909639255636, 0.35909708773029625, 0.8877008357273198, 0.638609177641413, 0.4299967804707131, 0.035742682500483736, 0.7701281241915822, 0.502105580724113, 0.7861884994650608, 0.7480227993432699, 0.7935673680483519, 0.30065115869676684, 0.800798599067631, 0.5488463284661673, 0.4733262004491432, 0.6751259138207569, 0.021358682938348306, 0.10231681588759423, 0.292177365167522, 0.9829901099578396, 0.13974577886495498, 0.3305963007592103, 0.05105306469192361, 0.3312688802927263, 0.3203262865108316, 0.9468071709445222, 0.8451540869957204, 0.3827642192427715, 0.02476905803924867, 0.8310311139953782, 0.6605361771445402, 0.15236448365130972, 0.9960712710101872, 0.10023343742412305, 0.8671145415936629, 0.294266164129689, 0.43535346632515703, 0.7954565267763146, 0.6775083559940103, 0.9378643744440391, 0.6211403254603832, 0.09781016147905275, 0.8843603632419503, 0.7691555249885911, 0.711870450893322, 0.05373354735373537, 0.396222744661744, 0.16743581945953923, 0.821903908407869, 0.7005286228178628, 0.8830775973191353, 0.9665751069241639, 0.7747476141924986, 0.9942330832207621, 0.6147698861442988, 0.037129603891447815, 0.01425151515029588, 0.3421038752037887, 0.8234717190675371, 0.866134706324731, 0.9608125288226779, 0.0651214685488184, 0.044571110987664087, 0.9132835963670337, 0.3050466983628616, 0.5579874006096327, 0.9824448830007099, 0.40044853473592334, 0.6658713983098399, 0.4008795636319853, 0.7681946644627797, 0.5277147255728658, 0.23752313798634972, 0.2713061012584662, 0.25805921253840725, 0.5323203282585608, 0.7031890160156643, 0.9492799000431421, 0.6940873750829799, 0.7811928439623906, 0.16892611586456563, 0.3740626250073257, 0.4137802198582661, 0.686380230000473, 0.2958919763768736, 0.3032919214196661, 0.3558891546408681, 0.8103020815432322, 0.5775900900433487, 0.07527727983273447, 0.0782460996904688, 0.3712869442824993, 0.7665910506061583, 0.6886834264786805, 0.7079823546511866, 0.7672100660540969, 0.2871527128863294, 0.548256281942088, 0.5433526403167696, 0.7396325011799493, 0.9568705691056464, 0.2779899447954659, 0.7932816727570482, 0.6599705485232624, 0.5802378708634601, 0.7748797793474019, 0.9440324664280993, 0.03669141786941299, 0.14740010251322067, 0.7562872315011158, 0.08379135291008455, 0.516123700741093, 0.21986077664160697, 0.2742957038427515, 0.7018404829444163, 0.030192773268711615, 0.8733194279019036, 0.44447895535227, 0.5023932939071011, 0.5400479636936919, 0.6455442945794939, 0.3448565866227481, 0.10110749056349133, 0.31837893666617656, 0.1681421180228203, 0.5561331794504739, 0.31802863096038647, 0.9580671779934349, 0.9657342779802807, 0.6201258805682415, 0.617497267667286, 0.9853785645671059, 0.887283151235186, 0.76506994915814, 0.3135906117427012, 0.36553902811807637, 0.20126676576086544, 0.48714812691737464, 0.9903685221395796, 0.9121509530115888, 0.11834943401946907, 0.02519028929050615, 0.8986376684082662, 0.5371701279487074, 0.20018988820845618, 0.6736532695846353, 0.64422317792982, 0.12208560683646719, 0.2596002334976195, 0.06007796431325796, 0.20986047387014195, 0.13230567485306288, 0.1932362924745561, 0.685467145913117, 0.049499744261067735, 0.10185461524821648, 0.13417363807828764, 0.316541120958134, 0.2987503108519409, 0.2550637853166394, 0.7505366534324163, 0.9980227877581006, 0.5339779236287958, 0.9442027179139421, 0.3966101121260647, 0.10668244680271344, 0.4087738294698373, 0.2961277734146933, 0.49340696228917813, 0.6570436769164232, 0.4610502191524082, 0.9351605120778256, 0.8847648222993799, 0.7019775951661972, 0.48968491236454914, 0.13168728181445688, 0.3970136667776334, 0.7044015394011737, 0.2848855205537312, 0.10398807754342043, 0.9078984575106281, 0.7090508098155446, 0.6152764266350972, 0.7924989056089166, 0.8356460375834757, 0.48345899817102334, 0.88118825140944, 0.9164190107011054, 0.2715510954247814, 0.6075453597907943, 0.5265840288204692, 0.5379457791519024, 0.937663093964562, 0.305188702693029, 0.9834339782345762, 0.9021312148042321, 0.45872288861350263, 0.8174532636181662, 0.7690469943201406, 0.6778949696185212, 0.31983388938315427, 0.19645099182095394, 0.6715276967657582, 0.8429732964010842, 0.01625278867736968, 0.6428033753085121, 0.44287302462221245, 0.8980877551269836, 0.32147293085420525, 0.47418481226362175, 0.5147671040351863, 0.14043952137105298, 0.7128923026989064, 0.8304763451209596, 0.05790927689626024, 0.29138882053690274, 0.03804468153505791, 0.9565441046882424, 0.6671688207038236, 0.9642004194547815, 0.5314942783313078, 0.8020685238956139, 0.37441398387974, 0.3538190325726409, 0.3782678171051327, 0.6578621337264282, 0.3594531510216522, 0.9003674516042256, 0.9832748650501553, 0.03042651533700591, 0.19362329040249648, 0.112249992587386, 0.04236404709144326, 0.22774099334728648, 0.44679332036908115, 0.8369903652886838, 0.2218240305099961, 0.49394525566220127, 0.9296187394467544, 0.6672147067866749, 0.7980790197559975, 0.5509939701854862, 0.98046645884063, 0.5886621546814742, 0.04551071396259154, 0.19798279993979484, 0.40477362895714797, 0.6012771725737763, 0.7719308674089599, 0.41308612590388627, 0.7100583050156887, 0.789869503109125, 0.3172601972499828, 0.9792702400358185, 0.6496564952358356, 0.8809980607861312, 0.5559376888086067, 0.7416031071523237, 0.7705440616163653, 0.908248379234579, 0.1503497568713703, 0.5582834241692485, 0.4283785131058563, 0.9231590211737403, 0.1050946942489992, 0.9825738886809128, 0.8754513240932963, 0.07382628156424853, 0.49096638554047445, 0.7175595002125259, 0.7381515456910565, 0.9064941252379208, 0.7998654355823109, 0.3109303803440895, 0.4984347858624524, 0.701785762935854, 0.13843684136435142, 0.19399079760019733, 0.4810424481098726, 0.2982457987055127, 0.8625592497840949, 0.586277321520618, 0.3486651977183677, 0.8488330256044101, 0.8048784485542388, 0.9983548770992695, 0.8473076883337348, 0.4144565348056797, 0.12749888176998847, 0.8406409082004779, 0.05975791796039398, 0.3502712073448542, 0.9197379807712738, 0.9607664734451007, 0.6405645935212184, 0.6886483169837135, 0.04245448872552249, 0.5144803410895479, 0.5468681788063412, 0.3401007461241111, 0.06859683475388434, 0.228907599724394, 0.35798393686119634, 0.4351419865163296, 0.5909267255335692, 0.7223915186982364, 0.31763187327394615, 0.3289537599189831, 0.019691642723703717, 0.040874860094018306, 0.2578216943085364, 0.7402449976749567, 0.6283138303739122, 0.7697890206778347, 0.7689194362148217, 0.8565674676693013, 0.7203192659868836, 0.9790109190008228, 0.8988252193018174, 0.5867171662232342, 0.5881576704911717, 0.034267040352229494, 0.9985265777083543, 0.13157599736614178, 0.740347196631592, 0.8210151951243089, 0.3730545293052032, 0.19685205466531375, 0.09875988679055503, 0.7486060058295778, 0.4526535292056957, 0.7137177590011357, 0.9154076488518006, 0.1465837361510567, 0.9191710007237996, 0.4116264595084367, 0.30526700989728905, 0.9430622606027791, 0.9906516926063994, 0.19889221776744814, 0.6568383469519833, 0.10649531377106036, 0.6509140038575058, 0.8273132277758497, 0.6844985465240676, 0.41733314206259575, 0.38306635956376955, 0.39312241522341707, 0.5897118179929232, 0.8815672700724956, 0.9290661572687678, 0.05352962020731811, 0.1816223946456883, 0.11222431582828851, 0.19333464076691398, 0.3466078106091718, 0.5065316826226238, 0.6294612270091522, 0.7321422191397015, 0.8901115413858071, 0.9890884372297908, 0.6628564785571198, 0.845364518667763, 0.7780388469246989, 0.30753203921871197, 0.875692270234923, 0.042763137947798846, 0.00036734375145786036, 0.2737326293884642, 0.4620975296274499, 0.6383628950006773, 0.1017702665445448, 0.67301013383484, 0.8018158670596945, 0.1853129195793668, 0.41512525483179386, 0.5199849899533098, 0.45180701808448065, 0.7998299308884707, 0.9605223981193308, 0.79895316400818, 0.07799281787582457, 0.8049355721591271, 0.06659633223555117, 0.23597038524157, 0.1530968968357228, 0.19751910684608542, 0.5283151270573506, 0.6716898576108536, 0.470321282336041, 0.9596956390292436, 0.24029232003016665, 0.7631402302547929, 0.8701821785071485, 0.5620661107547191, 0.4562225019615531, 0.5961844467792436, 0.42880977016034294, 0.5551938823233694, 0.4169339521288016, 0.40046971029413403, 0.6953464681395298, 0.09285121306796795, 0.16654207272882082, 0.851198471715004, 0.771077346815188, 0.2814537272943938, 0.3772689326657602, 0.9260265066284805, 0.8180772251738613, 0.6143462999796309, 0.22149017880295774, 0.04425197131526959, 0.43125784684189816, 0.6726271392574619, 0.8284804905178516, 0.8526890057422694, 0.032775901293363385, 0.24415703383687937, 0.33909458847740304, 0.18873221095938586, 0.8029753783638378, 0.7674657630849425, 0.5168330403809708, 0.9829264779958555, 0.14405854109084226, 0.8996517033276689, 0.11646325416191616, 0.1631817055141207, 0.6962192002506188, 0.10956969204168931, 0.5658450954187109, 0.4202335364726536, 0.7284739655410148, 0.9006752388088651, 0.7698715146690553, 0.8496898764211883, 0.03294544853123238, 0.3101954982442031, 0.5154330866863324, 0.41595331462977947, 0.23125495297968846, 0.3078740979600233, 0.9454309703916057, 0.29418087987135155, 0.35390411017978574, 0.0037097737500186856, 0.8450776272778745, 0.15484070340157663, 0.20414427551372605, 0.2552645170920256, 0.8846220568448426, 0.20645141124631472, 0.7975263608679399, 0.8080493403229045, 0.9270205687863958, 0.11556131360031396, 0.21727897233783133, 0.7428982920807213, 0.19600075428073904, 0.286329546873449, 0.16674158011136264, 0.1726966861550493, 0.4815533546282057, 0.10968306211295853, 0.3216976184636222, 0.426593909598879, 0.024548116523977592, 0.3883331664744526, 0.09412243608551696, 0.49357853114617667, 0.8257381885570964, 0.8184221621817872, 0.08044851553740051, 0.6012277576745202, 0.8345863819712365, 0.23797254293463532, 0.7619265114495991, 0.8907643464500933, 0.8061241514404441, 0.10730103151550452, 0.009059999522384898, 0.19172410992184563, 0.27047734094662323, 0.6161829906159595, 0.3842731752423184, 0.7034070306614735, 0.3530749605216086, 0.1544254246546003, 0.3126898434860351, 0.884324226389135, 0.9585323442450648, 0.20751273406736848, 0.7884683870244413, 0.2733487365398707, 0.8871315434314075, 0.16554561279625546, 0.6659599186940569, 0.08421126471318252, 0.9738933239738695, 0.7006333446405428, 0.8418157394050853, 0.5666693393630345, 0.4768013639288602, 0.6218823913943651, 0.528741511699448, 0.469384355145153, 0.7594502514576428, 0.17820095493926813, 0.17117204815515852, 0.4318426506497062, 0.320747916044946, 0.0741245179130241, 0.8444704924187344, 0.7716028066943498, 0.5439214998029848, 0.979324537264369, 0.07260006654760531, 0.766669301027875, 0.2663703916090968, 0.3685989199435754, 0.21927938520306756, 0.7890378914618652, 0.144240102021566, 0.840016697735278, 0.6615776482298339, 0.05902324526911473, 0.8109817316001534, 0.6277557161629794, 0.9049823195783415, 0.7487223064086382, 0.5611209274297748, 0.8365471806191977, 0.2780502383012221, 0.5469500618445636, 0.29361681980341015, 0.9682043910905087, 0.22619630084200637, 0.015738239670462506, 0.3258548420037962, 0.5025093969698047, 0.028362934230075076, 0.5592483000339074, 0.8742827749512302, 0.7047321944166701, 0.6229683229783043, 0.9559617507082513, 0.9582793338421058, 0.8242664697032286, 0.607741847385851, 0.4877645594977067, 0.01331611324646731, 0.6062619062265414, 0.9890880753568537, 0.818101050024029, 0.34060464388435197, 0.15204702504442202, 0.7840586144671541, 0.7439378206317415, 0.9670467919793231, 0.8748423619393388, 0.5556626252724074, 0.10128424696474214, 0.48350065838396006, 0.313695054022317, 0.5124084820248398, 0.3017015746396532, 0.8618229919755958, 0.8443270139828222, 0.3154651571670559, 0.599581346373995, 0.4301808576390541, 0.9090927570285632, 0.18736090142632278, 0.6977284017849593, 0.9703753278372166, 0.17527551956309018, 0.2019664284242032, 0.6937233386722279, 0.7791539245249139, 0.49054905479393274, 0.6096864651471213, 0.21268240881182876, 0.47661422856732605, 0.11207188087491515, 0.3214219252756647, 0.2847797050309543, 0.44462536010242004, 0.9301263622436969, 0.1812676621478424, 0.4013882466247205, 0.6155972191096447, 0.9465570860740017, 0.13314819153576818, 0.9178766169931135, 0.08105378863406154, 0.4807413971602644, 0.4545898672304828, 0.20960272887719555, 0.3474596681158475, 0.4541652683483981, 0.8652114629106292, 0.9550641453735598, 0.5189256786371652, 0.8700997902944219, 0.6081715856767794, 0.34908734198767544, 0.19419420761346906, 0.4131347717827648, 0.5228242829238477, 0.04444338825955618, 0.14584116587085072, 0.60018442398144, 0.22500159467716208, 0.8373263760812975, 0.3269422497741906, 0.10483420725325088, 0.08353058855531703, 0.9371230205317796, 0.11802031124187917, 0.14090976416206002, 0.862666057932753, 0.254288130277344, 0.6659514107544161, 0.8167256870298913, 0.6071806401301921, 0.9574885434478494, 0.7088828992608414, 0.11275150990365834, 0.5584100265458799, 0.7181865260891839, 0.8019572403734452, 0.02632133528409153, 0.7188789152714244, 0.8256808210194495, 0.7468338109353565, 0.5123491456555981, 0.4580209630888029, 0.5494185832998159, 0.704643698826602, 0.9229142906324854, 0.617035210641125, 0.8878343062834696, 0.7012568474457274, 0.06833637743884757, 0.5008281900016218, 0.28648634831524233, 0.2851749281325723, 0.3559275512159842, 0.3147327720004758, 0.5786099801171002, 0.683601500353557, 0.26874938707673546, 0.12976262256838134, 0.0588087209344621, 0.5757528450305522, 0.1861301663330024, 0.009247994223699374, 0.9277530793237041, 0.5371404190832406, 0.09244818024697277, 0.8429211121274853, 0.9832027382901181, 0.4486006620177172, 0.04248960823815817, 0.1175459407560635, 0.38165374664543217, 0.885522637448877, 0.14803867927776204, 0.8239900940026955, 0.014976260219019433, 0.457388698785811, 0.6443971409505681, 0.0603794820255964, 0.6147627506164867, 0.9444041214695561, 0.16025992013659507, 0.7296113830353528, 0.6090938979435434, 0.18511638887923054, 0.006203414709366029, 0.009284450505927078, 0.5320924091720286, 0.9427794095611935, 0.6442986274656124, 0.7142998462937437, 0.4938654869205371, 0.5818889426345698, 0.12636752533060347, 0.8768206204006308, 0.7607926286668019, 0.9981989528446096, 0.29772294755562134, 0.22701777179605787, 0.12516165796134293, 0.9642097566160448, 0.7808851846280236, 0.1663246137196387, 0.5526864711082139, 0.41376820806727677, 0.1514860078061928, 0.16207298502996548, 0.9634699947451857, 0.3049641829577222, 0.9414392916839432, 0.0756106730650844, 0.4608030422931906, 0.1296190485167601, 0.004787385241172171, 0.5537660737060766, 0.11389410481136342, 0.7220245309336797, 0.6981163757068996, 0.17633290746448915, 0.9417421432655166, 0.7210434084324896, 0.29797026415692995, 0.7092337639507906, 0.731930277681536, 0.3422263312509606, 0.3755885624614592, 0.3591065076533001, 0.6166184435520741, 0.9004101465401794, 0.17319323622858762, 0.8751996098363184, 0.027653156505342502, 0.6603385959782224, 0.4144388731896942, 0.7912815522324937, 0.721198112930903, 0.480107807145184, 0.6438640367225967, 0.5017731306999452, 0.8115184706218832, 0.476083985968232, 0.5231559899920758, 0.25052058641717856, 0.6050430168104723, 0.30290480866983505, 0.5772840145429821, 0.16967811557406987, 0.1594690923411588, 0.41702974119199354, 0.42681951511054084, 0.26810926491253906, 0.13159685036669, 0.039210539213897055, 0.025231827287816144, 0.2715502900064757, 0.46185344216193713, 0.726243281306127, 0.47487170071400897, 0.9040508194352818, 0.03521980457540708, 0.18066062154063434, 0.33851449277014267, 0.5774961880651225, 0.8527361578939424, 0.35020195195657644, 0.2679886825044201, 0.06188916884289575, 0.8213034776981041, 0.37966644320667897, 0.5715501954169343, 0.9835554181712204, 0.0015945706042945762, 0.14545014069886597, 0.7791109940193555, 0.8051274852078378, 0.7692471190327554, 0.5369988910428293, 0.9788569810230561, 0.39618455999532054, 0.6019436981131123, 0.0633690045488483, 0.40985744990733874, 0.722500087509922, 0.2387388407706993, 0.9438275890731485, 0.6867833676432891, 0.28757538285757145, 0.7689989227058563, 0.0831647718081091, 0.9747744225104565, 0.04928525907355852, 0.9334558911487183, 0.2528538777482612, 0.7578241076908905, 7.369943310608917e-05, 0.2542400891795997, 0.7491006066497199, 0.5323360708973223, 0.11495214963711486, 0.3936297458372753, 0.37554935508454257, 0.5681622441809498, 0.6679770724602178};
std::vector<float> bias_rand {0.84083024241225, 0.4972313969773726, 0.3920217173051701, 0.1439765340953817, 0.8048229649897223, 0.71337040541456, 0.40867739745468534, 0.5184323099426694};

ConvReluChunkGraphTest<ConvReluScalarBCHW, 1, mchunk, 12, 8, 1, 6, 8, 5> fpscalar_bchw(
  "fpscalar_bchw", weights, bias, "conv_fpin.txt", "conv_fpout_ConvReluScalarBCHW.txt");
ConvReluChunkGraphTest<ConvReluScalarBCHW, 1, mchunk, 12, 8, 1, 6, 8, 5> fpscalar_bchw_rand(
  "fpscalar_bchw_rand", weights_rand, bias_rand, "conv_fpin_rand.txt", empty_input, "conv_fpout_ConvReluScalarBCHW_rand.txt");

ConvReluChunkGraphTest<ConvReluScalarBHWC, 0, mchunk, 12, 8, 1, 6, 8, 5> fpscalar_bhwc(
  "fpscalar_bhwc", weights, bias, "conv_fpin.txt", "conv_fpout_ConvReluScalarBHWC.txt");
ConvReluChunkGraphTest<ConvReluScalarBHWC, 0, mchunk, 12, 8, 1, 6, 8, 5> fpscalar_bhwc_rand(
  "fpscalar_bhwc_rand", weights_rand, bias, "conv_fpin_rand.txt", empty_input, "conv_fpout_ConvReluScalarBHWC_rand.txt");

ConvReluChunkGraphTest<Conv5x5ReluBCHW, 1, mchunk, 12, 8, 1, 6, 8, 5> fpvector_bchw(
  "fpvector_bchw", weights, bias, "conv_fpin.txt", "conv_fpout_Conv5x5ReluBCHW.txt");
ConvReluChunkGraphTest<Conv5x5ReluBCHW, 1, mchunk, 12, 8, 1, 6, 8, 5> fpvector_bchw_rand(
  "fpvector_bchw_rand", weights_rand, bias_rand, "conv_fpin_rand.txt", empty_input, "conv_fpout_Conv5x5ReluBCHW_rand.txt");


#ifdef __X86SIM__
int main(int argc, char ** argv) {
  adfCheck(fpscalar_bchw.init(), "init fpscalar_bchw");
  adfCheck(fpscalar_bchw.run(1), "run fpscalar_bchw");
	adfCheck(fpscalar_bchw.end(), "end fpscalar_bchw");
  adfCheck(fpscalar_bchw_rand.init(), "init fpscalar_bchw_rand");
  adfCheck(fpscalar_bchw_rand.run(1), "run fpscalar_bchw_rand");
	adfCheck(fpscalar_bchw_rand.end(), "end fpscalar_bchw_rand");

  adfCheck(fpscalar_bhwc.init(), "init fpscalar_bhwc");
  adfCheck(fpscalar_bhwc.run(1), "run fpscalar_bhwc");
	adfCheck(fpscalar_bhwc.end(), "end fpscalar_bhwc");
  adfCheck(fpscalar_bhwc_rand.init(), "init fpscalar_bhwc_rand");
  adfCheck(fpscalar_bhwc_rand.run(1), "run fpscalar_bhwc_rand");
	adfCheck(fpscalar_bhwc_rand.end(), "end fpscalar_bhwc_rand");

  adfCheck(fpvector_bchw.init(), "init fpvector_bchw");
  adfCheck(fpvector_bchw.run(1), "run fpvector_bchw");
	adfCheck(fpvector_bchw.end(), "end fpvector_bchw");
  adfCheck(fpvector_bchw_rand.init(), "init fpvector_bchw_rand");
  adfCheck(fpvector_bchw_rand.run(1), "run fpvector_bchw_rand");
	adfCheck(fpvector_bchw_rand.end(), "end fpvector_bchw_rand");
  return 0;
}
#endif


#ifdef __AIESIM__
int main(int argc, char ** argv) {
  adfCheck(fpscalar_bchw.init(), "init fpscalar_bchw");
  get_graph_throughput_by_port(fpscalar_bchw, "plout[0]", fpscalar_bchw.plout[0], 1*8*8*8, sizeof(float_t), ITER_CNT);
	adfCheck(fpscalar_bchw.end(), "end fpscalar_bchw");

  adfCheck(fpscalar_bchw_rand.init(), "init fpscalar_bchw_rand");
  get_graph_throughput_by_port(fpscalar_bchw_rand, "plout[0]", fpscalar_bchw_rand.plout[0], 1*8*8*8, sizeof(float_t), ITER_CNT);
	adfCheck(fpscalar_bchw_rand.end(), "end fpscalar_bchw_rand");


  adfCheck(fpscalar_bhwc.init(), "init fpscalar_bhwc");
  get_graph_throughput_by_port(fpscalar_bhwc, "plout[0]", fpscalar_bhwc.plout[0], 1*8*8*8, sizeof(float_t), ITER_CNT);
	adfCheck(fpscalar_bhwc.end(), "end fpscalar_bhwc");

  adfCheck(fpscalar_bhwc_rand.init(), "init fpscalar_bhwc_rand");
  get_graph_throughput_by_port(fpscalar_bhwc_rand, "plout[0]", fpscalar_bhwc_rand.plout[0], 1*8*8*8, sizeof(float_t), ITER_CNT);
	adfCheck(fpscalar_bhwc_rand.end(), "end fpscalar_bhwc_rand");
  

  adfCheck(fpvector_bchw.init(), "init fpvector_bchw");
  get_graph_throughput_by_port(fpvector_bchw, "plout[0]", fpvector_bchw.plout[0], 1*8*8*8, sizeof(float_t), ITER_CNT);
	adfCheck(fpvector_bchw.end(), "end fpvector_bchw");

  adfCheck(fpvector_bchw_rand.init(), "init fpvector_bchw_rand");
  get_graph_throughput_by_port(fpvector_bchw_rand, "plout[0]", fpvector_bchw_rand.plout[0], 1*8*8*8, sizeof(float_t), ITER_CNT);
	adfCheck(fpvector_bchw_rand.end(), "end fpvector_bchw_rand");
  return 0;
}
#endif
