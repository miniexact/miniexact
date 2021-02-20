#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <set>
#include <string>

#include "algorithm.hpp"
#include "problem.hpp"

#include <boost/multi_array.hpp>

#define DANCINGLINKS_HTML_TD_STYLE \
  "style='text-align: center; vertical-align: middle; font-family: monospace'"

namespace dancing_links {
class WordPuzzle {
  public:
  using WS = std::u32string;
  using Alphabet = std::set<char32_t>;
  using P = MappedColoredExactCoveringProblemInt32Char32;
  using Option = typename P::MappedOption;
  using PI = typename P::MappedPI;
  using CI = typename P::MappedCI;

  static int32_t getItemFromCoordWithDir(int16_t x, int16_t y, int8_t dir);

  static int32_t getItemFromCoord(int16_t x,
                                  int16_t y,
                                  bool assignedItem = false);

  static std::pair<int16_t, int16_t> getCoordFromItem(int32_t i);

  static int32_t getItemFromWord(size_t word);

  static size_t getWordFromItem(int32_t i);

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
                     size_t i,
                     size_t uniqueWord,
                     size_t width,
                     size_t height);

    void paint(P& p, Orientation o);

    void paintRect(P& p, size_t x, size_t y, Orientation o);

    template<typename Functor>
    void paintRectWrapper(P& p, size_t x, size_t y, Functor f) {
      Option option;
      option.push_back(PI{ getItemFromWord(wPI) });
      (this->*f)(option, x, y);
      p.addMappedOption(option);
    }

    void paintRectLeftToRight(Option& o, size_t x, size_t y);
    void paintRectRightToLeft(Option& o, size_t x, size_t y);
    void paintRectTopToBottom(Option& o, size_t x, size_t y);
    void paintRectBottomToTop(Option& o, size_t x, size_t y);
    void paintRectUpperLeftToLowerRight(Option& o, size_t x, size_t y);
    void paintRectLowerRightToUpperLeft(Option& o, size_t x, size_t y);
    void paintRectLowerLeftToUpperRight(Option& o, size_t x, size_t y);
    void paintRectUpperRightToLowerLeft(Option& o, size_t x, size_t y);

    const WS& w;
    const size_t wPI;
    size_t width, height;
    size_t side;
    size_t uniqueWord;
  };

  explicit WordPuzzle(size_t width, size_t height);
  ~WordPuzzle();

  bool compute_next_solution();
  bool has_solution();
  bool continue_calling();
  void step();

  void addWord(WS word);
  void addLetter(typename Alphabet::value_type l);

  size_t wordCount() const;
  size_t alphabetSize() const;

  Orientation orientation = { true, true, true, true, true, true, true, true };

  using BoardArr = boost::multi_array<std::u32string, 2>;
  BoardArr getPuzzle() const;

  template<class OutStream = decltype(std::cout)>
  void printPuzzle(OutStream& outStream = std::cout) {
    assert(has_solution());

    auto arr = getPuzzle();

    for(size_t y = 0; y < height; ++y) {
      for(size_t x = 0; x < width; ++x) {
        outStream << WStringToUtf8Str(arr[x][y]) << " ";
      }
      outStream << std::endl;

      if(y + 1 < height) {
        outStream << std::endl;
      }
    }
  }

  template<class OutStream = decltype(std::cout)>
  void printPuzzleHTML(OutStream& outStream = std::cout) {
    assert(has_solution());

    auto arr = getPuzzle();

    outStream << "<section><table>";

    for(size_t y = 0; y < height; ++y) {
      outStream << "<tr>";
      for(size_t x = 0; x < width; ++x) {
        outStream << "<td " DANCINGLINKS_HTML_TD_STYLE ">"
                  << WStringToUtf8Str(arr[x][y]) << "</td>";
      }
      outStream << "</tr>";
    }

    outStream << "</table></section>" << std::endl;
  }

  template<class OutStream = decltype(std::cout)>
  void printSolutionHTML(OutStream& outStream = std::cout) {
    assert(has_solution());

    auto arr = getPuzzle();

    size_t word = 0;
    std::set<std::pair<size_t, size_t>> positions;
    for(auto& o : xcc->current_selected_option_starts()) {
      auto name = p->getMappedItem(p->hna[p->na[o].TOP].NAME);

      // Only go over word options.
      if((name & 0b11) != 0b01)
        continue;

      positions.clear();

      outStream << "<section><h3>"
                << WStringToUtf8Str(words[getWordFromItem(name)])
                << "</h3><table>";

      for(size_t i = o + 1; p->na[i + 1].TOP >= 0; ++i) {
        auto mappedItem = p->getMappedItem(p->hna[p->na[i].TOP].NAME);
        positions.insert(getCoordFromItem(mappedItem));
      }

      for(size_t y = 0; y < height; ++y) {
        outStream << "<tr>";
        for(size_t x = 0; x < width; ++x) {
          if(positions.count(std::pair<size_t, size_t>(x, y))) {
            outStream << "<td " DANCINGLINKS_HTML_TD_STYLE
                         "><strong style='background-color: orange;'>"
                      << WStringToUtf8Str(arr[x][y]) << "</strong></td>";
          } else {
            outStream << "<td " DANCINGLINKS_HTML_TD_STYLE ">"
                      << WStringToUtf8Str(arr[x][y]) << "</td>";
          }
        }
        outStream << "</tr>";
      }

      outStream << "</table></section>" << std::endl;

      if(++word >= words.size()) {
        break;
      }
    }
  }

  template<class OutStream = decltype(std::cout)>
  void printSolution(OutStream& outStream = std::cout) {
    assert(has_solution());

    auto arr = getPuzzle();

    size_t word = 0;
    std::set<std::pair<size_t, size_t>> positions;
    for(auto& o : xcc->current_selected_option_starts()) {
      auto name = p->getMappedItem(p->hna[p->na[o].TOP].NAME);

      // Only go over word options.
      if((name & 0b11) != 0b01)
        continue;

      positions.clear();

      outStream << "  " << WStringToUtf8Str(words[getWordFromItem(name)]) << ":"
                << std::endl;

      for(size_t i = o + 1; p->na[i + 1].TOP >= 0; ++i) {
        auto mappedItem = p->getMappedItem(p->hna[p->na[i].TOP].NAME);
        positions.insert(getCoordFromItem(mappedItem));
      }

      for(size_t y = 0; y < height; ++y) {
        for(size_t x = 0; x < width; ++x) {
          if(positions.count(std::pair<size_t, size_t>(x, y))) {
            outStream << WStringToUtf8Str(arr[x][y]);
          } else {
            outStream << " ";
          }
        }
        outStream << std::endl;
      }

      if(++word >= words.size()) {
        break;
      }
    }
  }

  void printPuzzleToSTDOUT();
  void printSolutionToSTDOUT();
  void printPuzzleHTMLToSTDOUT();
  void printSolutionHTMLToSTDOUT();

  size_t getOptionCount();
  size_t getPrimaryItemCount();
  size_t getSecondaryItemCount();
  std::string getPossibleConfigurations();

  void setUseMRV(bool useMRV);

  private:
  std::unique_ptr<P> p;
  std::unique_ptr<AlgorithmC<P::HNA, P::NA>> xcc;
  Alphabet alphabet;
  std::vector<WS> words;
  std::map<WS, size_t> uniqueWords;
  size_t width, height;
  bool needsRegen = true;

  void ensureRegen();
  void regen();
};

}
