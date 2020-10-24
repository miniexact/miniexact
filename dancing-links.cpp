#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <vector>

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

namespace dancing_links {
using std::cerr;
using std::clog;
using std::cout;
using std::endl;

template<typename T, typename N>
struct HeaderNode {
  using link_type = T;
  using name_type = N;

  HeaderNode(N n, T l, T r)
    : LLINK(l)
    , RLINK(r)
    , NAME(n) {}

  N NAME;
  T LLINK;
  T RLINK;

  friend std::ostream& operator<<(std::ostream& o, HeaderNode& n) {
    return o << "[" << n.NAME << "," << n.LLINK << "," << n.RLINK << "]";
  }
};

template<typename T, typename C>
struct ColoredNode {
  using link_type = T;
  using color_type = C;
  static const C color_undefined = std::numeric_limits<C>::max();

  ColoredNode(T len, T u, T d)
    : ULINK(u)
    , DLINK(d)
    , LEN(len)
    , COLOR(color_undefined) {}
  ColoredNode(T t, T u, T d, C c)
    : ULINK(u)
    , DLINK(d)
    , TOP(t)
    , COLOR(c) {}

  T ULINK;
  T DLINK;
  union {
    T TOP;
    T LEN;
  };
  C COLOR;

  friend std::ostream& operator<<(std::ostream& o, ColoredNode& n) {
    return o << "[" << n.TOP << "," << n.ULINK << "," << n.DLINK << ","
             << n.COLOR << "]";
  }
};

#ifdef DEBUG
#define VIRT virtual
#else
#define VIRT
#endif

template<class HNA,
         class NA,
         class HN = typename HNA::value_type,
         class NodeT = typename NA::value_type>
class AlgorithmC {
  public:
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

  AlgorithmC(HNA& hn, NA& n)
    : hn(hn)
    , n(n) {
    static_assert(
      sizeof(L) <= sizeof(typename NA::size_type),
      "Link_type must be smaller or equal to size_type of container.");

    static_assert(
      std::is_same<typename HN::link_type, typename NodeT::link_type>::value,
      "Link types must be the same for header and color nodes!");
  }
  VIRT ~AlgorithmC() = default;

  void set_expected_solution_option_count(SizeType count) {
    xarr.resize(count);
  }

  const NodePointerArray& current_solution() const { return xarr; }

  bool compute_next_solution() {
    StepResult res;
    do {
      res = step();
    } while(res == CallAgain);

    return res == ResultAvailable;
  }

