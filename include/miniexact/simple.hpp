#include <functional>
#include <iostream>
#include <memory>
#include <vector>

#include "simple.h"

typedef miniexacts* (*miniexacts_init)();

template<miniexacts_init init>
class miniexacts_wrapper {
  private:
  std::unique_ptr<miniexacts, void (*)(miniexacts*)> h_;
  std::vector<int32_t> selected_options_;
  bool solution_valid_ = false;
  int last_res_ = 0;

  void extract_solution() {
    if(solution_valid_)
      return;
    if(last_res_ != 10) {
      fprintf(stderr, "%s\n", "not in state with last result 10");
      exit(1);
    }
    selected_options_.resize(miniexacts_solution_length(h_.get()));
    int32_t len =
      miniexacts_extract_solution(h_.get(), selected_options_.data());
    selected_options_.resize(len);
    solution_valid_ = true;
  }

  public:
  miniexacts_wrapper()
    : h_(init(), &miniexacts_free) {}

  ~miniexacts_wrapper() {}

  int32_t primary(const char* name, unsigned int u = 1, unsigned int v = 1) {
    return miniexacts_define_primary_item_with_slack(h_.get(), name, u, v);
  }
  int32_t secondary(const char* name) {
    return miniexacts_define_secondary_item(h_.get(), name);
  }
  int32_t color(const char* name) {
    return miniexacts_define_color(h_.get(), name);
  }

  int32_t add(const char* name, const char* color = NULL, int32_t cost = 0) {
    return miniexacts_add_named(h_.get(), name, color, cost);
  }
  int32_t add(int32_t item, int32_t color = 0, int32_t cost = 0) {
    return miniexacts_add(h_.get(), item, color, cost);
  }

  int32_t add(std::vector<const char*> items) {
    for(const auto i : items) {
      add(i);
    }
    return add(0);
  }
  int32_t add(std::vector<int32_t> items) {
    for(const auto i : items) {
      add(i);
    }
    return add(0);
  }

  int solve() {
    solution_valid_ = false;
    int res = miniexacts_solve(h_.get());
    last_res_ = res;
    return res;
  }
  bool solution(miniexacts_solution_iterator it, void* userdata) {
    extract_solution();
    return miniexacts_solution(h_.get(), it, userdata);
  }

  using solution_cb_func =
    std::function<void(const char**, const char**, unsigned int)>;

  void solution(solution_cb_func cb) {
    extract_solution();
    miniexacts_solution(
      h_.get(),
      [](miniexacts* h,
         const char** names,
         const char** colors,
         unsigned int items_count,
         void* userdata) {
        solution_cb_func* cb = reinterpret_cast<solution_cb_func*>(userdata);
        (*cb)(names, colors, items_count);
      },
      &cb);
  }

  const std::vector<int32_t>& selected_options() {
    extract_solution();
    return selected_options_;
  }
  int32_t operator[](unsigned int i) {
    extract_solution();
    if(i >= selected_options_.size()) {
      fprintf(stderr, "%s\n", "!! Solution index out of range");
      return 0;
    }
    return selected_options_[i];
  }
  unsigned int size() {
    extract_solution();
    return selected_options_.size();
  }

  bool write_to_dlx(const char* path) {
    return miniexacts_write_to_dlx(h_.get(), path);
  }

  bool has_solution() const { return last_res_ == 10; }

  void print_solution() {
    solution([](const char** items, const char** colors, unsigned int count) {
      for(unsigned int i = 0; i < count; ++i) {
        std::cout << items[i];
        if(colors[i])
          std::cout << ":" << colors[i];
        std::cout << " ";
      }
      std::cout << "\n";
    });
    std::cout << std::endl;
  }

  miniexact_problem* problem() { return miniexacts_problem(h_.get()); }
};

using miniexacts_x = miniexacts_wrapper<&miniexacts_init_x>;
using miniexacts_c = miniexacts_wrapper<&miniexacts_init_c>;
using miniexacts_m = miniexacts_wrapper<&miniexacts_init_m>;
