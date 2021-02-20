#include "delta-debug-problem.hpp"
#include "algorithm.hpp"
#include <algorithm>

#include <boost/log/trivial.hpp>

using std::cerr;
using std::clog;
using std::cout;
using std::endl;

namespace std {
std::ostream&
operator<<(std::ostream& o, const std::vector<char> v) {
  for(auto c : v) {
    o << ((c == true) ? '1' : '0') << " ";
  }
  return o;
}
}

namespace dancing_links {

template<class P>
DeltaDebugProblem<P>::DeltaDebugProblem(P& p)
  : m_p(p) {}

template<class P>
DeltaDebugProblem<P>::~DeltaDebugProblem() {}

std::vector<char>
BooleanTrueOfLengthN(size_t n) {
  std::vector<char> v(n);
  std::fill(v.begin(), v.end(), true);
  return std::move(v);
}

template<class P>
std::optional<P>
DeltaDebugProblem<P>::keep_sat_while_removing_options() {
  std::optional<CECP> cecp = ddmin(
    [this](const CECP& problem, std::vector<char> activeOptions) -> bool {
      return !satisfiable(problem, activeOptions);
    },
    m_p,
    BooleanTrueOfLengthN(m_p.getOptionCount()),
    2);

  if(cecp) {
    return getProblemFromCECP(m_p, cecp.value());
  }
  return std::nullopt;
}

template<class P>
std::optional<P>
DeltaDebugProblem<P>::make_sat_by_removing_options() {
  std::optional<CECP> cecp = ddmin(
    [this](const CECP& problem, std::vector<char> activeOptions) -> bool {
      return satisfiable(problem, activeOptions);
    },
    m_p,
    BooleanTrueOfLengthN(m_p.getOptionCount()),
    2);

  if(cecp) {
    return getProblemFromCECP(m_p, cecp.value());
  }
  return std::nullopt;
}

template<class P, class TGT>
TGT
MakeProblemFromProblemWithActiveOptions(const P& source,
                                        std::vector<char> activeOptions) {
  TGT target = P::copyItems(source);
  auto activeIt = activeOptions.begin();
  for(auto o : source) {
    if(*(activeIt++)) {
      target.addOption(o);
    }
  }
  return target;
}

size_t
getPhysNOffsetInActiveOptionVector(const std::vector<char>& v, size_t n) {
  size_t phys_i = 0;
  for(size_t logical_i = 0; logical_i != n; ++logical_i, ++phys_i) {
    while(!v[phys_i]) {
      ++phys_i;
      assert(phys_i < v.size());
    }
  }
  return phys_i;
}

template<class P>
std::optional<typename DeltaDebugProblem<P>::CECP>
DeltaDebugProblem<P>::ddmin(TestPredicate test,
                            const CECP& problem,
                            std::vector<char> activeOptions,
                            size_t n) {
  size_t active = std::count(activeOptions.begin(), activeOptions.end(), true);

  if(active == 1) {
    return problem;
  }

  for(size_t i = 0; i <= n; ++i) {

    size_t phys_i = getPhysNOffsetInActiveOptionVector(activeOptions, i);
    activeOptions[phys_i] = false;

    if(m_triedOptionSets.count(activeOptions)) {
      BOOST_LOG_TRIVIAL(debug) << "Already cached option configuration "
                               << activeOptions << ", not checking again.";
      activeOptions[phys_i] = true;
      continue;
    }

    CECP reduced =
      MakeProblemFromProblemWithActiveOptions<P, CECP>(m_p, activeOptions);
    bool testResult = !test(reduced, activeOptions);
    BOOST_LOG_TRIVIAL(debug)
      << "Testing with options " << activeOptions << ". Result: " << testResult;
    if(!testResult) {
      return ddmin(test, reduced, activeOptions, std::max(n - 1, 2Lu));
    }

    activeOptions[phys_i] = true;
  }

  if(n < active) {
    return ddmin(test, problem, activeOptions, std::min(2 * n, active));
  }

  return problem;
}

template<class P>
P
DeltaDebugProblem<P>::getProblemFromCECP(const P& p, const CECP& cecp) {
  P newP = [&]() {
    if constexpr(std::is_same_v<P,
                                MappedColoredExactCoveringProblemInt32String> ||
                 std::is_same_v<P,
                                MappedColoredExactCoveringProblemInt32Char32>) {
      return P::copyItemsMapped(p);
    } else {
      return P::copyItems(p);
    }
  }();

  for(auto o : cecp) {
    newP.addOption(o);
  }

  return newP;
}

template<class P>
bool
DeltaDebugProblem<P>::satisfiable(const CECP& cecp,
                                  const std::vector<char>& activeOptions) {
  auto it = m_triedOptionSets.find(activeOptions);

  bool sat;

  if(it == m_triedOptionSets.end()) {
    typename CECP::HNA hna(cecp.hna);
    typename CECP::NA na(cecp.na);

    // auto p = getProblemFromCECP(m_p, cecp);
    // p.printMapped(std::cout);

    AlgorithmC algo(hna, na);
    sat = algo.compute_next_solution();
    m_triedOptionSets[activeOptions] = sat;
  } else {
    sat = it->second;
  }

  return sat;
}

template class DeltaDebugProblem<MappedColoredExactCoveringProblemInt32String>;
template class DeltaDebugProblem<MappedColoredExactCoveringProblemInt32Char32>;
// template class DeltaDebugProblem<ColoredExactCoveringProblem<int32_t,
// int32_t>>;
}