  friend std::ostream& operator<<(std::ostream& o, AlgorithmC& c) {
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

  protected:
  AlgorithmState state = C1;
  L N;
  L Z;
  L l, j, i;

  enum StepResult { ResultAvailable, NoResultAvailable, CallAgain };

  VIRT StepResult step() {
    L p;
    switch(state) {
      case C1:
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
        i = RLINK(0);
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

  L last_spacer_node() {
    L last_spacer = 0;
    for(L i = 0; i < n.size(); ++i) {
      if(TOP(i) < 0) {
        last_spacer = i;
      }
    }
    return last_spacer;
  }

  VIRT void cover(L i) {
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
  VIRT void hide(L p) {
    L q = p + 1;
    while(q != p) {
      L x = TOP(q);
      L u = ULINK(q);
      L d = DLINK(q);

      if(x <= 0) {
        cout << "SPACER with x:" << x << ", q:" << q << ", u:" << u << endl;
        assert(q != u);
        q = u;// q was a spacer
      } else if(COLOR(q) < 0) {
        cout << "COLOR" << endl;
        q = u;// q is ignored because of color (page 88)
      } else {
        DLINK(u) = d;
        ULINK(d) = u;
        LEN(x) = LEN(x) - 1;
        q = q + 1;
      }
    }
  }
  VIRT void uncover(L i) {
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
  VIRT void unhide(L p) {
    L q = p - 1;
    while(q != p) {
      L x = TOP(q);
      L u = ULINK(q);
      L d = DLINK(q);
      if(x <= 0) {
        q = d;// q was a spacer
      } else if(COLOR(q) < 0) {
        q = d;// q is ignored because of color (page 88)
      } else {
        DLINK(u) = q;
        ULINK(d) = q;
        LEN(x) = LEN(x) + 1;
        q = q - 1;
      }
    }
  }
  VIRT void commit(L p, L j) {
    if(COLOR(p) == 0)
      cover(j);
    else if(COLOR(p) > 0)
      purify(p);
  }
  VIRT void purify(L p) {
    C c = COLOR(p);
    L i = TOP(p);
    L q = DLINK(i);
    while(q != i) {
      q = DLINK(q);
      if(COLOR(q) == c)
        COLOR(q) = -1;
      else
        hide(q);
    }
  }
  VIRT void uncommit(L p, L j) {
    if(COLOR(p) == 0)
      uncover(j);
    else if(COLOR(p) > 0)
      unpurify(p);
  }
  VIRT void unpurify(L p) {
    C c = COLOR(p);
    L i = TOP(p);
    L q = ULINK(i);
    while(q != i) {
      q = ULINK(q);
      if(COLOR(q) < 0)
        COLOR(q) = c;
      else
        unhide(q);
    }
  }

  L& LLINK(L i) {
    assert(i < hn.size());
    return hn[i].LLINK;
  }
  L& RLINK(L i) {
    assert(i < hn.size());
    return hn[i].RLINK;
  }
  NameT& NAME(L i) {
    assert(i < hn.size());
    return hn[i].NAME;
  }
  L& ULINK(L i) {
    assert(i < n.size());
    return n[i].ULINK;
  }
  L& DLINK(L i) {
    assert(i < n.size());
    return n[i].DLINK;
  }
  L& TOP(L i) {
    assert(i < n.size());
    return n[i].TOP;
  }
  L& LEN(L i) {
    assert(i < n.size());
    return n[i].LEN;
  }
  C& COLOR(L i) {
    assert(i < n.size());
    auto& c = n[i].COLOR;
    assert(c != NodeT::color_undefined);
    return c;
  }

  typename NodePointerArray::value_type& x(L i) {
    if(i >= xarr.size()) {
      xarr.resize(i + 1);
    }
    return xarr[i];
  }

  HNA& hn;
  NA& n;

  NodePointerArray xarr;
};

using HNode = HeaderNode<std::int32_t, char>;
using Node = ColoredNode<std::int32_t, char>;
using NodeVector = std::vector<Node>;
using HNodeVector = std::vector<HNode>;

#define IGN 0

#ifdef DEBUG
template<class HNA, class NA>
class ShoutingAlgorithmC : public AlgorithmC<HNA, NA> {
  public:
  using Base = AlgorithmC<HNA, NA>;

  ShoutingAlgorithmC(HNA& hn, NA& n)
    : Base(hn, n) {}

  using L = Base::L;

  VIRT ~ShoutingAlgorithmC() = default;

  VIRT Base::StepResult step() {
    cout << Base::AlgorithmStateToStr(Base::state) << " -> " << endl;
    auto res = Base::step();
    cout << " -> " << Base::AlgorithmStateToStr(Base::state)
         << " Result: " << res << endl;

    return res;
  }

  VIRT void cover(L i) {
    cout << Base::AlgorithmStateToStr(Base::state) << ": cover " << i << endl;
    Base::cover(i);
  }
  VIRT void hide(L p) {
    cout << Base::AlgorithmStateToStr(Base::state) << ": hide " << p << endl;
    Base::hide(p);
  }
  VIRT void uncover(L i) {
    cout << Base::AlgorithmStateToStr(Base::state) << ": uncover " << i << endl;
    Base::uncover(i);
  }
  VIRT void unhide(L p) {
    cout << Base::AlgorithmStateToStr(Base::state) << ": unhide " << p << endl;
    Base::unhide(p);
  }
  VIRT void commit(L p, L j) {
    cout << Base::AlgorithmStateToStr(Base::state) << ": commit " << p << ", "
         << j << endl;
    Base::commit(p, j);
  }
  VIRT void purify(L p) {
    cout << Base::AlgorithmStateToStr(Base::state) << ": purify " << p << endl;
    Base::purify(p);
  }
  VIRT void uncommit(L p, L j) {
    cout << Base::AlgorithmStateToStr(Base::state) << ": uncommit " << p << ", "
         << j << endl;
    Base::uncommit(p, j);
  }
  VIRT void unpurify(L p) {
    cout << Base::AlgorithmStateToStr(Base::state) << ": unpurify " << p
         << endl;
    Base::unpurify(p);
  }
};

auto
produce_vectors_for_example_49() {
  using H = HNode;
  using N = Node;
  auto pair =
    std::pair<HNodeVector, NodeVector>(HNodeVector{ H(' ', 3, 1),
                                                    H('p', 0, 2),
                                                    H('q', 1, 3),
                                                    H('r', 2, 0),
                                                    H('x', 6, 5),
                                                    H('y', 4, 6),
                                                    H(' ', 5, 4) },

                                       NodeVector{ // Row 2
                                                   N(IGN, IGN, IGN),
                                                   N(3, 17, 7),
                                                   N(2, 20, 8),
                                                   N(2, 23, 13),
                                                   N(3, 21, 9),
                                                   N(3, 24, 10),
                                                   N(0, IGN, 10),
                                                   // Row 3
                                                   N(1, 1, 12, 0),
                                                   N(2, 2, 20, 0),
                                                   N(4, 4, 14, 0),
                                                   N(5, 5, 15, 'A'),
                                                   N(-1, 7, 15, 0),
                                                   N(1, 7, 17, 0),
                                                   N(3, 3, 23, 0),
                                                   // Row 4
                                                   N(4, 9, 18, 'A'),
                                                   N(5, 10, 24, 0),
                                                   N(-2, 12, 18, 0),
                                                   N(1, 12, 1, 0),
                                                   N(4, 14, 21, 'B'),
                                                   N(-3, 17, 21, 0),
                                                   N(2, 8, 2, 0),
                                                   // Row 5
                                                   N(4, 18, 4, 'A'),
                                                   N(-4, 20, 24, 0),
                                                   N(3, 13, 3, 0),
                                                   N(5, 15, 5, 'B'),
                                                   N(-5, 23, IGN, 0) });

  // Check sizes compared to the book indexes.
  CHECK(pair.first.size() == 7);
  CHECK(pair.second.size() == 26);
  return pair;
}

TEST_CASE("Algorithm C example problem from page 87") {
  auto vecs = produce_vectors_for_example_49();
  auto& hnvec = vecs.first;
  auto& nvec = vecs.second;

  ShoutingAlgorithmC<HNodeVector, NodeVector> xcc(hnvec, nvec);

  auto& s = xcc.current_solution();

  while(bool solution_available = xcc.compute_next_solution()) {
    cout << "Solution: " << endl;
    std::for_each(
      s.begin(), s.end(), [&nvec](auto n) { cout << nvec[n].TOP << endl; });
  }

  // REQUIRE(solution_available);
  // REQUIRE(s.size() == 2);
  // REQUIRE(nvec[s[0]].TOP == 1);
  // REQUIRE(nvec[s[1]].TOP == 3);
}

#endif

class WordPuzzle {
  public:
  WordPuzzle()
    : xcc(hnodes, nodes) {}
  ~WordPuzzle() {}

  private:
  HNodeVector hnodes;
  NodeVector nodes;
  AlgorithmC<HNodeVector, NodeVector> xcc;
};
}

int
main(int argc, const char* argv[]) {
  if(argc > 1 && strcmp(argv[1], "--test") == 0) {
    doctest::Context ctx;

    ctx.setOption("no-breaks", false);

    ctx.applyCommandLine(argc, argv);
    int res = ctx.run();
    ctx.shouldExit();
    return res;
  }

  using namespace dancing_links;
  WordPuzzle puzzle;

  return 0;
}
