#include "wordpuzzle.hpp"

using std::cerr;
using std::clog;
using std::cout;
using std::endl;

namespace dancing_links {

int32_t
WordPuzzle::getItemFromCoordWithDir(int16_t x, int16_t y, int8_t dir) {
  assert(x >= 0);
  assert(y >= 0);
  assert(dir >= 0 && dir < 8);

  int32_t i = (x + 1) & 0b1111111111111;
  i <<= 13u;
  i |= (y + 1) & 0b1111111111111;
  i <<= 4u;
  i |= dir & 0b1111;
  i <<= 2u;

  assert(i != 0);
  assert(i > 0);
  assert((i & 0b11) == 0);

  return i;
}

int32_t
WordPuzzle::getItemFromCoord(int16_t x, int16_t y, bool assignedItem) {
  assert(x >= 0);
  assert(y >= 0);

  int32_t i = x;
  i <<= 16u;
  i |= y;

  i <<= 2u;

  if(assignedItem) {
    i |= 0b10;
  } else {
    i |= 0b11;
  }

  assert(i > 0);

  return i;
}

std::pair<int16_t, int16_t>
WordPuzzle::getCoordFromItem(int32_t i) {
  assert(i > 0);
  // Remove flags;
  i >>= 2u;

  int16_t x, y;
  y = i;
  i >>= 16u;
  x = i;

  return { x, y };
}

int32_t
WordPuzzle::getItemFromWord(size_t word) {
  int32_t i = word;
  i <<= 2u;
  i |= 0b01;
  assert(i > 0);
  return i;
}

size_t
WordPuzzle::getWordFromItem(int32_t i) {
  assert(i & 0b01);
  i >>= 2u;
  return i;
}

WordPuzzle::WordPuzzle(size_t width, size_t height)
  : width(width)
  , height(height) {}

WordPuzzle::~WordPuzzle() {}

bool
WordPuzzle::compute_next_solution() {
  ensureRegen();
  clog << "Computing next solution for word puzzle..." << endl;
  return xcc->compute_next_solution();
}

bool
WordPuzzle::has_solution() {
  if(!xcc)
    return false;
  return xcc->has_solution();
}

bool
WordPuzzle::continue_calling() {
  if(!xcc)
    return false;
  return xcc->continue_calling();
}

void
WordPuzzle::step() {
  assert(xcc);
  xcc->step();
}

void
WordPuzzle::addWord(WordPuzzle::WS word) {
  if(word.length() > width && word.length() > height) {
    cerr << "Word \"" << WStringToUtf8Str(word) << "\" is too long!" << endl;
    return;
  }

  for(char32_t c : word) {
    if(!alphabet.count(c)) {
      alphabet.insert(c);
    }
  }
  words.push_back(word);

  if(!uniqueWords.count(word)) {
    uniqueWords.insert({ word, words.size() });
  }

  needsRegen = true;
}

void
WordPuzzle::addLetter(typename Alphabet::value_type l) {
  if(alphabet.insert(l).second) {
    needsRegen = true;
  }
}

size_t
WordPuzzle::wordCount() const {
  return words.size();
}

size_t
WordPuzzle::alphabetSize() const {
  return alphabet.size();
}

WordPuzzle::BoardArr
WordPuzzle::getPuzzle() const {
  BoardArr arr(boost::extents[width][height]);

  for(auto& o : xcc->current_selected_option_starts()) {
    for(size_t i = o; p->na[i].TOP >= 0; ++i) {
      auto mappedItem = p->getMappedItem(p->hna[p->na[i].TOP].NAME);
      if(p->na[i].COLOR > 0 && (mappedItem & 0b11) != 0) {
        auto mappedColor = p->getMappedColor(p->na[i].COLOR);
        auto [x, y] = getCoordFromItem(mappedItem);
        arr[x][y] += mappedColor;
      }
    }
  }
  return arr;
}

void
WordPuzzle::printPuzzleToSTDOUT() {
  printPuzzle(std::cout);
}

void
WordPuzzle::printSolutionToSTDOUT() {
  printSolution(std::cout);
}

void
WordPuzzle::printPuzzleHTMLToSTDOUT() {
  printPuzzleHTML(std::cout);
}

void
WordPuzzle::printSolutionHTMLToSTDOUT() {
  printSolutionHTML(std::cout);
}

size_t
WordPuzzle::getOptionCount() {
  ensureRegen();
  return p->getOptionCount();
}

size_t
WordPuzzle::getPrimaryItemCount() {
  ensureRegen();
  return p->getPrimaryItemCount();
}

size_t
WordPuzzle::getSecondaryItemCount() {
  ensureRegen();
  return p->getSecondaryItemCount();
}

std::string
WordPuzzle::getPossibleConfigurations() {
  std::ostringstream out;
  out.precision(0);
  out << alphabetSize() << "^" << width * height << " = ";
  out.imbue(std::locale(""));
  out << std::fixed
      << std::pow(static_cast<double>(alphabetSize()),
                  static_cast<double>(width * height));
  return out.str();
}

void
WordPuzzle::setUseMRV(bool useMRV) {
  ensureRegen();
  xcc->setUseMRV(useMRV);
}

void
WordPuzzle::ensureRegen() {
  if(needsRegen)
    regen();
}

void
WordPuzzle::regen() {
  p = std::make_unique<P>();

  clog << "Translating word puzzle to color-controlled exact covering problem."
       << endl;

  for(size_t i = 0; i < words.size(); ++i) {
    auto pi = PI{ getItemFromWord(i) };
    p->addMappedPrimaryItem(pi);
  }

  for(size_t y = 0; y < height; ++y) {
    for(size_t x = 0; x < width; ++x) {
      auto pi = PI{ getItemFromCoord(x, y, true) };
      p->addMappedPrimaryItem(pi);
    }
  }

  for(size_t y = 0; y < height; ++y) {
    for(size_t x = 0; x < width; ++x) {
      auto pi = PI{ getItemFromCoord(x, y) };
      p->addMappedSecondaryItem(pi);

      for(size_t i = 0; i < 8; ++i) {
        auto pi = PI{ getItemFromCoordWithDir(x, y, i) };
        p->addMappedSecondaryItem(pi);
      }
    }
  }

  for(size_t i = 0; i < words.size(); ++i) {
    Painter painter(words[i], i, uniqueWords[words[i]], width, height);
    painter.paint(*p, orientation);
  }

  for(size_t y = 0; y < height; ++y) {
    for(size_t x = 0; x < width; ++x) {
      auto pi = PI{ getItemFromCoord(x, y, true) };
      for(auto& l : alphabet) {
        Option option;
        option.push_back(pi);
        option.push_back(CI{ getItemFromCoord(x, y), l });
        p->addMappedOption(std::move(option));
      }
    }
  }

  needsRegen = false;

  xcc = std::make_unique<AlgorithmC<P::HNA, P::NA>>(p->hna, p->na);
  clog << "Finished translating." << endl;
}

WordPuzzle::Painter::Painter(const WordPuzzle::WS& w,
                             size_t i,
                             size_t uniqueWord,
                             size_t width,
                             size_t height)
  : w(w)
  , wPI(i)
  , width(width)
  , height(height)
  , side(w.length())
  , uniqueWord(uniqueWord) {
  assert(uniqueWord != 0);
}

void
WordPuzzle::Painter::paint(WordPuzzle::P& p, WordPuzzle::Orientation o) {
  for(size_t y = 0; y < height; ++y) {
    for(size_t x = 0; x < width; ++x) {
      paintRect(p, x, y, o);
    }
  }
}

void
WordPuzzle::Painter::paintRect(WordPuzzle::P& p,
                               size_t x,
                               size_t y,
                               WordPuzzle::Orientation o) {
  if(o.left_to_right && x + side <= width && w.length() <= width)
    paintRectWrapper(p, x, y, &Painter::paintRectLeftToRight);
  if(o.right_to_left && x + side <= width && w.length() <= width)
    paintRectWrapper(p, x, y, &Painter::paintRectRightToLeft);
  if(o.top_to_bottom && y + side <= height && w.length() <= height)
    paintRectWrapper(p, x, y, &Painter::paintRectTopToBottom);
  if(o.bottom_to_top && y + side <= height && w.length() <= height)
    paintRectWrapper(p, x, y, &Painter::paintRectBottomToTop);
  if(o.upper_left_to_lower_right && x + side <= width && y + side <= height &&
     w.length() <= width && w.length() <= height)
    paintRectWrapper(p, x, y, &Painter::paintRectUpperLeftToLowerRight);
  if(o.lower_right_to_upper_left && x + side <= width && y + side <= height &&
     w.length() <= width && w.length() <= height)
    paintRectWrapper(p, x, y, &Painter::paintRectLowerRightToUpperLeft);
  if(o.lower_left_to_upper_right && x + side <= width && y + side <= height &&
     w.length() <= width && w.length() <= height)
    paintRectWrapper(p, x, y, &Painter::paintRectLowerLeftToUpperRight);
  if(o.upper_right_to_lower_left && x + side <= width && y + side <= height &&
     w.length() <= width && w.length() <= height)
    paintRectWrapper(p, x, y, &Painter::paintRectUpperRightToLowerLeft);
}

void
WordPuzzle::Painter::paintRectLeftToRight(WordPuzzle::Option& o,
                                          size_t x,
                                          size_t y) {
  assert(x + side <= width);
  for(size_t i = 0; i < side; ++i)
    o.push_back(CI{ getItemFromCoord(x + i, y), w[i] });
  o.push_back(
    CI{ getItemFromCoordWithDir(x, y, 0), static_cast<char32_t>(wPI) });
}

void
WordPuzzle::Painter::paintRectRightToLeft(WordPuzzle::Option& o,
                                          size_t x,
                                          size_t y) {
  assert(x + side <= width);
  for(size_t i = 0; i < side; ++i)
    o.push_back(CI{ getItemFromCoord(x + side - i - 1, y), w[i] });
  o.push_back(
    CI{ getItemFromCoordWithDir(x, y, 1), static_cast<char32_t>(wPI) });
}

void
WordPuzzle::Painter::paintRectTopToBottom(WordPuzzle::Option& o,
                                          size_t x,
                                          size_t y) {
  assert(y + side <= height);
  for(size_t i = 0; i < side; ++i)
    o.push_back(CI{ getItemFromCoord(x, y + i), w[i] });
  o.push_back(
    CI{ getItemFromCoordWithDir(x, y, 2), static_cast<char32_t>(wPI) });
}

void
WordPuzzle::Painter::paintRectBottomToTop(WordPuzzle::Option& o,
                                          size_t x,
                                          size_t y) {
  assert(y + side <= height);
  for(size_t i = 0; i < side; ++i)
    o.push_back(CI{ getItemFromCoord(x, y + side - i - 1), w[i] });
  o.push_back(
    CI{ getItemFromCoordWithDir(x, y, 3), static_cast<char32_t>(wPI) });
}

void
WordPuzzle::Painter::paintRectUpperLeftToLowerRight(WordPuzzle::Option& o,
                                                    size_t x,
                                                    size_t y) {
  assert(x + side <= width);
  assert(y + side <= height);
  for(size_t i = 0; i < side; ++i)
    o.push_back(CI{ getItemFromCoord(x + i, y + i), w[i] });
  o.push_back(
    CI{ getItemFromCoordWithDir(x, y, 4), static_cast<char32_t>(wPI) });
}

void
WordPuzzle::Painter::paintRectLowerRightToUpperLeft(WordPuzzle::Option& o,
                                                    size_t x,
                                                    size_t y) {
  assert(x + side <= width);
  assert(y + side <= height);
  for(size_t i = 0; i < side; ++i)
    o.push_back(
      CI{ getItemFromCoord(x + side - i - 1, y + side - i - 1), w[i] });
  o.push_back(
    CI{ getItemFromCoordWithDir(x, y, 5), static_cast<char32_t>(wPI) });
}

void
WordPuzzle::Painter::paintRectLowerLeftToUpperRight(WordPuzzle::Option& o,
                                                    size_t x,
                                                    size_t y) {
  assert(x + side <= width);
  assert(y + side <= height);
  for(size_t i = 0; i < side; ++i)
    o.push_back(CI{ getItemFromCoord(x + i, y + side - i - 1), w[i] });
  o.push_back(
    CI{ getItemFromCoordWithDir(x, y, 6), static_cast<char32_t>(wPI) });
}

void
WordPuzzle::Painter::paintRectUpperRightToLowerLeft(WordPuzzle::Option& o,
                                                    size_t x,
                                                    size_t y) {
  assert(x + side <= width);
  assert(y + side <= height);
  for(size_t i = 0; i < side; ++i)
    o.push_back(CI{ getItemFromCoord(x + side - i - 1, y + i), w[i] });
  o.push_back(
    CI{ getItemFromCoordWithDir(x, y, 7), static_cast<char32_t>(wPI) });
}

}
