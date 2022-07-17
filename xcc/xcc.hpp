#ifndef XCC_HPP
#define XCC_HPP

#include <memory>

#include "xcc.h"

struct xcc_problem_deleter {
  void operator()(xcc_problem* p) const { xcc_problem_free(p); }
};

using xcc_problem_ptr = std::unique_ptr<xcc_problem, xcc_problem_deleter>;

#endif
