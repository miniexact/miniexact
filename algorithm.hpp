#pragma once

#include <limits>
#include <ostream>
#include <vector>

#include "nodes.hpp"

namespace dancing_links {

template<class HNA,
         class NA,
         class HN = typename HNA::value_type,
         class NodeT = typename NA::value_type>
class AlgorithmC {
  public:
  enum StepResult { ResultAvailable, NoResultAvailable, CallAgain };

  using L = typename NodeT::link_type;
  using C = typename NodeT::color_type;
  using NameT = typename HN::name_type;
  using NodePointerArray = std::vector<L>;
  using SizeType = typename NodePointerArray::size_type;

  enum AlgorithmState { C1, C2, C3, C4, C5, C6, C7, C8, _STATE_COUNT };
  const char* AlgorithmStateStr[_STATE_COUNT] = { "C1", "C2", "C3", "C4",
                                                  "C5", "C6", "C7", "C8" };
  constexpr const char* AlgorithmStateToStr(AlgorithmState s) {
    return AlgorithmStateStr[s];
  }

  AlgorithmC(HNA& hn, NA& n);
  ~AlgorithmC() = default;

  void setUseMRV(bool useMRV);

  void set_expected_solution_option_count(SizeType count);

  const NodePointerArray& current_solution() const;

  const NodePointerArray& current_selected_options() const;
  const NodePointerArray& current_selected_option_starts() const;

  bool compute_next_solution();

  bool has_solution() const;
  bool continue_calling() const;

  StepResult step();

  friend std::ostream& operator<<(std::ostream& o,
                                  const AlgorithmC<HNA, NA, HN, NodeT>& c);

  protected:
  AlgorithmState state = C1;
  L N;
  L Z;
  L l, j, i;

  StepResult last_result = CallAgain;

  void chooseI();

  StepResult stepExec();

  L last_spacer_node();

  void cover(L i);
  void hide(L p);
  void uncover(L i);
  void unhide(L p);
  void commit(L p, L j);
  void purify(L p);
  void uncommit(L p, L j);
  void unpurify(L p);

  L& LLINK(L i);
  L& RLINK(L i);
  NameT& NAME(L i);
  L& ULINK(L i);
  L& DLINK(L i);
  L& TOP(L i);
  L TOP(L i) const;
  L& LEN(L i);
  C& COLOR(L i);

  typename NodePointerArray::value_type& x(L i);
  typename NodePointerArray::value_type x(L i) const;

  void update_selected_options();

  HNA& hn;
  NA& n;

  NodePointerArray xarr;
  NodePointerArray selected_options;
  NodePointerArray selected_option_starts;

  bool m_useMRV = false;
};

template<typename C>
using HNode = HeaderNode<std::int32_t, C>;
template<typename C>
using Node = ColoredNode<std::int32_t, C>;
template<typename C>
using HNodeVector = std::vector<HNode<C>>;
template<typename C>
using NodeVector = std::vector<Node<C>>;

using HNodeVectorChar = HNodeVector<char>;
using NodeVectorChar = NodeVector<char>;
using HNodeVectorInt32 = HNodeVector<std::int32_t>;
using NodeVectorInt32 = NodeVector<std::int32_t>;
using HNodeVectorInt = std::vector<HeaderNode<int, int>>;
using NodeVectorInt = std::vector<ColoredNode<int, int>>;

using AlgorithmCInt32Char = AlgorithmC<HNodeVectorChar, NodeVectorChar>;
using AlgorithmCInt32Int32 = AlgorithmC<HNodeVectorInt32, NodeVectorInt32>;
using AlgorithmCInt32Int = AlgorithmC<HNodeVectorInt, NodeVectorInt>;

extern template class AlgorithmC<HNodeVectorChar, NodeVectorChar>;
extern template class AlgorithmC<HNodeVectorInt32, NodeVectorInt32>;
extern template class AlgorithmC<HNodeVectorInt, NodeVectorInt>;

}
