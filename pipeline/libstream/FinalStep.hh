#ifndef __FINALSTEP_HH__
#define __FINALSTEP_HH__

#include "Handler.hh"

class FinalStep: public Handler {
public:
  void process(frame_t& frame, uint32_t *params, void* addr);

protected:
  FinalStep();
  ~FinalStep();

  virtual void doProcess(frame_t& frame, uint32_t *params, void* addr) = 0;
};

#endif
