from cffi import FFI
ffibuilder = FFI()

ffibuilder.cdef("""
struct xccs;

extern "Python" void solution_iterator(struct xccs*, const char**, const char**, unsigned int, void*);

struct xccs*
xccs_init_x();

struct xccs*
xccs_init_c();

struct xccs*
xccs_init_m();

void
xccs_define_primary_item(struct xccs* h, const char* name, unsigned int u, unsigned int v);

void
xccs_define_secondary_item(struct xccs* h, const char* name);

void
xccs_add(struct xccs* h, const char* name, const char* color);

int
xccs_solve(struct xccs* h);

void
xccs_solution(struct xccs* h, void(*xccs_solution_iterator)(struct xccs*, const char**, const char**, unsigned int, void*), void* userdata);

void
xccs_free(struct xccs* h);
""")

ffibuilder.set_source('pyxcc', """#include "../include/xcc/simple.h" """,
                      libraries=['./xcc-static'],
                      extra_link_args=['-Wl,-rpath=./'],)

if __name__ == "__main__":
    ffibuilder.compile(verbose=True)
