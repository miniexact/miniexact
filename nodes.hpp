#pragma once

#include <limits>
#include <ostream>

namespace dancing_links {

template<typename T, typename N>
struct HeaderNode {
  using link_type = T;
  using name_type = N;

  HeaderNode(T l, T r)
    : LLINK(l)
    , RLINK(r) {}

  HeaderNode(N n, T l, T r)
    : LLINK(l)
    , RLINK(r)
    , NAME(n) {}

  N NAME = 0;
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

}
