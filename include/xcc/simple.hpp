#include <functional>
#include <iostream>
#include <memory>
#include <vector>

#include "simple.h"

typedef xccs* (*xccs_init)();

template<xccs_init init>
class xccs_wrapper {
  private:
  std::unique_ptr<xccs, void (*)(xccs*)> h_;
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
    selected_options_.resize(xccs_solution_length(h_.get()));
    int32_t len = xccs_extract_solution(h_.get(), selected_options_.data());
    selected_options_.resize(len);
    solution_valid_ = true;
  }

  public:
  xccs_wrapper()
    : h_(init(), &xccs_free) {}

  ~xccs_wrapper() {}

  int32_t primary(const char* name, unsigned int u = 1, unsigned int v = 1) {
    return xccs_define_primary_item_with_slack(h_.get(), name, u, v);
  }
  int32_t secondary(const char* name) {
    return xccs_define_secondary_item(h_.get(), name);
  }
  int32_t color(const char* name) { return xccs_define_color(h_.get(), name); }

  int32_t add(const char* name, const char* color = NULL) {
    return xccs_add_named(h_.get(), name, color);
  }
  int32_t add(int32_t item, int32_t color = 0) {
    return xccs_add(h_.get(), item, color);
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
    int res = xccs_solve(h_.get());
    last_res_ = res;
    return res;
  }
  void solution(xccs_solution_iterator it, void* userdata) {
    extract_solution();
    return xccs_solution(h_.get(), it, userdata);
  }

  using solution_cb_func =
    std::function<void(const char**, const char**, unsigned int)>;

  void solution(solution_cb_func cb) {
    extract_solution();
    xccs_solution(
      h_.get(),
      [](xccs* h,
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
      fprintf(stderr, "%s\n", "Solution index out of range");
      exit(1);
    }
    return selected_options_[i];
  }
  unsigned int size() {
    extract_solution();
    return selected_options_.size();
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
};

using xccs_x = xccs_wrapper<&xccs_init_x>;
using xccs_c = xccs_wrapper<&xccs_init_c>;
using xccs_m = xccs_wrapper<&xccs_init_m>;
