#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

#ifdef DEBUG
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"
#endif

#include <boost/bimap.hpp>
#include <boost/bimap/list_of.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bind.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/phoenix/bind/bind_member_function.hpp>
#include <boost/spirit/home/qi/auto/create_parser.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/qi.hpp>

namespace dancing_links {
using std::cerr;
using std::clog;
using std::cout;
using std::endl;

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

template<typename I>
struct PrimaryItem {
  I item;
};

template<typename I, typename C>
struct ColoredItem {
  I item;
  C color;
};

template<typename I, typename C>
struct ColoredExactCoveringProblem {
  using PI = PrimaryItem<I>;
  using CI = ColoredItem<I, C>;
  using Item = std::variant<PI, CI>;
  using Option = std::vector<Item>;
  using Options = std::vector<Option>;

  using HN = HeaderNode<I, I>;
  using N = ColoredNode<I, C>;
  using HNA = std::vector<HN>;
  using NA = std::vector<N>;

  using Size = typename Option::size_type;

  void addOption(Option o) {
    if(firstOption) {
      na.push_back(N(0, 0, 0));

      hna.push_back(HN(hna.size() - 1, hna.size() - secondaryItemCount));
      hna[hna.size() - 2].RLINK = hna.size() - 1;
      hna[hna.size() - secondaryItemCount - 1].LLINK = hna.size() - 1;

      firstOption = false;
    }

    for(const auto& i : o) {
      I item = 0;
      C color = 0;

      if(std::holds_alternative<PI>(i)) {
        const auto& pi = std::get<PI>(i);
        item = pi.item;
      }
      if(std::holds_alternative<CI>(i)) {
        const auto& ci = std::get<CI>(i);
        item = ci.item;
        color = ci.color;
      }
    }

    // Options are added sequentially to the node array. Finalize at the end
    // links up everything correctly.
  }

  Size getPrimaryItemCount() const {
    // Two spacer nodes. The second spacer node is only there if options have
    // been added.
    return hna.size() - secondaryItemCount - !firstOption - 1;
  }
  Size getSecondaryItemCount() const { return secondaryItemCount; }

  void addPrimaryItem(PI i) {
    assert(secondaryItemCount == 0);
    hna.push_back(HN(i.item, hna.size() - 1, 0));
    hna[hna.size() - 2].RLINK = hna.size() - 1;
    hna[0].LLINK = hna.size() - 1;

    na.push_back(N(0, 0, 0));
    tops[i.item] = na.size() - 1;
  }
  void addSecondaryItem(PI i) {
    hna.push_back(HN(i.item, hna.size() - 1, hna.size() - secondaryItemCount));
    if(secondaryItemCount > 0) {
      hna[hna.size() - 2].RLINK = hna.size() - 1;
      hna[hna.size() - secondaryItemCount - 1].LLINK = hna.size() - 1;
    }

    na.push_back(N(0, 0, 0));
    tops[i.item] = na.size() - 1;

    ++secondaryItemCount;
  }

  HNA hna = { HN(0, 0, 0) };
  NA na = { N(0, 0, 0) };

