#ifndef IDENTITY_H_
#define IDENTITY_H_

#include <adf.h>


template <int N>
class IdentityScalar {
  public:
    void filter(
      input_window<float>* in,
      output_window<float>* out
    );
    static void registerKernelClass() {
      REGISTER_FUNCTION(IdentityScalar::filter);
    }
};


#endif // IDENTITY_H_
