#include <algorithm>
#include <boost/proto/detail/remove_typename.hpp>
#include <cassert>
#include <codecvt>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
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

using std::cerr;
using std::clog;
using std::cout;
using std::endl;

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

template<typename I>
struct PrimaryItem {
  I item;
};

template<typename I, typename C>
struct ColoredItem {
  I item;
  C color;
};

// Roughly following
// https://stackoverflow.com/a/31302660
template<typename T>
std::string
WStringToUtf8Str(T m) {
  if constexpr(std::is_same_v<T, std::string>) {
    return m;
  } else if constexpr(std::is_same_v<T, std::u16string>) {
    std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> conv;
    return conv.to_bytes(m);
  } else if constexpr(std::is_same_v<T, std::u32string>) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.to_bytes(m);
  }
}

std::u32string
Utf8StringToUTF32String(const std::string& s) {
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
  return conv.from_bytes(s);
}

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
    if(optionCount == 0) {
      na.push_back(N(0, 0, 0));
      lastSpacer = na.size() - 1;

      hna.push_back(HN(hna.size() - 1, hna.size() - secondaryItemCount));
      hna[hna.size() - 2].RLINK = hna.size() - 1;
      hna[hna.size() - secondaryItemCount - 1].LLINK = hna.size() - 1;
    }
    optionCount++;

    Size beginningOfOption = na.size();
    Size lastDLINK = 0;

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

      Link& l = links[item];

      N& t = na[l.top];
      ++t.LEN;
      t.ULINK = na.size();

      if(t.LEN == 1) {
        l.up = l.top;
      }

      na[l.up].DLINK = na.size();
      na.push_back(N(l.top, l.up, l.top, color));
      l.up = na.size() - 1;
    }
    na[lastSpacer].DLINK = na.size() - 1;
    na.push_back(N(-optionCount, beginningOfOption, 0, 0));
    lastSpacer = na.size() - 1;

    // Options are added sequentially to the node array. Finalize at the end
    // links up everything correctly.
  }

  Size getOptionCount() const { return optionCount; }
  Size getPrimaryItemCount() const {
    // Two spacer nodes. The second spacer node is only there if options have
    // been added.
    return hna.size() - secondaryItemCount - !(optionCount == 0) - 1;
  }
  Size getSecondaryItemCount() const { return secondaryItemCount; }

  void addPrimaryItem(PI i) {
    assert(secondaryItemCount == 0);
    hna.push_back(HN(i.item, hna.size() - 1, 0));
    hna[hna.size() - 2].RLINK = hna.size() - 1;
    hna[0].LLINK = hna.size() - 1;

    na.push_back(N(0, 0, 0));
    links[i.item].top = na.size() - 1;
  }
  void addSecondaryItem(PI i) {
    hna.push_back(HN(i.item, hna.size() - 1, hna.size() - secondaryItemCount));
    if(secondaryItemCount > 0) {
      hna[hna.size() - 2].RLINK = hna.size() - 1;
      hna[hna.size() - secondaryItemCount - 1].LLINK = hna.size() - 1;
    }

    na.push_back(N(0, 0, 0));
    links[i.item].top = na.size() - 1;

    ++secondaryItemCount;
  }

  HNA hna = { HN(0, 0, 0) };
  NA na = { N(0, 0, 0) };

  private:
  struct Link {
    Size top = 0;
    Size up = 0;
  };

  std::unordered_map<I, Link> links;
  Size secondaryItemCount = 0;
  Size optionCount = 0;
  Size lastSpacer = 0;
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
    if constexpr(std::is_same_v<I, IM>) {
      return n;
    }

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
    if constexpr(std::is_same_v<C, CM>) {
      return n;
    }

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
    if constexpr(std::is_same_v<I, IM>) {
      return m;
    }

    auto it = itemMappings.left.find(m);
    if(it == itemMappings.left.end())
      return 0;
    return it->second;
  }
  C getColorMappingConst(CM m) const {
    if constexpr(std::is_same_v<C, CM>()) {
      return m;
    }

    auto it = colorMappings.left.find(m);
    if(it == colorMappings.left.end())
      return 0;
    return it->second;
  }

  IM getMappedItem(I i) const {
    if constexpr(std::is_same_v<I, IM>) {
      return i;
    }

    auto it = itemMappings.right.find(i);
    assert(it != itemMappings.right.end());
    return it->second;
  }
  CM getMappedColor(C c) const {
    auto it = colorMappings.right.find(c);
    assert(it != colorMappings.right.end());
    return it->second;
  }

  bool addMappedOption(const MappedOption& mappedOption) {
    typename B::Option option(mappedOption.size());
    for(typename MappedOption::size_type i = 0; i < mappedOption.size(); ++i) {
      const auto& v = mappedOption[i];
      if(std::holds_alternative<MappedPI>(v)) {
        const auto& mappedPI = std::get<MappedPI>(v);
        auto mi = getItemMappingConst(mappedPI.item);
        if(mi == 0) {
          cerr << "Item \"" << WStringToUtf8Str(mappedPI.item)
               << "\" not specified as primary or secondary item!" << endl;
          return false;
        }
        option[i] = typename B::PI{ mi };
      } else if(std::holds_alternative<MappedCI>(v)) {
        const auto& mappedCI = std::get<MappedCI>(v);
        auto mi = getItemMappingConst(mappedCI.item);
        if(mi == 0) {
          cerr << "Item \"" << WStringToUtf8Str(mappedCI.item)
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
      cerr << "Error: Primary item \"" << WStringToUtf8Str(item.item)
           << "\" is already known! Cannot add again!" << endl;
    } else if(error) {
      cerr << "Error: Too many mappings! Cannot add primary item \""
           << WStringToUtf8Str(item.item) << "\"!" << endl;
    } else {
      B::addPrimaryItem(typename B::PI{ getItemMapping(item.item) });
    }
  }
  void addMappedSecondaryItem(MappedPI& item) {
    if(isItemMappingKnown(item.item)) {
      cerr << "Error: Secondary item \"" << WStringToUtf8Str(item.item)
           << "\" is already known! Cannot add again!" << endl;
    } else if(error) {
      cerr << "Error: Too many mappings! Cannot add secondary item \""
           << WStringToUtf8Str(item.item) << "\"!" << endl;
    } else {
      B::addSecondaryItem(typename B::PI{ getItemMapping(item.item) });
    }
  }

  private:
  using ItemMapping =
    boost::bimap<boost::bimaps::set_of<IM>, boost::bimaps::set_of<I>>;
  using ColorMapping =
    boost::bimap<boost::bimaps::set_of<CM>, boost::bimaps::set_of<C>>;
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

struct printer {
  typedef boost::spirit::utf8_string string;

  void element(string const& tag, string const& value, int depth) const {
    for(int i = 0; i < (depth * 4); ++i)// indent to depth
      std::cout << ' ';

    std::cout << "tag: " << tag;
    if(value != "")
      std::cout << ", value: " << value;
    std::cout << std::endl;
  }
};

void
print_info(boost::spirit::info const& what) {
  using boost::spirit::basic_info_walker;

  printer pr;
  basic_info_walker<printer> walker(pr, what.tag, 0);
  boost::apply_visitor(walker, what.value);
}

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

  bool r;
  try {
    r = boost::spirit::qi::phrase_parse(iter, end, grammar, space, problem);
  } catch(boost::spirit::qi::expectation_failure<Begin> const& x) {
    std::cout << "expected: ";
    print_info(x.what_);
    std::cout << "got: \"" << std::string(x.first, x.last) << '"' << std::endl;
  }
  if(r && iter == end) {
    return problem;
  } else {
    cerr << "Did not parse whole string!" << endl;
    return std::nullopt;
  }
}

auto
parse_string_mapped_int32_from_file(const std::string& filepath) {
  std::ifstream ifs(filepath);
  ifs >> std::noskipws;
  boost::spirit::istream_iterator f(ifs), l;

  return ParseMappedColoredExactCoveringProblem<int32_t,
                                                int32_t,
                                                std::string,
                                                std::string>(f, l);
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
  const NodePointerArray& current_selected_option_starts() const {
    assert(has_solution());
    return selected_option_starts;
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

  HNA& hn;
  NA& n;

  NodePointerArray xarr;
  NodePointerArray selected_options;
  NodePointerArray selected_option_starts;
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
                                                   N(4, 21, 9),
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

  auto staticallyDefinedVectorPair = produce_vectors_for_example_49();
  auto& shna = staticallyDefinedVectorPair.first;
  auto& sna = staticallyDefinedVectorPair.second;

  for(size_t i = 0; i < shna.size(); ++i) {
    CAPTURE(i);

    REQUIRE(problem.hna[i].LLINK == shna[i].LLINK);
    REQUIRE(problem.hna[i].RLINK == shna[i].RLINK);
  }
  for(size_t i = 0; i < sna.size(); ++i) {
    CAPTURE(i);

    REQUIRE(problem.na[i].TOP == sna[i].TOP);
    REQUIRE(problem.na[i].ULINK == sna[i].ULINK);
    REQUIRE(problem.na[i].DLINK == sna[i].DLINK);
  }
}

#endif

class WordPuzzle {
  public:
  using WS = std::u32string;
  using Alphabet = std::set<char32_t>;
  using P = MappedColoredExactCoveringProblem<int32_t, char32_t, WS, char32_t>;
  using Option = typename P::MappedOption;
  using PI = typename P::MappedPI;
  using CI = typename P::MappedCI;

  static size_t numDigits(int32_t x) {
    if(x == std::numeric_limits<int32_t>::min())
      return 10 + 1;
    if(x < 0)
      return numDigits(-x) + 1;

    if(x >= 10000) {
      if(x >= 10000000) {
        if(x >= 100000000) {
          if(x >= 1000000000)
            return 10;
          return 9;
        }
        return 8;
      }
      if(x >= 100000) {
        if(x >= 1000000)
          return 7;
        return 6;
      }
      return 5;
    }
    if(x >= 100) {
      if(x >= 1000)
        return 4;
      return 3;
    }
    if(x >= 10)
      return 2;
    return 1;
  }

  static WS getItemFromCoord(size_t w, size_t h, size_t x, size_t y) {
    std::stringstream ss;
    ss << std::setw(numDigits(w)) << std::setfill('0') << x;
    ss << "-";
    ss << std::setw(numDigits(h)) << std::setfill('0') << y;
    return Utf8StringToUTF32String(ss.str());
  }

  struct Orientation {
    bool left_to_right : 1;
    bool right_to_left : 1;
    bool top_to_bottom : 1;
    bool bottom_to_top : 1;
    bool upper_left_to_lower_right : 1;
    bool lower_right_to_upper_left : 1;
    bool lower_left_to_upper_right : 1;
    bool upper_right_to_lower_left : 1;
  };

  struct Painter {
    explicit Painter(const WS& w,
                     const Alphabet& a,
                     size_t width,
                     size_t height)
      : w(w)
      , width(width)
      , height(height)
      , side(w.length()) {}

    void paint(P& p, Alphabet& alphabet, Orientation o) {
      for(size_t y = 0; y < height; ++y) {
        for(size_t x = 0; x < width; ++x) {
          paintRect(p, x, y, o);

          for(auto& l : alphabet) {
            Option option;
            option.push_back(CI{ getItemFromCoord(width, height, x, y), l });
            p.addMappedOption(option);
          }
        }
      }
    }

    void paintRect(P& p, size_t x, size_t y, Orientation o) {
      if(o.left_to_right && x + side < width)
        paintRectWrapper(p, x, y, &Painter::paintRectLeftToRight);
      if(o.right_to_left && x + side < width)
        paintRectWrapper(p, x, y, &Painter::paintRectRightToLeft);
      if(o.top_to_bottom && y + side < height)
        paintRectWrapper(p, x, y, &Painter::paintRectTopToBottom);
      if(o.bottom_to_top && y + side < height)
        paintRectWrapper(p, x, y, &Painter::paintRectBottomToTop);
      if(o.upper_left_to_lower_right && x + side < width && y + side < height)
        paintRectWrapper(p, x, y, &Painter::paintRectUpperLeftToLowerRight);
      if(o.lower_right_to_upper_left && x + side < width && y + side < height)
        paintRectWrapper(p, x, y, &Painter::paintRectLowerRightToUpperLeft);
      if(o.lower_left_to_upper_right && x + side < width && y + side < height)
        paintRectWrapper(p, x, y, &Painter::paintRectLowerLeftToUpperRight);
      if(o.upper_right_to_lower_left && x + side < width && y + side < height)
        paintRectWrapper(p, x, y, &Painter::paintRectUpperLeftToLowerRight);
    }

    template<typename Functor>
    void paintRectWrapper(P& p, size_t x, size_t y, Functor f) {
      Option option;
      option.push_back(PI{ w });
      (this->*f)(option, p, x, y);
      p.addMappedOption(option);
    }

    void paintRectLeftToRight(Option& o, P& p, size_t x, size_t y) {
      assert(x + side < width);
      for(size_t i = 0; i < side; ++i)
        o.push_back(CI{ getItemFromCoord(width, height, x + i, y), w[i] });
    }
    void paintRectRightToLeft(Option& o, P& p, size_t x, size_t y) {
      assert(x + side < width);
      for(size_t i = 0; i < side; ++i)
        o.push_back(
          CI{ getItemFromCoord(width, height, x + side - i, y), w[i] });
    }
    void paintRectTopToBottom(Option& o, P& p, size_t x, size_t y) {
      assert(y + side < height);
      for(size_t i = 0; i < side; ++i)
        o.push_back(CI{ getItemFromCoord(width, height, x, y + i), w[i] });
    }
    void paintRectBottomToTop(Option& o, P& p, size_t x, size_t y) {
      assert(y + side < height);
      for(size_t i = 0; i < side; ++i)
        o.push_back(
          CI{ getItemFromCoord(width, height, x, y + side - i), w[i] });
    }
    void paintRectUpperLeftToLowerRight(Option& o, P& p, size_t x, size_t y) {
      assert(x + side < width);
      assert(y + side < height);
      for(size_t i = 0; i < side; ++i)
        o.push_back(CI{ getItemFromCoord(width, height, x + i, y + i), w[i] });
    }
    void paintRectLowerRightToUpperLeft(Option& o, P& p, size_t x, size_t y) {
      assert(x + side < width);
      assert(y + side < height);
      for(size_t i = 0; i < side; ++i)
        o.push_back(CI{
          getItemFromCoord(width, height, x + side - i, y + side - i), w[i] });
    }
    void paintRectLowerLeftToUpperRight(Option& o, P& p, size_t x, size_t y) {
      assert(x + side < width);
      assert(y + side < height);
      for(size_t i = 0; i < side; ++i)
        o.push_back(
          CI{ getItemFromCoord(width, height, x + i, y + side - i), w[i] });
    }
    void paintRectUpperRightToLowerLeft(Option& o, P& p, size_t x, size_t y) {
      assert(x + side < width);
      assert(y + side < height);
      for(size_t i = 0; i < side; ++i)
        o.push_back(
          CI{ getItemFromCoord(width, height, x + side - i, y + i), w[i] });
    }

    const WS& w;
    size_t width, height;
    size_t side;
  };

  explicit WordPuzzle(size_t width, size_t height)
    : width(width)
    , height(height) {}
  ~WordPuzzle() {}

  bool compute_next_solution() {
    ensureRegen();
    return xcc->compute_next_solution();
  }
  bool has_solution() {
    if(!xcc)
      return false;
    return xcc->has_solution();
  }
  bool continue_calling() {
    if(!xcc)
      return false;
    return xcc->continue_calling();
  }
  void step() {
    assert(xcc);
    xcc->step();
  }

  void addWord(WS word) {
    for(char32_t c : word) {
      if(!alphabet.count(c)) {
        alphabet.insert(c);
      }
    }
    if(words.insert(word).second) {
      needsRegen = true;
    }
  }
  void addLetter(typename Alphabet::value_type l) {
    if(alphabet.insert(l).second) {
      needsRegen = true;
    }
  }

  auto wordCount() const { return words.size(); }
  auto alphabetSize() const { return alphabet.size(); }

  Orientation orientation;

  private:
  std::unique_ptr<P> p;
  std::unique_ptr<AlgorithmC<P::HNA, P::NA>> xcc;
  Alphabet alphabet;
  std::set<WS> words;
  size_t width, height;
  bool needsRegen = true;

  void ensureRegen() {
    if(needsRegen)
      regen();
  }
  void regen() {
    p = std::make_unique<P>();
    xcc = std::make_unique<AlgorithmC<P::HNA, P::NA>>(p->hna, p->na);

    for(const auto& word : words) {
      auto pi = PI{ word };
      p->addMappedPrimaryItem(pi);
    }

    for(size_t y = 0; y < height; ++y) {
      for(size_t x = 0; x < width; ++x) {
        auto pi = PI{ getItemFromCoord(width, height, x, y) };
        p->addMappedSecondaryItem(pi);
      }
    }

    for(const auto& word : words) {
      Painter painter(word, alphabet, width, height);
      painter.paint(*p, alphabet, orientation);
    }
  }
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
    .constructor<size_t, size_t>()
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

  if(argc > 1 && strcmp(argv[1], "--parse-and-solve") == 0) {
    if(argc <= 2) {
      cerr << "Require file to read!" << endl;
      return EXIT_FAILURE;
    }

    auto problemOpt = parse_string_mapped_int32_from_file(argv[2]);
    if(!problemOpt) {
      cerr << "Could not parse file!" << endl;
    } else {

      auto& problem = *problemOpt;

      clog << "  Parsed file. Read " << problem.getOptionCount()
           << " options with " << problem.getPrimaryItemCount()
           << " primary and " << problem.getSecondaryItemCount()
           << " secondary items. Starting solver." << endl;

      AlgorithmC xcc(problem.hna, problem.na);
      bool solution_found = xcc.compute_next_solution();

      if(solution_found) {
        clog << "  Solution Found! Printing solution:" << endl;
        const auto& s = xcc.current_selected_options();
        clog << "  Selected Options: ";
        std::for_each(s.begin(), s.end(), [](auto& s) { clog << s << " "; });
        clog << endl;
        clog << "  Stringified Selected Options: " << endl;
        for(auto& o : xcc.current_selected_option_starts()) {
          clog << "    ";
          for(size_t i = o; problem.na[i].TOP >= 0; ++i) {
            clog << problem.getMappedItem(problem.na[i].TOP);
            if(problem.na[i].COLOR > 0) {
              clog << ":" << problem.getMappedColor(problem.na[i].COLOR);
            }
            clog << " ";
          }
          clog << ";" << endl;
        }
      } else {
        clog << "  No Solution Found!" << endl;
      }
    }
  }

  if(argc > 1 && strcmp(argv[1], "--wordpuzzle") == 0) {
    if(argc != 5 && argc != 6) {
      cerr << "Wordpuzzle requires width, height, a filename containing the "
              "word list, and optionally a file containing the alphabet!"
           << endl;
      cerr << "Alphabet files are line by line single character alphabets "
              "(unicode aware). File lists are line by line unicode words."
           << endl;
      cerr << "Call like " << argv[0]
           << " <width> <height> <word-list> [alphabet-file]" << endl;
      return EXIT_FAILURE;
    }

    int width = atoi(argv[2]);
    int height = atoi(argv[3]);

    if(width <= 0 || height <= 0) {
      cerr << "Width and height must both be > 0!" << endl;
      return EXIT_FAILURE;
    }

    WordPuzzle wordPuzzle(width, height);

    std::string line;

    std::string wordlistFilePath = argv[4];
    if(argc == 6) {
      std::ifstream alphabetFile(argv[5]);
      if(!alphabetFile) {
        cerr << "Cannot open alphabet file \"" << argv[5] << "\"!" << endl;
        return EXIT_FAILURE;
      }

      while(std::getline(alphabetFile, line)) {
        auto u32str = Utf8StringToUTF32String(line);
        wordPuzzle.addLetter(u32str[0]);
      }
    }

    std::ifstream wordListFile(argv[4]);
    if(!wordListFile) {
      cerr << "Cannot open word list file \"" << argv[4] << "\"!" << endl;
      return EXIT_FAILURE;
    }
    while(std::getline(wordListFile, line)) {
      auto u32str = Utf8StringToUTF32String(line);
      wordPuzzle.addWord(u32str);
    }

    clog << "Parsed word list with " << wordPuzzle.wordCount() << " words and "
         << wordPuzzle.alphabetSize() << " letters in the alphabet." << endl;
    clog << "Start to search for possible puzzles..." << endl;

    bool solution_fount = wordPuzzle.compute_next_solution();

    if(solution_fount) {
      clog << "Possibility found:" << endl;
    } else {
      clog << "No way to align letters to fit all " << wordPuzzle.wordCount()
           << " words into a " << width << "x" << height << " field!";
    }
  }

  return 0;
}
