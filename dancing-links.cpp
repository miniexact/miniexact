#include <cstdint>
#include <iostream>
#include <vector>

namespace dancing_links {
using std::cerr;
using std::clog;
using std::cout;
using std::endl;

template<typename T, typename C>
struct ColoredNode {
  using link_type = T;
  using color_type = C;

  union {
    T LLINK;
    T ULINK;
  };
  union {
    T RLINK;
    T DLINK;
  };
  union {
    T TOP;
    T LEN;
  };
  C color;
};

template<class NA, class N = typename NA::value_type>
class AlgorithmC {
  using L = typename N::link_type;
  using C = typename N::color_type;
  using NodePointerArray = std::vector<L>;
  using SizeType = typename NodePointerArray::size_type;

  public:
  AlgorithmC(NA& n)
    : n(n) {
    static_assert(
      sizeof(L) <= sizeof(typename NA::size_type),
      "Link_type must be smaller or equal to size_type of container.");
  }
  ~AlgorithmC() = default;

  void set_expected_solution_option_count(SizeType count) {
    xarr.resize(count);
  }

  const NodePointerArray& current_solution() const { return xarr; }

  bool compute_next_solution() {
    const L n = n.size();
    const L z = [this](){}();
  }

  protected:
  L last_spacer_node();
  
  void cover(L i) {
    L p = DLINK(i);
    while(p != i) {
      hide(p);
      p = DLINK(p);
    }
    L l = LLINK(i);
    L r = RLINK(i);
    RLINK(l) = r;
    NLINK(r) = l;
  }
  void hide(L p) {
    L q = p + 1;
    while(q != p) {
      L x = TOP(q);
      L u = ULINK(q);
      L d = DLINK(q);

      if(x <= 0) {
        q = u;// q was a spacer
      } else if(COLOR(q) < 0) {
        q = d;// q is ignored because of color (page 88)
      } else {
        DLINK(u) = d;
        ULINK(d) = u;
        LEN(x) = LEN(x) - 1;
        q = q + 1;
      }
    }
  }
  void uncover(L i) {
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
  void unhide(L p) {
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
  void commit(L p, L j) {
    if(COLOR(p) == 0)
      cover(j);
    else if(COLOR(p) > 0)
      purify(p);
  }
  void purify(L p) {
    C c = COLOR(p);
    L i = TOP(p);
    L q = ULINK(i);
    while(q != i) {
      L q = ULINK(q);
      if(COLOR(q) == c)
        COLOR(q) = -1;
      else
        hide(q);
    }
  }
  void uncommit(L p, L j) {
    if(COLOR(p) == 0)
      uncover(j);
    else if(COLOR(p) > 0)
      unpurify(p);
  }
  void unpurify(L p) {
    C c = COLOR(p);
    L i = TOP(p);
    L q = ULINK(i);
    while(q != i) {
      L q = ULINK(q);
      if(COLOR(q) < 0)
        COLOR(q) = c;
      else
        unhide(q);
    }
  }

  L& LLINK(L i) { return n[i].LLINK; }
  L& RLINK(L i) { return n[i].RLINK; }
  L& ULINK(L i) { return n[i].ULINK; }
  L& DLINK(L i) { return n[i].DLINK; }
  L& TOP(L i) { return n[i].TOP; }
  L& LEN(L i) { return n[i].LEN; }
  C& COLOR(L i) { return n[i].COLOR; }

  typename NodePointerArray::value_type& x(L i) {
    assert(i < xarr.size());
    return xarr[i];
  }

  private:
  NA& n;

  NodePointerArray xarr;
};

class WordPuzzle {
  public:
  using Node = ColoredNode<std::uint32_t, std::uint32_t>;
  using NodeVector = std::vector<Node>;

  WordPuzzle()
    : xcc(nodes) {}
  ~WordPuzzle() {}

  private:
  NodeVector nodes;
  AlgorithmC<NodeVector> xcc;
};
}

int
main() {
  using namespace dancing_links;
  WordPuzzle puzzle;

  return 0;
}
