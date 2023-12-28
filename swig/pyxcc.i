%module pyxcc
%{
#define SWIG_FILE_WITH_INIT
#include <xcc/simple.h>
#include <xcc/simple.hpp>
%}

%include <stdint.i>
%include "std_vector.i"

%template(vectorstr) std::vector<char*>;
%template(vectori) std::vector<int32_t>;

%include <xcc/simple.h>
%include <xcc/simple.hpp>

%template(xccs_x) xccs_wrapper<xccs_init_x>;
%template(xccs_c) xccs_wrapper<xccs_init_c>;
%template(xccs_m) xccs_wrapper<xccs_init_m>;

%extend xccs_wrapper<xccs_init_x> {
    int32_t __getitem__(unsigned int i) {
        return (*($self))[i];
    }
}
%extend xccs_wrapper<xccs_init_c> {
    int32_t __getitem__(unsigned int i) {
        return (*($self))[i];
    }
}
%extend xccs_wrapper<xccs_init_m> {
    int32_t __getitem__(unsigned int i) {
        return (*($self))[i];
    }
}