  private:
  std::unordered_map<I, Size> tops;
  Size secondaryItemCount = 0;
  bool firstOption = true;
};

template<typename I, typename C, typename IM, typename CM>
class MappedColoredExactCoveringProblem
  : public ColoredExactCoveringProblem<I, C> {
  public:
  using B = ColoredExactCoveringProblem<I, C>;
  using MappedPI = PrimaryItem<IM>;
  using MappedCI = ColoredItem<IM, CM>;
  using MappedItem = std::variant<MappedPI, MappedCI>;
  using MappedOption = std::vector<MappedItem>;
  using MappedPrimaryItems = std::vector<PrimaryItem<IM>>;

  I getItemMapping(IM n) {
    I i;
    auto it = itemMappings.left.find(n);
    if(it == itemMappings.left.end()) {
      if(itemMappings.size() == std::numeric_limits<I>::max()) {
        error = true;
        return 0;
      }
      i = itemMappings.size() + 1;
      itemMappings.insert(ItemMappingValue(n, i));
    } else {
      i = it->second;
    }
    return i;
  }
  C getColorMapping(CM n) {
    C c;
    auto it = colorMappings.left.find(n);
    if(it == colorMappings.left.end()) {
      if(colorMappings.size() == std::numeric_limits<C>::max()) {
        error = true;
        return 0;
      }
      c = colorMappings.size() + 1;
      colorMappings.insert(ColorMappingValue(n, c));
    } else {
      c = it->second;
    }
    return c;
  }
  I getItemMappingConst(IM m) const {
    auto it = itemMappings.left.find(m);
    if(it == itemMappings.left.end())
      return 0;
    return it->second;
  }
  C getColorMappingConst(CM m) const {
    auto it = colorMappings.left.find(m);
    if(it == colorMappings.left.end())
      return 0;
    return it->second;
  }

  IM getMappedItem(I i) const {
    auto it = itemMappings.right.find(i);
    assert(it != itemMappings.right.end());
    return it.second;
  }
  CM getMappedColor(C c) const {
    auto it = colorMappings.right.find(c);
    assert(it != colorMappings.right.end());
    return it.second;
  }

  bool addMappedOption(const MappedOption& mappedOption) {
    typename B::Option option(mappedOption.size());
    for(typename MappedOption::size_type i = 0; i < mappedOption.size(); ++i) {
      const auto& v = mappedOption[i];
      if(std::holds_alternative<MappedPI>(v)) {
        const auto& mappedPI = std::get<MappedPI>(v);
        auto mi = getItemMappingConst(mappedPI.item);
        if(mi == 0) {
          cerr << "Item \"" << mappedPI.item
               << "\" not specified as primary or secondary item!" << endl;
          return false;
        }
        option[i] = typename B::PI{ mi };
      } else if(std::holds_alternative<MappedCI>(v)) {
        const auto& mappedCI = std::get<MappedCI>(v);
        auto mi = getItemMappingConst(mappedCI.item);
        if(mi == 0) {
          cerr << "Item \"" << mappedCI.item
               << "\" not specified as primary or secondary item!" << endl;
          return false;
        }
        option[i] = typename B::CI{ mi, getColorMapping(mappedCI.color) };
      } else {
        assert(false);
        return false;
      }
    }
    if(error) {
      error = false;
      return false;
    }
    B::addOption(option);
    return true;
  }

  friend std::ostream& operator<<(std::ostream& o,
                                  MappedColoredExactCoveringProblem& c) {
    if(c.hna.size() < 1 || c.na.size() < 1)
      return o << "{[],[]}";

    o << "{[" << c.hna[0];
    for(auto i = 1; i < c.hna.size(); ++i) {
      o << "," << endl << c.hna[i];
    }
    o << "],[" << c.na[0];
    for(auto i = 1; i < c.na.size(); ++i) {
      o << "," << endl << c.na[i];
    }
    return o << "]}";
  }

  bool isItemMappingKnown(IM m) { return itemMappings.left.count(m); }
  bool isColorMappingKnown(CM m) { return colorMappings.left.count(m); }

  void addMappedPrimaryItem(MappedPI& item) {
    if(isItemMappingKnown(item.item)) {
      cerr << "Error: Primary item \"" << item.item
           << "\" is already known! Cannot add again!" << endl;
    } else if(error) {
      cerr << "Error: Too many mappings! Cannot add primary item \""
           << item.item << "\"!" << endl;
    } else {
      B::addPrimaryItem(typename B::PI{ getItemMapping(item.item) });
    }
  }
  void addMappedSecondaryItem(MappedPI& item) {
    if(isItemMappingKnown(item.item)) {
      cerr << "Error: Secondary item \"" << item.item
           << "\" is already known! Cannot add again!" << endl;
    } else if(error) {
      cerr << "Error: Too many mappings! Cannot add secondary item \""
           << item.item << "\"!" << endl;
    } else {
      B::addSecondaryItem(typename B::PI{ getItemMapping(item.item) });
    }
  }

  private:
  using ItemMapping =
    boost::bimap<boost::bimaps::set_of<IM>, boost::bimaps::list_of<I>>;
  using ColorMapping =
    boost::bimap<boost::bimaps::set_of<CM>, boost::bimaps::list_of<C>>;
  using ItemMappingValue = typename ItemMapping::value_type;
  using ColorMappingValue = typename ColorMapping::value_type;
  ItemMapping itemMappings;
  ColorMapping colorMappings;

  bool error = false;
};
}

BOOST_FUSION_ADAPT_TPL_STRUCT((I), (dancing_links::PrimaryItem)(I), item)
BOOST_FUSION_ADAPT_TPL_STRUCT((I)(C),
                              (dancing_links::ColoredItem)(I)(C),
                              item,
                              color)

