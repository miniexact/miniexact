#pragma once

#include <functional>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

#include "algorithm.hpp"
#include "problem.hpp"

namespace dancing_links {
template<class P>
class DeltaDebugProblem {
  public:
  using I = typename P::ITEM;
  using C = typename P::COLOR;
  using CECP = ColoredExactCoveringProblem<I, C>;
  using Algo = AlgorithmC<decltype(CECP::hna), decltype(CECP::na)>;
  using AlgoOptionsApplier = std::function<void(Algo& a)>;

  DeltaDebugProblem(P& p, bool deep = false, AlgoOptionsApplier o = nullptr);
  ~DeltaDebugProblem();

  std::optional<P> keep_sat_while_removing_options();
  std::optional<P> make_sat_by_removing_options();

  private:
  using TestPredicate =
    std::function<bool(const CECP&, const std::vector<char>& activeOptions)>;

  std::optional<CECP> ddmin(TestPredicate test,
                            const CECP& problem,
                            std::vector<char> activeOptions,
                            size_t n);

  std::optional<CECP> deep_explore(TestPredicate test);

  static P getProblemFromCECP(const P& problem, const CECP& cecp);

  P& m_p;
  bool m_deep;
  AlgoOptionsApplier m_o;

  bool satisfiable(const CECP& p, const std::vector<char>& activeOptions);

  // Taken from https://stackoverflow.com/a/10405129
  template<typename Container>
  struct container_hash {
    std::size_t operator()(Container const& c) const {
      return boost::hash_range(c.begin(), c.end());
    }
  };

  using TriedOptionSetsMap = std::
    unordered_map<std::vector<char>, bool, container_hash<std::vector<char>>>;
  TriedOptionSetsMap m_triedOptionSets;
  std::mutex m_triedOptionSetsMtx;
};

extern template class DeltaDebugProblem<
  MappedColoredExactCoveringProblemInt32String>;
extern template class DeltaDebugProblem<
  MappedColoredExactCoveringProblemInt32Char32>;
extern template class DeltaDebugProblem<
  ColoredExactCoveringProblem<int32_t, int32_t>>;
}
