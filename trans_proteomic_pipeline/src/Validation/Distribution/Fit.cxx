#include "Fit.h"

Fit::Fit(Array<double>* grid, Array<double>* density) {
  grid_ = grid;
  dens_ = density;
  varbws_ = NULL;
  int r =  grid_->size()-1;
  gridwid_ = ((*grid_)[r]-(*grid_)[0]) / r;
}

Fit::Fit(Array<double>* grid, Array<double>* density, Array<double>* varbws) {
  grid_ = grid;
  dens_ = density;
  varbws_ = varbws;;
  int r =  grid_->size()-1;
  gridwid_ = ((*grid_)[r]-(*grid_)[0]) / r;
}
Fit::~Fit() {
// grid_ and varbws_ managed by client code (KDModel)
  if (dens_ != NULL)
    delete dens_;  
}