// Custom list target
// https://stackoverflow.com/questions/17042851/boost-spirit-parse-integer-to-custom-list-template
namespace boost::spirit::traits {
template<typename I, typename C, typename IM, typename CM>
struct container_value<
  dancing_links::MappedColoredExactCoveringProblem<I, C, IM, CM>,
  void> {
  typedef typename dancing_links::
    MappedColoredExactCoveringProblem<I, C, IM, CM>::MappedOption type;
};

template<typename I, typename C, typename IM, typename CM>
struct push_back_container<
  dancing_links::MappedColoredExactCoveringProblem<I, C, IM, CM>,
  typename dancing_links::MappedColoredExactCoveringProblem<I, C, IM, CM>::
    MappedOption,
  void> {
  static bool call(
    dancing_links::MappedColoredExactCoveringProblem<I, C, IM, CM>& x,
    typename dancing_links::MappedColoredExactCoveringProblem<I, C, IM, CM>::
      MappedOption o) {
    return x.addMappedOption(o);
  }
};
}

namespace dancing_links {
// Parser following examples from Boost Spirit documentation
// https://www.boost.org/doc/libs/1_47_0/libs/spirit/example/qi/mini_xml1.cpp
// https://www.boost.org/doc/libs/1_47_0/libs/spirit/doc/html/spirit/qi/tutorials/mini_xml___asts_.html

template<typename Iterator,
         typename I,
         typename C,
         typename IM,
         typename CM,
         typename XX = MappedColoredExactCoveringProblem<I, C, IM, CM>>
class MappedColoredExactCoveringProblemParser
  : public boost::spirit::qi::
      grammar<Iterator, XX(), boost::spirit::ascii::space_type> {
  public:
  using Option = typename XX::MappedOption;
  using Item = typename XX::MappedItem;
  using MappedPI = typename XX::MappedPI;
  using MappedCI = typename XX::MappedCI;

  // https://stackoverflow.com/a/57812868
  template<typename T>
  struct is_string
    : public std::disjunction<
        std::is_same<char*, typename std::decay<T>::type>,
        std::is_same<const char*, typename std::decay<T>::type>,
        std::is_same<std::string, typename std::decay<T>::type>> {};

  MappedColoredExactCoveringProblemParser()
    : MappedColoredExactCoveringProblemParser::base_type(problemWrapper,
                                                         "problem") {
    using boost::spirit::ascii::alnum;
    using boost::spirit::ascii::alpha;
    using boost::spirit::ascii::char_;
    using boost::spirit::qi::fail;
    using boost::spirit::qi::lexeme;
    using boost::spirit::qi::lit;
    using boost::spirit::qi::on_error;

    using boost::phoenix::construct;
    using boost::phoenix::val;

    using boost::spirit::qi::_a;
    using boost::spirit::qi::_r1;
    using boost::spirit::qi::labels::_1;
    using boost::spirit::qi::labels::_2;
    using boost::spirit::qi::labels::_3;
    using boost::spirit::qi::labels::_4;
    using boost::spirit::qi::labels::_val;

    identifier %= lexeme[alpha >> *(alnum | char_('_'))];

    problemWrapper %= problem > '.';
    problem %= ('<' > primaryItemMapList(_val) > '>' > '[' >
                secondaryItemMapList(_val) > ']' > (option % ";"));
    option %= +item;
    item %= mappedCI | mappedPI;
    mappedCI %= im >> ':' > cm;
    mappedPI %= im;

    primaryItemMapList %=
      +(mappedPI[boost::phoenix::bind(&XX::addMappedPrimaryItem, _r1, _1)]);
    secondaryItemMapList %=
      +(mappedPI[boost::phoenix::bind(&XX::addMappedSecondaryItem, _r1, _1)]);

    // Only if mapping is to string, identifier is used. If some other type is
    // used, the parser should use that type.
    if constexpr(is_string<IM>::value) {
      im %= identifier;
    } else {
      im = boost::spirit::qi::create_parser<IM>();
    }
    if constexpr(is_string<CM>::value) {
      cm %= identifier;
    } else {
      cm = boost::spirit::qi::create_parser<CM>();
    }

    problemWrapper.name("problem");
    problem.name("problem");
    option.name("option");
    item.name("item");
    im.name("primary item");
    cm.name("colored secondary item");
    identifier.name("identifier");

    on_error<fail>(
      problem,
      std::cerr << val("Parser error! Expecting ") << _4// what failed?
                << val(" here: \"")
                << construct<std::string>(_3, _2)// iterators to error-pos, end
                << val("\"") << std::endl);
  }

  private:
  boost::spirit::qi::rule<Iterator, XX(), boost::spirit::ascii::space_type>
    problemWrapper;
  boost::spirit::qi::rule<Iterator,
                          XX(),
                          boost::spirit::qi::locals<XX>,
                          boost::spirit::ascii::space_type>
    problem;
  boost::spirit::qi::rule<Iterator, Option(), boost::spirit::ascii::space_type>
    option;
  boost::spirit::qi::rule<Iterator, Item(), boost::spirit::ascii::space_type>
    item;
  boost::spirit::qi::rule<Iterator, IM(), boost::spirit::ascii::space_type> im;
  boost::spirit::qi::rule<Iterator, CM(), boost::spirit::ascii::space_type> cm;

  boost::spirit::qi::rule<Iterator, void(XX&), boost::spirit::ascii::space_type>
    primaryItemMapList;
  boost::spirit::qi::rule<Iterator, void(XX&), boost::spirit::ascii::space_type>
    secondaryItemMapList;

  boost::spirit::qi::
    rule<Iterator, MappedPI(), boost::spirit::ascii::space_type>
      mappedPI;
  boost::spirit::qi::
    rule<Iterator, MappedCI(), boost::spirit::ascii::space_type>
      mappedCI;

  boost::spirit::qi::
    rule<Iterator, std::string(), boost::spirit::ascii::space_type>
      identifier;
};

template<typename I,
         typename C,
         typename IM,
         typename CM,
         typename Begin,
         typename End,
         typename XX = MappedColoredExactCoveringProblem<I, C, IM, CM>>
std::optional<XX>
ParseMappedColoredExactCoveringProblem(Begin begin, End end) {
  using boost::spirit::ascii::space;
  MappedColoredExactCoveringProblemParser<Begin, I, C, IM, CM> grammar;
  XX problem;
  Begin iter = begin;

  bool r = boost::spirit::qi::phrase_parse(iter, end, grammar, space, problem);
  if(r && iter == end) {
    return problem;
  } else {
    Begin some = iter + 30;
    std::string context(iter, (some > end) ? end : some);
    cerr << "Did not parse whole string! Stopped at position " << iter - begin
         << ". Context: " << context << endl;
    return std::nullopt;
  }
}

auto
parse_string_mapped_int32(const std::string& str) {
  return ParseMappedColoredExactCoveringProblem<int32_t,
                                                int32_t,
                                                std::string,
                                                std::string>(str.begin(),
                                                             str.end());
}
auto
parse_string_mapped_int32(std::string_view str) {
  return ParseMappedColoredExactCoveringProblem<int32_t,
                                                int32_t,
                                                std::string,
                                                std::string>(str.begin(),
                                                             str.end());
}

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

