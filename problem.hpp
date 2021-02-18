#pragma once

#include <iostream>
#include <unordered_map>
#include <variant>
#include <vector>

#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>

#include "nodes.hpp"
#include "util.hpp"

namespace dancing_links {
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

  void addOption(Option o);

  Size getOptionCount() const;
  Size getPrimaryItemCount() const;
  Size getSecondaryItemCount() const;

  void addPrimaryItem(PI i);
  void addSecondaryItem(PI i);

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

  I getItemMapping(IM n);
  C getColorMapping(CM n);

  I getItemMappingConst(IM m) const;
  C getColorMappingConst(CM m) const;

  IM getMappedItem(I i) const;
  CM getMappedColor(C c) const;

  bool addMappedOption(const MappedOption& mappedOption);

  friend std::ostream& operator<<(std::ostream& o,
                                  MappedColoredExactCoveringProblem& c) {
    if(c.hna.size() < 1 || c.na.size() < 1)
      return o << "{[],[]}";

    o << "{[" << c.hna[0];
    for(auto i = 1; i < c.hna.size(); ++i) {
      o << "," << std::endl << c.hna[i];
    }
    o << "],[" << c.na[0];
    for(auto i = 1; i < c.na.size(); ++i) {
      o << "," << std::endl << c.na[i];
    }
    return o << "]}";
  }

  bool isItemMappingKnown(IM m) { return itemMappings.left.count(m); }
  bool isColorMappingKnown(CM m) { return colorMappings.left.count(m); }

  void addMappedPrimaryItem(MappedPI& item) {
    if(isItemMappingKnown(item.item)) {
      std::cerr << "Error: Primary item \"" << WStringToUtf8Str(item.item)
                << "\" is already known! Cannot add again!" << std::endl;
    } else if(error) {
      std::cerr << "Error: Too many mappings! Cannot add primary item \""
                << WStringToUtf8Str(item.item) << "\"!" << std::endl;
    } else {
      B::addPrimaryItem(typename B::PI{ getItemMapping(item.item) });
    }
  }
  void addMappedSecondaryItem(MappedPI& item) {
    if(isItemMappingKnown(item.item)) {
      std::cerr << "Error: Secondary item \"" << WStringToUtf8Str(item.item)
                << "\" is already known! Cannot add again!" << std::endl;
    } else if(error) {
      std::cerr << "Error: Too many mappings! Cannot add secondary item \""
                << WStringToUtf8Str(item.item) << "\"!" << std::endl;
    } else {
      B::addSecondaryItem(typename B::PI{ getItemMapping(item.item) });
    }
  }

  template<class OutStream>
  void printMapped(OutStream& o) {
    o << "< ";
    for(size_t i = 1; i < B::getPrimaryItemCount() + 1; ++i) {
      o << WStringToUtf8Str(getMappedItem(B::hna[i].NAME)) << " ";
    }
    o << ">" << std::endl;

    o << "[ ";
    for(size_t i = 1 + B::getPrimaryItemCount();
        i < B::getPrimaryItemCount() + B::getSecondaryItemCount() + 1;
        ++i) {
      o << WStringToUtf8Str(getMappedItem(B::hna[i].NAME)) << " ";
    }
    o << "]" << std::endl;

    for(size_t i = B::hna.size(); i < B::na.size(); ++i) {
      const auto& n = B::na[i];
      if(n.TOP > 0) {
        o << WStringToUtf8Str(getMappedItem(B::hna[n.TOP].NAME));
        if(n.COLOR > 0) {
          o << ":" << WStringToUtf8Str(getMappedColor(n.COLOR));
        }
        o << " ";
      } else if(i + 1 < B::na.size()) {
        o << ";" << std::endl;
      } else {
        o << "." << std::endl;
      }
    }
  }

  template<class Solution, class OutStream>
  void printMappedSolution(const Solution& s, OutStream& out) const {
    for(auto& o : s) {
      out << "    ";
      for(size_t i = o; B::na[i].TOP >= 0; ++i) {
        out << getMappedItem(B::hna[B::na[i].TOP].NAME);
        if(B::na[i].COLOR > 0) {
          out << ":" << getMappedColor(B::na[i].COLOR);
        }
        out << " ";
      }
      out << ";" << std::endl;
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

using MappedColoredExactCoveringProblemInt32String =
  MappedColoredExactCoveringProblem<int32_t, int32_t, std::string, std::string>;
using MappedColoredExactCoveringProblemInt32Char32 =
  MappedColoredExactCoveringProblem<int32_t, int32_t, int32_t, char32_t>;

extern template struct ColoredExactCoveringProblem<int, int>;

// Instantiate the problem
extern template struct MappedColoredExactCoveringProblem<int32_t,
                                                         int32_t,
                                                         std::string,
                                                         std::string>;
extern template struct MappedColoredExactCoveringProblem<int32_t,
                                                         int32_t,
                                                         int32_t,
                                                         char32_t>;
}
