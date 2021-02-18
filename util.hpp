#pragma once

#include <codecvt>
#include <locale>
#include <string>
#include <type_traits>

namespace dancing_links {
// Roughly following
// https://stackoverflow.com/a/31302660
template<typename T>
inline std::string
WStringToUtf8Str(T m) {
  if constexpr(std::is_same_v<T, std::string>) {
    return m;
  } else if constexpr(std::is_same_v<T, std::u16string>) {
    std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> conv;
    return conv.to_bytes(m);
  } else if constexpr(std::is_same_v<T, std::u32string>) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.to_bytes(m);
  } else if constexpr(std::is_same_v<T, char32_t>) {
    std::u32string str;
    str += m;
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.to_bytes(str);
  } else {
    return std::to_string(m);
  }
}

std::u32string
Utf8StringToUTF32String(const std::string& s);
}
