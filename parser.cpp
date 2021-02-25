#include <fstream>
#include <iostream>

#include "parser.hpp"
#include "problem.hpp"

#include <boost/bind.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/multi_array.hpp>
#include <boost/phoenix/bind/bind_member_function.hpp>
#include <boost/regex/pending/unicode_iterator.hpp>
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
      grammar<Iterator, XX(), boost::spirit::standard_wide::space_type> {
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
    using boost::spirit::qi::fail;
    using boost::spirit::qi::lexeme;
    using boost::spirit::qi::lit;
    using boost::spirit::qi::on_error;
    using boost::spirit::standard_wide::alnum;
    using boost::spirit::standard_wide::alpha;
    using boost::spirit::standard_wide::char_;
    using boost::spirit::standard_wide::string;

    using boost::phoenix::construct;
    using boost::phoenix::val;

    using boost::spirit::qi::_a;
    using boost::spirit::qi::_r1;
    using boost::spirit::qi::labels::_1;
    using boost::spirit::qi::labels::_2;
    using boost::spirit::qi::labels::_3;
    using boost::spirit::qi::labels::_4;
    using boost::spirit::qi::labels::_val;

    identifier %= lexeme[+(char_ - char_(';') - char_(':') - char_(']') -
                           char_('>') - char_(' ') - char_('.'))];

    problemWrapper %= problem >> -(char_('.') | char_(';'));
    problem %= ('<' > primaryItemMapList(_val) > '>') >
               -('[' > secondaryItemMapList(_val) > ']') > (option % ";");
    option %= +item;
    item %= mappedCI | mappedPI;
    mappedCI %= im >> ':' > cm;
    mappedPI %= im;

    primaryItemMapList %=
      +(mappedPI[boost::phoenix::bind(&XX::addMappedPrimaryItem, _r1, _1)]);
    secondaryItemMapList %=
      *(mappedPI[boost::phoenix::bind(&XX::addMappedSecondaryItem, _r1, _1)]);

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
    primaryItemMapList.name("primary item list");
    secondaryItemMapList.name("secondary item list");

    on_error<fail>(
      problem,
      std::cerr << val("Parser error! Expecting ") << _4// what failed?
                << val(" here: \"")
                << construct<std::string>(_3, _2)// iterators to error-pos, end
                << val("\"") << std::endl);
  }

  private:
  boost::spirit::qi::
    rule<Iterator, XX(), boost::spirit::standard_wide::space_type>
      problemWrapper;
  boost::spirit::qi::rule<Iterator,
                          XX(),
                          boost::spirit::qi::locals<XX>,
                          boost::spirit::standard_wide::space_type>
    problem;
  boost::spirit::qi::
    rule<Iterator, Option(), boost::spirit::standard_wide::space_type>
      option;
  boost::spirit::qi::
    rule<Iterator, Item(), boost::spirit::standard_wide::space_type>
      item;
  boost::spirit::qi::
    rule<Iterator, IM(), boost::spirit::standard_wide::space_type>
      im;
  boost::spirit::qi::
    rule<Iterator, CM(), boost::spirit::standard_wide::space_type>
      cm;

  boost::spirit::qi::
    rule<Iterator, void(XX&), boost::spirit::standard_wide::space_type>
      primaryItemMapList;
  boost::spirit::qi::
    rule<Iterator, void(XX&), boost::spirit::standard_wide::space_type>
      secondaryItemMapList;

  boost::spirit::qi::
    rule<Iterator, MappedPI(), boost::spirit::standard_wide::space_type>
      mappedPI;
  boost::spirit::qi::
    rule<Iterator, MappedCI(), boost::spirit::standard_wide::space_type>
      mappedCI;

  boost::spirit::qi::
    rule<Iterator, std::string(), boost::spirit::standard_wide::space_type>
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
  using boost::spirit::standard_wide::space;
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

std::optional<MappedColoredExactCoveringProblemInt32String>
parse_string_mapped_int32_from_file(const std::string& filepath) {
  std::ifstream ifs(filepath);
  ifs >> std::noskipws;
  boost::spirit::istream_iterator f(ifs), l;

  return ParseMappedColoredExactCoveringProblem<int32_t,
                                                int32_t,
                                                std::string,
                                                std::string>(f, l);
}
std::optional<MappedColoredExactCoveringProblemInt32String>
parse_string_mapped_int32(const std::string& str) {
  return ParseMappedColoredExactCoveringProblem<int32_t,
                                                int32_t,
                                                std::string,
                                                std::string>(str.begin(),
                                                             str.end());
}
std::optional<MappedColoredExactCoveringProblemInt32String>
parse_string_mapped_int32(std::string_view str) {
  return ParseMappedColoredExactCoveringProblem<int32_t,
                                                int32_t,
                                                std::string,
                                                std::string>(str.begin(),
                                                             str.end());
}

ColoredExactCoverProblemSolverWrapper::ColoredExactCoverProblemSolverWrapper() {
}

ColoredExactCoverProblemSolverWrapper::
  ~ColoredExactCoverProblemSolverWrapper() {}

bool
ColoredExactCoverProblemSolverWrapper::parse(const std::string& problem) {
  m_xx = parse_string_mapped_int32(problem);
  if(m_xx) {
    m_xcc = std::make_unique<XCC>(m_xx->hna, m_xx->na);
  }
  return m_xx.has_value();
}

bool
ColoredExactCoverProblemSolverWrapper::compute_next_solution() {
  if(!m_xx || !m_xcc)
    return false;
  return m_xcc->compute_next_solution();
}

bool
ColoredExactCoverProblemSolverWrapper::has_solution() const {
  return m_xx && m_xcc && m_xcc->has_solution();
}

const typename ColoredExactCoverProblemSolverWrapper::XCC::NodePointerArray&
ColoredExactCoverProblemSolverWrapper::get_selected_options() {
  if(!has_solution())
    return m_emptySolution;
  else
    return m_xcc->current_selected_options();
}

void
ColoredExactCoverProblemSolverWrapper::print_solution() const {
  if(has_solution()) {
    m_xx->printMappedSolution(m_xcc->current_selected_option_starts(),
                              std::cout);
  }
}

std::string
ColoredExactCoverProblemSolverWrapper::get_stringified_solution() const {
  if(!has_solution()) {
    return "";
  }

  std::stringstream s;
  m_xx->printMappedSolution(m_xcc->current_selected_option_starts(), s);
  return s.str();
}
}
