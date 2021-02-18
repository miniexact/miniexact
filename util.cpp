#include "util.hpp"

namespace dancing_links {

std::u32string
Utf8StringToUTF32String(const std::string& s) {
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
  return conv.from_bytes(s);
}

}