  const NodePointerArray& current_selected_options() const {
    assert(has_solution());
    return selected_options;
  }

  bool compute_next_solution() {
    StepResult res;
    do {
      res = step();
    } while(res == CallAgain);

    return res == ResultAvailable;
  }

  bool has_solution() const { return last_result == ResultAvailable; }
  bool continue_calling() const {
    return last_result == NoResultAvailable || last_result == CallAgain;
  }

  VIRT StepResult step() {
    last_result = stepExec();
    if(last_result == ResultAvailable) {
      update_selected_options();
    }
    return last_result;
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

  StepResult last_result = CallAgain;

  StepResult stepExec() {
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
        q = q - 1;
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
    for(L q = DLINK(i); q != i; q = DLINK(q)) {
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
    for(L q = DLINK(i); q != i; q = DLINK(q)) {
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
  L TOP(L i) const {
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
  typename NodePointerArray::value_type x(L i) const {
    assert(i < xarr.size());
    return xarr[i];
  }

  void update_selected_options() {
    selected_options.resize(l);
    for(L j = 0; j < l; ++j) {
      L r = x(j);
      while(TOP(r) >= 0) {
        ++r;
      }
      selected_options[j] = -TOP(r);
    }
  }

  HNA& hn;
  NA& n;

  NodePointerArray xarr;
  NodePointerArray selected_options;
};

using HNode = HeaderNode<std::int32_t, char>;
using Node = ColoredNode<std::int32_t, char>;
using HNodeVector = std::vector<HNode>;
using NodeVector = std::vector<Node>;

#define IGN 0

#ifdef DEBUG
template<class HNA, class NA, class OutStream = std::ostream>
class ShoutingAlgorithmC : public AlgorithmC<HNA, NA> {
  public:
  using Base = AlgorithmC<HNA, NA>;

  ShoutingAlgorithmC(HNA& hn, NA& n, OutStream& o = cout)
    : Base(hn, n)
    , o(o) {}

  using L = typename Base::L;

  VIRT ~ShoutingAlgorithmC() = default;

  VIRT typename Base::StepResult step() {
    o << Base::AlgorithmStateToStr(Base::state) << " -> " << endl;
    auto res = Base::step();
    o << " -> " << Base::AlgorithmStateToStr(Base::state) << " Result: " << res
      << endl;

    return res;
  }

  VIRT void cover(L i) {
    o << Base::AlgorithmStateToStr(Base::state) << ": cover " << i << endl;
    Base::cover(i);
  }
  VIRT void hide(L p) {
    o << Base::AlgorithmStateToStr(Base::state) << ": hide " << p << endl;
    Base::hide(p);
  }
  VIRT void uncover(L i) {
    o << Base::AlgorithmStateToStr(Base::state) << ": uncover " << i << endl;
    Base::uncover(i);
  }
  VIRT void unhide(L p) {
    o << Base::AlgorithmStateToStr(Base::state) << ": unhide " << p << endl;
    Base::unhide(p);
  }
  VIRT void commit(L p, L j) {
    o << Base::AlgorithmStateToStr(Base::state) << ": commit " << p << ", " << j
      << endl;
    Base::commit(p, j);
  }
  VIRT void purify(L p) {
    o << Base::AlgorithmStateToStr(Base::state) << ": purify " << p << endl;
    Base::purify(p);
  }
  VIRT void uncommit(L p, L j) {
    o << Base::AlgorithmStateToStr(Base::state) << ": uncommit " << p << ", "
      << j << endl;
    Base::uncommit(p, j);
  }
  VIRT void unpurify(L p) {
    o << Base::AlgorithmStateToStr(Base::state) << ": unpurify " << p << endl;
    Base::unpurify(p);
  }

  private:
  OutStream& o;
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

  std::stringstream outStream;

  ShoutingAlgorithmC<HNodeVector, NodeVector> xcc(hnvec, nvec, outStream);

  // Printing solutions during testing:
  // while(bool solution_available = xcc.compute_next_solution()) {
  //  cout << "Solution: " << endl;
  //  std::for_each(
  //    s.begin(), s.end(), [&nvec](auto n) { cout << nvec[n].TOP << endl; });
  //}

  CAPTURE(outStream.str());

  bool solution_available = xcc.compute_next_solution();

  REQUIRE(solution_available);

  const auto& s = xcc.current_selected_options();
  REQUIRE(s.size() == 2);
  REQUIRE(s[0] == 2);
  REQUIRE(s[1] == 4);

  solution_available = xcc.compute_next_solution();

  REQUIRE(!solution_available);
}

TEST_CASE("Parse example problem from page 87") {
  std::string_view problemStr =
    "<p q r> [x y] p q x y:A; p r x:A y; p x:B; q x:A; r y:B .";
  auto problemOpt = parse_string_mapped_int32(problemStr);

  REQUIRE(problemOpt);

  auto problem = problemOpt.value();

  REQUIRE(problem.getPrimaryItemCount() == 3);
  REQUIRE(problem.getSecondaryItemCount() == 2);

  cout << problem;
}

#endif

class WordPuzzle {
  public:
  using C = AlgorithmC<HNodeVector, NodeVector>;

  WordPuzzle()
    : xcc(hnodes, nodes) {}
  ~WordPuzzle() {}

  bool compute_next_solution() { return xcc.compute_next_solution(); }
  bool has_solution() { return xcc.has_solution(); }
  bool continue_calling() { return xcc.continue_calling(); }
  void step() { xcc.step(); }

  const HNodeVector getHNodes() { return hnodes; }
  const NodeVector& getNodes() { return nodes; }

  private:
  HNodeVector hnodes;
  NodeVector nodes;
  C xcc;
};
}

#ifdef __EMSCRIPTEN__

#include <emscripten/bind.h>

using namespace emscripten;
using namespace dancing_links;

// Following tutorial from
// https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html
EMSCRIPTEN_BINDINGS(dancinglinks) {
  // clang-format off
  class_<WordPuzzle>("WordPuzzle")
    .constructor<>()
    .function("compute_next_solution", &WordPuzzle::compute_next_solution)
    .function("has_solution", &WordPuzzle::has_solution)
    .function("continue_calling", &WordPuzzle::continue_calling)
    .function("step", &WordPuzzle::step)
  ;
  // clang-format on
}
#endif

int
main(int argc, const char* argv[]) {
  if(argc > 1 && strcmp(argv[1], "--test") == 0) {
#ifdef DEBUG
    doctest::Context ctx;

    ctx.setOption("no-breaks", false);

    ctx.applyCommandLine(argc, argv);
    int res = ctx.run();
    ctx.shouldExit();
    return res;
#else
    cout << "!! DEBUG undefined! No doctest and testing code included in "
            "binary. Recompile with -DDEBUG to enable testing functionality."
         << endl;
#endif
  }

  using namespace dancing_links;
  WordPuzzle puzzle;

  return 0;
}
