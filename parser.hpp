#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "algorithm.hpp"
#include "problem.hpp"

namespace dancing_links {
std::optional<MappedColoredExactCoveringProblemInt32String>
parse_string_mapped_int32_from_file(const std::string& filepath);

std::optional<MappedColoredExactCoveringProblemInt32String>
parse_string_mapped_int32(const std::string& str);

std::optional<MappedColoredExactCoveringProblemInt32String>
parse_string_mapped_int32(std::string_view str);

class ColoredExactCoverProblemSolverWrapper {
  public:
  using XX = MappedColoredExactCoveringProblem<int32_t,
                                               int32_t,
                                               std::string,
                                               std::string>;
  using XCC = AlgorithmC<XX::B::HNA, XX::B::NA>;

  ColoredExactCoverProblemSolverWrapper();
  ~ColoredExactCoverProblemSolverWrapper();

  bool parse(const std::string& problem);

  bool compute_next_solution();

  bool has_solution() const;

  const XCC::NodePointerArray& get_selected_options();

  void print_solution() const;

  std::string get_stringified_solution() const;

  private:
  std::optional<XX> m_xx;
  std::unique_ptr<XCC> m_xcc;

  XCC::NodePointerArray m_emptySolution;
};
}
