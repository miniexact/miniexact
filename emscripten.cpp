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
    .function("addWord", &WordPuzzle::addWord)
    .function("addLetter", &WordPuzzle::addLetter)
    .function("getPuzzle", &WordPuzzle::getPuzzle)
    .function("printPuzzle", &WordPuzzle::printPuzzleToSTDOUT)
    .function("printSolution", &WordPuzzle::printSolutionToSTDOUT)
    .function("printPuzzleHTML", &WordPuzzle::printPuzzleHTMLToSTDOUT)
    .function("printSolutionHTML", &WordPuzzle::printSolutionHTMLToSTDOUT)
  ;

  register_vector<ColoredExactCoverProblemSolverWrapper::XCC::NodePointerArray::value_type>("NodePointerArray");

  class_<ColoredExactCoverProblemSolverWrapper>("XCC")
    .constructor<>()
    .function("parse", &ColoredExactCoverProblemSolverWrapper::parse)
    .function("compute_next_solution", &ColoredExactCoverProblemSolverWrapper::compute_next_solution)
    .function("has_solution", &ColoredExactCoverProblemSolverWrapper::has_solution)
    .function("get_selected_options", &ColoredExactCoverProblemSolverWrapper::get_selected_options)
    .function("get_stringified_solution", &ColoredExactCoverProblemSolverWrapper::get_stringified_solution)
  ;
  // clang-format on
}
#endif
