#ifndef _FIT_H_
#define _FIT_H_
#include "common/sysdepend.h"
#include "common/Array.h"
class Fit {
  friend class NonParametricDistribution;
  friend class KDModel;
 public:
  Fit();
  Fit(Array<double>* grid, Array<double>* density);
  Fit(Array<double>* grid, Array<double>* density, Array<double>* varbws);
  ~Fit();

 private:
  Array<double>* grid_;
  Array<double>* dens_; 
  Array<double>* varbws_; 
  double gridwid_;
};
#endif
