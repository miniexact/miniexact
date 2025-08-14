%module miniexact
%{
#define SWIG_FILE_WITH_INIT
#include <miniexact/miniexact.h>
#include <miniexact/simple.h>
#include <miniexact/simple.hpp>
%}

%include <stdint.i>
%include "std_vector.i"

%template(vectorstr) std::vector<char*>;
%template(vectori) std::vector<int32_t>;

%include <miniexact/miniexact.h>
%include <miniexact/simple.h>
%include <miniexact/simple.hpp>

%template(miniexacts_x) miniexacts_wrapper<miniexacts_init_x>;
%template(miniexacts_c) miniexacts_wrapper<miniexacts_init_c>;
%template(miniexacts_m) miniexacts_wrapper<miniexacts_init_m>;

%extend miniexacts_wrapper<miniexacts_init_x> {
    int32_t __getitem__(unsigned int i) {
        return (*($self))[i];
    }
}
%extend miniexacts_wrapper<miniexacts_init_c> {
    int32_t __getitem__(unsigned int i) {
        return (*($self))[i];
    }
}
%extend miniexacts_wrapper<miniexacts_init_m> {
    int32_t __getitem__(unsigned int i) {
        return (*($self))[i];
    }
}
