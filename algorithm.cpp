#include <algorithm>
#include <cassert>
#include <iostream>
#include <stdexcept>

#include "algorithm.hpp"

using std::cerr;
using std::clog;
using std::cout;
using std::endl;

namespace dancing_links {

template<class HNA, class NA, class HN, class NodeT>
AlgorithmC<HNA, NA, HN, NodeT>::AlgorithmC(HNA& hn, NA& n)
  : hn(hn)
  , n(n) {
  static_assert(
    sizeof(L) <= sizeof(typename NA::size_type),
    "Link_type must be smaller or equal to size_type of container.");

  static_assert(
    std::is_same<typename HN::link_type, typename NodeT::link_type>::value,
    "Link types must be the same for header and color nodes!");

  static_assert(std::is_signed_v<C>, "Color must be a signed datatype.");
}

template<class HNA, class NA, class HN, class NodeT>
void
AlgorithmC<HNA, NA, HN, NodeT>::setUseMRV(bool useMRV) {
  m_useMRV = useMRV;
}

template<class HNA, class NA, class HN, class NodeT>
void
AlgorithmC<HNA, NA, HN, NodeT>::set_expected_solution_option_count(
  AlgorithmC::SizeType count) {
  xarr.resize(count);
}

template<class HNA, class NA, class HN, class NodeT>
const typename AlgorithmC<HNA, NA, HN, NodeT>::NodePointerArray&
AlgorithmC<HNA, NA, HN, NodeT>::current_solution() const {
  return xarr;
}

template<class HNA, class NA, class HN, class NodeT>
const typename AlgorithmC<HNA, NA, HN, NodeT>::NodePointerArray&
AlgorithmC<HNA, NA, HN, NodeT>::current_selected_options() const {
  assert(has_solution());
  return selected_options;
}

template<class HNA, class NA, class HN, class NodeT>
const typename AlgorithmC<HNA, NA, HN, NodeT>::NodePointerArray&
AlgorithmC<HNA, NA, HN, NodeT>::current_selected_option_starts() const {
  assert(has_solution());
  return selected_option_starts;
}

template<class HNA, class NA, class HN, class NodeT>
bool
AlgorithmC<HNA, NA, HN, NodeT>::compute_next_solution() {
  try {
    StepResult res;
    do {
      res = step();
    } while(res == CallAgain);

    return res == ResultAvailable;
  } catch(std::invalid_argument& e) {
    last_result = NoResultAvailable;
    return false;
  }
}

template<class HNA, class NA, class HN, class NodeT>
bool
AlgorithmC<HNA, NA, HN, NodeT>::has_solution() const {
  return last_result == ResultAvailable;
}

template<class HNA, class NA, class HN, class NodeT>
bool
AlgorithmC<HNA, NA, HN, NodeT>::continue_calling() const {
  return last_result == NoResultAvailable || last_result == CallAgain;
}

template<class HNA, class NA, class HN, class NodeT>
typename AlgorithmC<HNA, NA, HN, NodeT>::StepResult
AlgorithmC<HNA, NA, HN, NodeT>::step() {
  last_result = stepExec();
  if(last_result == ResultAvailable) {
    update_selected_options();
  }
  return last_result;
}

template<class HNA, class NA, class HN, class NodeT>
void
AlgorithmC<HNA, NA, HN, NodeT>::chooseI() {
  if(m_useMRV) {
    L p = RLINK(0), theta = std::numeric_limits<L>::max();
    while(p != 0) {
      L lambda = LEN(p);
      if(lambda < theta) {
        theta = lambda;
        i = p;
      }
      if(lambda == 0) {
        return;
      }
      p = RLINK(p);
    }
  } else {
    i = RLINK(0);
  }
}

template<class HNA, class NA, class HN, class NodeT>
typename AlgorithmC<HNA, NA, HN, NodeT>::StepResult
AlgorithmC<HNA, NA, HN, NodeT>::stepExec() {
  L p;
  switch(state) {
    case C1:
      if(hn.size() == n.size())
        return NoResultAvailable;

      N = n.size();
      Z = last_spacer_node();
      l = 0;
      j = 0;
      i = 0;
      state = C2;
      return CallAgain;
    case C2:
      if(RLINK(0) == 0) {
        state = C8;
        xarr.resize(l);// The solution only contains l values.
        return ResultAvailable;
      }
      state = C3;
      return CallAgain;
    case C3:
      // One of the possible i from header.
      chooseI();
      state = C4;
      return CallAgain;
    case C4:
      cover(i);
      x(l) = DLINK(i);
      state = C5;
      return CallAgain;
    case C5:
      if(x(l) == i) {
        // Tried all options for i
        state = C7;
        return CallAgain;
      }
      p = x(l) + 1;
      while(p != x(l)) {
        L j = TOP(p);
        if(j <= 0) {
          p = ULINK(p);
        } else {
          // Cover items != i in the option that contains x_l.
          commit(p, j);
          p = p + 1;
        }
      }
      l = l + 1;
      state = C2;
      return CallAgain;
    case C6:
      p = x(l) - 1;
      while(p != x(l)) {
        L j = TOP(p);
        if(j <= 0) {
          p = DLINK(p);
        } else {
          // Uncover items != i in the option that contains x_l, using reverse
          // order.
          uncommit(p, j);
          p = p - 1;
        }
      }
      i = TOP(x(l));
      x(l) = DLINK(x(l));
      state = C5;
      return CallAgain;
    case C7:
      uncover(i);
      state = C8;
      return CallAgain;
    case C8:
      if(l == 0)
        return NoResultAvailable;
      l = l - 1;
      state = C6;
      return CallAgain;
    default:
      assert(false);
  }
  return NoResultAvailable;
}

template<class HNA, class NA, class HN, class NodeT>
typename AlgorithmC<HNA, NA, HN, NodeT>::L
AlgorithmC<HNA, NA, HN, NodeT>::last_spacer_node() {
  L last_spacer = 0;
  for(L i = 0; i < n.size(); ++i) {
    if(TOP(i) < 0) {
      last_spacer = i;
    }
  }
  return last_spacer;
}

template<class HNA, class NA, class HN, class NodeT>
void
AlgorithmC<HNA, NA, HN, NodeT>::cover(AlgorithmC::L i) {
  L p = DLINK(i);
  while(p != i) {
    hide(p);
    p = DLINK(p);
  }
  L l = LLINK(i);
  L r = RLINK(i);
  RLINK(l) = r;
  LLINK(r) = l;
}

template<class HNA, class NA, class HN, class NodeT>
void
AlgorithmC<HNA, NA, HN, NodeT>::hide(AlgorithmC::L p) {
  L q = p + 1;
  while(q != p) {
    L x = TOP(q);
    L u = ULINK(q);
    L d = DLINK(q);

    if(x <= 0) {
      q = u;// q was a spacer
    } else if(COLOR(q) < 0) {
      q = q + 1;
    } else {
      DLINK(u) = d;
      ULINK(d) = u;
      LEN(x) = LEN(x) - 1;
      q = q + 1;
    }
  }
}

template<class HNA, class NA, class HN, class NodeT>
void
AlgorithmC<HNA, NA, HN, NodeT>::uncover(AlgorithmC::L i) {
  L l = LLINK(i);
  L r = RLINK(i);
  RLINK(l) = i;
  LLINK(r) = i;
  L p = ULINK(i);
  while(p != i) {
    unhide(p);
    p = ULINK(p);
  }
}

template<class HNA, class NA, class HN, class NodeT>
void
AlgorithmC<HNA, NA, HN, NodeT>::unhide(AlgorithmC::L p) {
  L q = p - 1;
  while(q != p) {
    L x = TOP(q);
    L u = ULINK(q);
    L d = DLINK(q);
    if(x <= 0) {
      q = d;// q was a spacer
    } else if(COLOR(q) < 0) {
      q = q - 1;
    } else {
      DLINK(u) = q;
      ULINK(d) = q;
      LEN(x) = LEN(x) + 1;
      q = q - 1;
    }
  }
}

template<class HNA, class NA, class HN, class NodeT>
void
AlgorithmC<HNA, NA, HN, NodeT>::commit(AlgorithmC::L p, AlgorithmC::L j) {
  if(COLOR(p) == 0)
    cover(j);
  else if(COLOR(p) > 0)
    purify(p);
}

template<class HNA, class NA, class HN, class NodeT>
void
AlgorithmC<HNA, NA, HN, NodeT>::purify(AlgorithmC::L p) {
  C c = COLOR(p);
  L i = TOP(p);
  for(L q = DLINK(i); q != i; q = DLINK(q)) {
    if(COLOR(q) == c)
      COLOR(q) = -1;
    else
      hide(q);
  }
}

template<class HNA, class NA, class HN, class NodeT>
void
AlgorithmC<HNA, NA, HN, NodeT>::uncommit(AlgorithmC::L p, AlgorithmC::L j) {
  if(COLOR(p) == 0)
    uncover(j);
  else if(COLOR(p) > 0)
    unpurify(p);
}

template<class HNA, class NA, class HN, class NodeT>
void
AlgorithmC<HNA, NA, HN, NodeT>::unpurify(AlgorithmC::L p) {
  C c = COLOR(p);
  L i = TOP(p);
  for(L q = DLINK(i); q != i; q = DLINK(q)) {
    if(COLOR(q) < 0)
      COLOR(q) = c;
    else
      unhide(q);
  }
}

template<class HNA, class NA, class HN, class NodeT>
typename AlgorithmC<HNA, NA, HN, NodeT>::L&
AlgorithmC<HNA, NA, HN, NodeT>::LLINK(AlgorithmC::L i) {
  assert(i < hn.size());
  return hn[i].LLINK;
}

template<class HNA, class NA, class HN, class NodeT>
typename AlgorithmC<HNA, NA, HN, NodeT>::L&
AlgorithmC<HNA, NA, HN, NodeT>::RLINK(AlgorithmC::L i) {
  assert(i < hn.size());
  return hn[i].RLINK;
}

template<class HNA, class NA, class HN, class NodeT>
typename AlgorithmC<HNA, NA, HN, NodeT>::NameT&
AlgorithmC<HNA, NA, HN, NodeT>::NAME(AlgorithmC::L i) {
  assert(i < hn.size());
  return hn[i].NAME;
}

template<class HNA, class NA, class HN, class NodeT>
typename AlgorithmC<HNA, NA, HN, NodeT>::L&
AlgorithmC<HNA, NA, HN, NodeT>::ULINK(AlgorithmC::L i) {
  assert(i < n.size());
  return n[i].ULINK;
}

template<class HNA, class NA, class HN, class NodeT>
typename AlgorithmC<HNA, NA, HN, NodeT>::L&
AlgorithmC<HNA, NA, HN, NodeT>::DLINK(AlgorithmC::L i) {
  assert(i < n.size());
  return n[i].DLINK;
}

template<class HNA, class NA, class HN, class NodeT>
typename AlgorithmC<HNA, NA, HN, NodeT>::L&
AlgorithmC<HNA, NA, HN, NodeT>::TOP(AlgorithmC::L i) {
  assert(i < n.size());
  return n[i].TOP;
}

template<class HNA, class NA, class HN, class NodeT>
typename AlgorithmC<HNA, NA, HN, NodeT>::L
AlgorithmC<HNA, NA, HN, NodeT>::TOP(AlgorithmC::L i) const {
  assert(i < n.size());
  return n[i].TOP;
}

template<class HNA, class NA, class HN, class NodeT>
typename AlgorithmC<HNA, NA, HN, NodeT>::L&
AlgorithmC<HNA, NA, HN, NodeT>::LEN(AlgorithmC::L i) {
  assert(i < n.size());
  return n[i].LEN;
}

template<class HNA, class NA, class HN, class NodeT>
typename AlgorithmC<HNA, NA, HN, NodeT>::C&
AlgorithmC<HNA, NA, HN, NodeT>::COLOR(AlgorithmC::L i) {
  assert(i < n.size());
  auto& c = n[i].COLOR;
  if(c == NodeT::color_undefined) {
    throw std::invalid_argument(
      "Undefined Color! This means an item never occurs in the options!");
  }
  return c;
}

template<class HNA, class NA, class HN, class NodeT>
typename AlgorithmC<HNA, NA, HN, NodeT>::NodePointerArray::value_type&
AlgorithmC<HNA, NA, HN, NodeT>::x(AlgorithmC::L i) {
  if(i >= xarr.size()) {
    xarr.resize(i + 1);
  }
  return xarr[i];
}

template<class HNA, class NA, class HN, class NodeT>
typename AlgorithmC<HNA, NA, HN, NodeT>::NodePointerArray::value_type
AlgorithmC<HNA, NA, HN, NodeT>::x(AlgorithmC::L i) const {
  assert(i < xarr.size());
  return xarr[i];
}

template<class HNA, class NA, class HN, class NodeT>
void
AlgorithmC<HNA, NA, HN, NodeT>::update_selected_options() {
  selected_options.resize(l);
  selected_option_starts.resize(l);
  for(L j = 0; j < l; ++j) {
    L r = x(j);
    while(TOP(r) >= 0) {
      ++r;
    }
    selected_options[j] = -TOP(r);
    selected_option_starts[j] = ULINK(r);
  }
}

template<class HNA, class NA, class HN, class NodeT>
std::ostream&
operator<<(std::ostream& o, const AlgorithmC<HNA, NA, HN, NodeT>& c) {
  using L = typename NodeT::link_type;
  using C = typename NodeT::color_type;

  if(c.hn.size() < 1 || c.n.size() < 1)
    return o << "{[],[]}";

  o << "{[" << c.hn[0];
  for(L i = 1; i < c.hn.size(); ++i) {
    o << "," << endl << c.hn[i];
  }
  o << "],[" << c.n[0];
  for(L i = 1; i < c.n.size(); ++i) {
    o << "," << endl << c.n[i];
  }
  return o << "]}";
}

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

template class AlgorithmC<HNodeVectorChar, NodeVectorChar>;
template class AlgorithmC<HNodeVectorInt32, NodeVectorInt32>;

}
