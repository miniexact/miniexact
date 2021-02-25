#include "delta-debug-problem.hpp"
#include "algorithm.hpp"
#include <algorithm>
#include <limits>
#include <mutex>
#include <sstream>
#include <thread>

#include <boost/dynamic_bitset.hpp>
#include <boost/log/trivial.hpp>

using std::cerr;
using std::clog;
using std::cout;
using std::endl;

extern bool log_verbose;

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
DeltaDebugProblem<P>::DeltaDebugProblem(P& p, bool deep, AlgoOptionsApplier o)
  : m_p(p)
  , m_deep(deep)
  , m_o(o) {}

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
  auto test = [this](const CECP& problem,
                     std::vector<char> activeOptions) -> bool {
    return !satisfiable(problem, activeOptions);
  };

  std::optional<CECP> cecp = [this, &test]() {
    if(m_deep) {
      return deep_explore(test);
    } else {
      return ddmin(test, m_p, BooleanTrueOfLengthN(m_p.getOptionCount()), 2);
    }
  }();

  if(cecp && cecp->getOptionCount() > 0) {
    return getProblemFromCECP(m_p, cecp.value());
  }
  return std::nullopt;
}

template<class P>
std::optional<P>
DeltaDebugProblem<P>::make_sat_by_removing_options() {
  auto test = [this](const CECP& problem,
                     std::vector<char> activeOptions) -> bool {
    return satisfiable(problem, activeOptions);
  };

  std::optional<CECP> cecp = [this, &test]() {
    if(m_deep) {
      return deep_explore(test);
    } else {
      return ddmin(test, m_p, BooleanTrueOfLengthN(m_p.getOptionCount()), 2);
    }
  }();

  if(cecp && cecp->getOptionCount() > 0) {
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

  for(size_t i = 0; i < n; ++i) {
    size_t phys_i = getPhysNOffsetInActiveOptionVector(activeOptions, i);
    assert(phys_i < activeOptions.size());
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

static bool
increment_bitset(boost::dynamic_bitset<>& bitset, size_t size) {
  size_t i;
  for(i = 0; i < size; ++i) {
    if(!bitset.test(i)) {
      bitset.set(i);
      break;
    }
    bitset.reset(i);
  }
  return i < size;
}

template<class P>
std::optional<typename DeltaDebugProblem<P>::CECP>
DeltaDebugProblem<P>::deep_explore(TestPredicate test) {
  struct GenerateActiveOptionsArr {
    GenerateActiveOptionsArr(size_t numberOfOptions)
      : m_size(numberOfOptions)
      , m_bitset(numberOfOptions) {}
    bool generate(std::vector<char>& targetActiveOptionsArr) {
      std::unique_lock lock(m_mtx);
      assert(targetActiveOptionsArr.size() == m_size);

      if(m_done) {
        return false;
      }

      if(!increment_bitset(m_bitset, m_size)) {
        m_done = true;
      }

      for(size_t i = 0; i < m_size; ++i) {
        targetActiveOptionsArr[i] = m_bitset[i];
      }

      return true;
    }

    std::vector<char> initArr() { return std::vector<char>(m_size); }

    void insertResultWithNegativeTest(CECP& res,
                                      std::vector<char>& activeOptions) {
      size_t size =
        std::count(activeOptions.begin(), activeOptions.end(), true);

      std::unique_lock lock(m_currentShortestMtx);

      if(size < m_currentShortestSize) {
        m_currentShortest = res;
        m_currentShortestSize = size;
        m_currentShortestActiveOptions = activeOptions;
      }
    }

    CECP& shortest() { return m_currentShortest; }
    std::vector<char>& shortestActiveOptions() {
      return m_currentShortestActiveOptions;
    }

    private:
    size_t m_size;
    boost::dynamic_bitset<> m_bitset;
    std::mutex m_mtx;
    bool m_done = false;

    std::mutex m_currentShortestMtx;
    CECP m_currentShortest;
    size_t m_currentShortestSize = std::numeric_limits<size_t>::max();
    std::vector<char> m_currentShortestActiveOptions;
  };

  GenerateActiveOptionsArr generator(m_p.getOptionCount());

  auto workerFunc = [&generator, &test, this]() {
    std::vector<char> activeOptions = generator.initArr();
    while(generator.generate(activeOptions)) {
      CECP reduced =
        MakeProblemFromProblemWithActiveOptions<P, CECP>(m_p, activeOptions);
      bool testResult = !test(reduced, activeOptions);
      BOOST_LOG_TRIVIAL(trace)
        << "Result of options: " << activeOptions << " is " << testResult;
      if(!testResult) {
        generator.insertResultWithNegativeTest(reduced, activeOptions);
      }
    }
  };

  std::vector<std::thread> threads;
  threads.reserve(std::thread::hardware_concurrency());
  for(size_t i = 0; i < std::thread::hardware_concurrency(); ++i) {
    threads.emplace_back(workerFunc);
  }

  std::for_each(threads.begin(), threads.end(), [](auto& t) { t.join(); });

  return generator.shortest();
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
  if(cecp.getOptionCount() == 0)
    return false;

  typename TriedOptionSetsMap::iterator it;
  {
    std::unique_lock lock(m_triedOptionSetsMtx);
    it = m_triedOptionSets.find(activeOptions);
  }

  bool sat;

  if(it == m_triedOptionSets.end()) {
    typename CECP::HNA hna(cecp.hna);
    typename CECP::NA na(cecp.na);

    if(log_verbose) {
      auto p = getProblemFromCECP(m_p, cecp);
      std::stringstream sstream;
      p.printMapped(sstream, true);
      BOOST_LOG_TRIVIAL(trace) << "Checking problem: " << sstream.str();
    }

    Algo algo(hna, na);

    if(m_o) {
      m_o(algo);
    }

    sat = algo.compute_next_solution();

    {
      std::unique_lock lock(m_triedOptionSetsMtx);
      m_triedOptionSets[activeOptions] = sat;
    }
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
