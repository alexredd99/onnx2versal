#ifndef __CONCAT_GRAPH_H__
#define __CONCAT_GRAPH_H__

#include <adf.h>
#include "concat.h"
#define NLANES 8


template <template<int, int, int, int> class CONCAT,
  int LCNT, int WINDOW_SIZE, int CHUNK_SIZE, int BLOCK_SIZE>
class ConcatGraph : public adf::graph {

  private:
    adf::kernel k[1];

  public:
    adf::port<input> pin[NLANES];
    adf::port<output> pout[1];

    ConcatGraph() { 
      k[0] = adf::kernel::create_object<CONCAT<LCNT, WINDOW_SIZE, CHUNK_SIZE, BLOCK_SIZE>>();
      adf::source(k[0]) = "concat.cc";
      adf::runtime<ratio>(k[0]) = 0.6;

      for (int i = 0; i < NLANES; i++) {  
        if (LCNT > i) {
          adf::connect<adf::window<WINDOW_SIZE*4>> (pin[i], k[0].in[i]);
        } else {
          adf::connect<adf::window<4>> (pin[i], k[0].in[i]);
        }
      }
      // BLOCK_SIZE <= WINDOW_SIZE
      adf::connect<adf::window<WINDOW_SIZE/CHUNK_SIZE*BLOCK_SIZE*4>> (k[0].out[0], pout[0]);
    }

};


#endif // __CONCAT_GRAPH_H__