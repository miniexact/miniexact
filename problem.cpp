#include "problem.hpp"
#include <iostream>

using std::cerr;
using std::clog;
using std::cout;
using std::endl;

namespace dancing_links {

template<typename I, typename C>
ColoredExactCoveringProblem<I, C>
ColoredExactCoveringProblem<I, C>::copyItems(
  const ColoredExactCoveringProblem<I, C>& o) {
  return ColoredExactCoveringProblem<I, C>{
    HNA(o.hna.begin(), o.hna.begin() + o.hna.size() - 1),
    o.optionCount == 0 ? NA(o.na.begin(), o.na.begin() + o.hna.size() - 1)
                       : o.na_originalHeader,
    o.originalLinks,
    o.secondaryItemCount
  };
}

template<typename I, typename C>
typename ColoredExactCoveringProblem<I, C>::Size
ColoredExactCoveringProblem<I, C>::getOptionCount() const {
  return optionCount;
}
template<typename I, typename C>
typename ColoredExactCoveringProblem<I, C>::Size
ColoredExactCoveringProblem<I, C>::getPrimaryItemCount() const {
  // Two spacer nodes. The second spacer node is only there if options have
  // been added.
  return hna.size() - secondaryItemCount - !(optionCount == 0) - 1;
}
template<typename I, typename C>
typename ColoredExactCoveringProblem<I, C>::Size
ColoredExactCoveringProblem<I, C>::getSecondaryItemCount() const {
  return secondaryItemCount;
}

template<typename I, typename C>
void
ColoredExactCoveringProblem<I, C>::addPrimaryItem(PI i) {
  assert(secondaryItemCount == 0);
  assert(optionCount == 0);
  hna.emplace_back(i.item, hna.size() - 1, 0);
  hna[hna.size() - 2].RLINK = hna.size() - 1;
  hna[0].LLINK = hna.size() - 1;

  na.emplace_back(0, 0, 0);
  links[i.item].top = na.size() - 1;
}

template<typename I, typename C>
void
ColoredExactCoveringProblem<I, C>::addSecondaryItem(PI i) {
  assert(optionCount == 0);
  hna.emplace_back(i.item, hna.size() - 1, hna.size() - secondaryItemCount);
  if(secondaryItemCount > 0) {
    hna[hna.size() - 2].RLINK = hna.size() - 1;
    hna[hna.size() - secondaryItemCount - 1].LLINK = hna.size() - 1;
  }

  na.emplace_back(0, 0, 0);
  links[i.item].top = na.size() - 1;

  ++secondaryItemCount;
}

template<typename I, typename C, typename IM, typename CM>
MappedColoredExactCoveringProblem<I, C, IM, CM>
MappedColoredExactCoveringProblem<I, C, IM, CM>::copyItemsMapped(
  const MappedColoredExactCoveringProblem<I, C, IM, CM>& o) {
  return MappedColoredExactCoveringProblem<I, C, IM, CM>{
    typename B::HNA(o.hna.begin(), o.hna.begin() + o.hna.size() - 1),
    o.optionCount == 0
      ? typename B::NA(o.na.begin(), o.na.begin() + o.hna.size() - 1)
      : o.na_originalHeader,
    o.originalLinks,
    o.secondaryItemCount,
    o.itemMappings,
    o.colorMappings
  };
}

template<typename I, typename C, typename IM, typename CM>
I
MappedColoredExactCoveringProblem<I, C, IM, CM>::getItemMapping(IM n) {
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
template<typename I, typename C, typename IM, typename CM>
C
MappedColoredExactCoveringProblem<I, C, IM, CM>::getColorMapping(CM n) {
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
template<typename I, typename C, typename IM, typename CM>
I
MappedColoredExactCoveringProblem<I, C, IM, CM>::getItemMappingConst(
  IM m) const {
  if constexpr(std::is_same_v<I, IM>) {
    return m;
  }

  auto it = itemMappings.left.find(m);
  if(it == itemMappings.left.end())
    return 0;
  return it->second;
}
template<typename I, typename C, typename IM, typename CM>
C
MappedColoredExactCoveringProblem<I, C, IM, CM>::getColorMappingConst(
  CM m) const {
  if constexpr(std::is_same_v<C, CM>) {
    return m;
  }

  auto it = colorMappings.left.find(m);
  if(it == colorMappings.left.end())
    return 0;
  return it->second;
}

template<typename I, typename C, typename IM, typename CM>
IM
MappedColoredExactCoveringProblem<I, C, IM, CM>::getMappedItem(I i) const {
  if constexpr(std::is_same_v<I, IM>) {
    return i;
  }

  auto it = itemMappings.right.find(i);
  assert(it != itemMappings.right.end());
  return it->second;
}

template<typename I, typename C, typename IM, typename CM>
CM
MappedColoredExactCoveringProblem<I, C, IM, CM>::getMappedColor(C c) const {
  if constexpr(std::is_same_v<C, CM>) {
    return c;
  }

  auto it = colorMappings.right.find(c);
  assert(it != colorMappings.right.end());
  return it->second;
}

template<typename I, typename C, typename IM, typename CM>
bool
MappedColoredExactCoveringProblem<I, C, IM, CM>::addMappedOption(
  const MappedOption& mappedOption) {
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

template struct ColoredExactCoveringProblem<int, int>;

// Instantiate the problem
template struct MappedColoredExactCoveringProblem<int32_t,
                                                  int32_t,
                                                  std::string,
                                                  std::string>;
template struct MappedColoredExactCoveringProblem<int32_t,
                                                  int32_t,
                                                  int32_t,
                                                  char32_t>;
}
