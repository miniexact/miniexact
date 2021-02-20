#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "delta-debug-problem.hpp"
#include "parser.hpp"
#include "wordpuzzle.hpp"

#ifndef __EMSCRIPTEN__
#include <boost/program_options.hpp>

#include "doctest.h"

using std::cerr;
using std::clog;
using std::cout;
using std::endl;

void
conflicting_options(const boost::program_options::variables_map& vm,
                    const char* opt1,
                    const char* opt2) {
  if(vm.count(opt1) && !vm[opt1].defaulted() && vm.count(opt2) &&
     !vm[opt2].defaulted())
    throw std::logic_error(std::string("Conflicting options '") + opt1 +
                           "' and '" + opt2 + "'.");
}

int
main(int argc, const char* argv[]) {
  if(argc > 1 && strcmp(argv[1], "--test") == 0) {
    doctest::Context ctx;

    ctx.setOption("no-breaks", false);

    ctx.applyCommandLine(argc, argv);
    int res = ctx.run();
    ctx.shouldExit();
    return res;
  }

  using namespace dancing_links;
  using namespace boost::program_options;

  std::string input_wordpuzzle = "";
  std::string input_xcc = "";
  bool exhaust = false;
  bool use_mrv = false;
  bool output_html = false;
  bool output_arrays = false;

  bool dd_keep_sat = false;
  bool dd_make_sat = false;

  uint16_t wordpuzzle_width = 5;
  uint16_t wordpuzzle_height = 5;

  // clang-format off
  options_description desc("Options");
  desc.add_options()
    ("help,h", "print help message")
    ("wordpuzzle,w", value<std::string>(&input_wordpuzzle), "specify word puzzle definition to parse and generate puzzle")
    ("xcc,x", value<std::string>(&input_xcc), "specify xcc problem file to parse and solve problem")
    ("exhaust,e", bool_switch(&exhaust), "compute all solutions that are available")
    ("width", value<uint16_t>(&wordpuzzle_width), "specify width of the word puzzle")
    ("height", value<uint16_t>(&wordpuzzle_height), "specify height of the word puzzle")
    ("html", bool_switch(&output_html), "activate HTML output for word puzzle")
    ("mrv", bool_switch(&use_mrv), "use the MRV heuristic")
    ("arrays", bool_switch(&output_arrays), "output the array encoding (2D linked lists)")
    ("dd-keep-sat", bool_switch(&dd_keep_sat), "try to minify formula using delta debugging while staying SAT")
    ("dd-make-sat", bool_switch(&dd_make_sat), "try to make formula SAT using delta debugging")
  ;
  // clang-format on

  variables_map vm;
  try {
    store(parse_command_line(argc, argv, desc), vm);

    conflicting_options(vm, "xcc", "width");
    conflicting_options(vm, "xcc", "height");
    conflicting_options(vm, "wordpuzzle", "xcc");
    conflicting_options(vm, "xcc", "html");
    conflicting_options(vm, "wordpuzzle", "dd-keep-sat");
    conflicting_options(vm, "wordpuzzle", "dd-make-sat");
    conflicting_options(vm, "dd-make-sat", "dd-keep-sat");

    if(vm.count("help")) {
      cout << desc << endl;
      return EXIT_SUCCESS;
    }
    if(vm.count("wordpuzzle"))
      input_wordpuzzle = vm["wordpuzzle"].as<std::string>();
    if(vm.count("xcc"))
      input_xcc = vm["xcc"].as<std::string>();
    if(vm.count("width"))
      wordpuzzle_width = vm["width"].as<uint16_t>();
    if(vm.count("height"))
      wordpuzzle_height = vm["height"].as<uint16_t>();
    if(vm.count("exhaust"))
      exhaust = vm["exhaust"].as<bool>();
    if(vm.count("mrv"))
      use_mrv = vm["mrv"].as<bool>();
    if(vm.count("html"))
      output_html = vm["html"].as<bool>();
    if(vm.count("arrays"))
      output_arrays = vm["arrays"].as<bool>();
    if(vm.count("dd-make-sat"))
      dd_make_sat = vm["dd-make-sat"].as<bool>();
    if(vm.count("dd-keep-sat"))
      dd_keep_sat = vm["dd-keep-sat"].as<bool>();
  } catch(std::exception& e) {
    cerr << "Could not parse parameters! Error: " << e.what() << endl;
    cerr << desc << endl;
    return EXIT_FAILURE;
  }

  if(vm.count("xcc")) {
    auto problemOpt = parse_string_mapped_int32_from_file(input_xcc);
    if(!problemOpt) {
      cerr << "Could not parse file \"" << input_xcc << "\"!" << endl;
    } else {
      auto& problem = *problemOpt;

      clog << "  Parsed file. Read " << problem.getOptionCount()
           << " options with " << problem.getPrimaryItemCount()
           << " primary and " << problem.getSecondaryItemCount()
           << " secondary items. Starting solver." << endl;
      if(use_mrv) {
        clog << "  Using MRV heuristic." << endl;
      }

      if(!dd_make_sat && !dd_keep_sat) {
        AlgorithmC xcc(problem.hna, problem.na);
        xcc.setUseMRV(use_mrv);

        if(output_arrays) {
          problem.printArrays(std::clog);
        }

        int solution_counter = 0;
        bool solution_found = true;
        while(solution_found) {
          solution_found = xcc.compute_next_solution();

          if(solution_found) {
            clog << "  Solution " << ++solution_counter
                 << " found! Printing solution:" << endl;
            const auto& s = xcc.current_selected_options();
            clog << "  Selected Options: ";
            std::for_each(
              s.begin(), s.end(), [](auto& s) { clog << s << " "; });
            clog << endl;
            clog << "  Stringified Selected Options: " << endl;
            problem.printMappedSolution(xcc.current_selected_option_starts(),
                                        std::cout);
          } else if(solution_counter == 0) {
            clog << "  No Solution Found!" << endl;
          }

          if(!exhaust) {
            solution_found = false;
          }
        }
        if(exhaust) {
          clog << "Found " << solution_counter << " solutions." << endl;
        }
      } else {
        DeltaDebugProblem dd(problem);
        if(dd_make_sat) {
          clog << "Try to make SAT using delta debugger..." << endl;
          auto resOpt = dd.make_sat_by_removing_options();
          if(resOpt) {
            clog << "Result: ";
            resOpt->printMapped(std::cout);
          } else {
            clog << "No result!" << endl;
          }
        }

        if(dd_keep_sat) {
          clog << "Try to minify while staying SAT using delta debugger..."
               << endl;
          auto resOpt = dd.keep_sat_while_removing_options();
          if(resOpt) {
            clog << "Result: ";
            resOpt->printMapped(std::cout);
          } else {
            clog << "No result!" << endl;
          }
        }
      }
    }
  } else if(vm.count("wordpuzzle")) {
    WordPuzzle wordPuzzle(wordpuzzle_width, wordpuzzle_height);

    std::string line;

    std::ifstream wordListFile(input_wordpuzzle);
    if(!wordListFile) {
      cerr << "Cannot open word list file \"" << input_wordpuzzle << "\"!"
           << endl;
      return EXIT_FAILURE;
    }
    while(std::getline(wordListFile, line)) {
      auto u32str = Utf8StringToUTF32String(line);
      wordPuzzle.addWord(u32str);
    }

    clog << "Parsed word list with " << wordPuzzle.wordCount() << " words and "
         << wordPuzzle.alphabetSize() << " letters in the alphabet on a "
         << wordpuzzle_width << "x" << wordpuzzle_height
         << " puzzle field. This computes to " << wordPuzzle.getOptionCount()
         << " options with " << wordPuzzle.getPrimaryItemCount()
         << " primary items and " << wordPuzzle.getSecondaryItemCount()
         << " secondary items, describing and compressing "
         << wordPuzzle.getPossibleConfigurations()
         << " possible puzzle configurations." << endl;
    clog << "Start to search for possible puzzles..." << endl;

    wordPuzzle.setUseMRV(use_mrv);

    if(use_mrv) {
      clog << "Using MRV heuristic." << endl;
    }

    int solution_counter = 0;
    bool solution_found = true;
    while(solution_found) {
      solution_found = wordPuzzle.compute_next_solution();
      if(solution_found) {
        clog << "Possibility " << ++solution_counter << " found:" << endl;
        if(output_html)
          wordPuzzle.printPuzzleHTML(std::cout);
        else
          wordPuzzle.printPuzzle(std::cout);

        clog << "Solution:" << endl;
        if(output_html)
          wordPuzzle.printSolutionHTML(std::cout);
        else
          wordPuzzle.printSolution(std::cout);
      } else if(solution_counter == 0) {
        clog << "No way to align letters to fit all " << wordPuzzle.wordCount()
             << " words into a " << wordpuzzle_width << "x" << wordpuzzle_height
             << " field!";
      }
      if(!exhaust)
        solution_found = false;
    }
    if(exhaust) {
      clog << "Found " << solution_counter << " solutions!" << endl;
    }
  } else {
    cerr << "No operation specified! Please specify what you want to do!"
         << endl;
    cerr << desc << endl;
    return EXIT_FAILURE;
  }

  return 0;
}

#else

int
main(int argc, char* argv[]) {
  return EXIT_SUCCESS;
}

#endif
