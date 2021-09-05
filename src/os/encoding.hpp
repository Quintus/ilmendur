#ifndef ILMENDUR_OS_ENCODING_HPP
#define ILMENDUR_OS_ENCODING_HPP
#include <string>

#ifdef _WIN32
#include <type_traits>
#include <windows.h>
#endif

namespace OS {

#if defined(_WIN32)
    template<typename FromStringType, typename ToStringType>
    ToStringType convert_encodings(const std::string& from_encoding, const std::string& to_encoding, const FromStringType& source_str)
    {
        int count = 0;

        if (from_encoding == "UTF-8" && to_encoding == "UTF-16") {
            static_assert(std::is_same<source_str::CharT, char>, "On Windows, converting from UTF-8 requires source_str to be a std::string");
            count = MultiByteToWideChar(CP_UTF8, 0, source_str.c_str(), source_str.length(), NULL, 0);
        } else if (from_encoding == "UTF-16" && to_encoding == "UTF-8") {
            static_assert(std::is_same<source_str::CharT, wchar_t>, "On Windows, converting from UTF-16 requires source_str to be a std::wstring");
            count = WideCharToMultiByte(CP_UTF8, 0, source_str.c_str(), source_str.length(), NULL, 0, NULL, NULL);
        } else {
            throw std::runtime_error("On Windows, can only convert UTF-8 to UTF-16 and vice-versa.");
        }

        ToStringType::CharT* p_target = new ToStringType::CharT[count * sizeof(ToStringType::CharT)]();

        if (from_encoding == "UTF-8" && to_encoding == "UTF-16") {
            static_assert(std::is_same<source_str::CharT, char>, "On Windows, converting from UTF-8 requires source_str to be a std::string");
            count = MultiByteToWideChar(CP_UTF8, 0, source_str.c_str(), source_str.length(), p_target, count);
        } else if (from_encoding == "UTF-16" && to_encoding == "UTF-8") {
            static_assert(std::is_same<source_str::CharT, wchar_t>, "On Windows, converting from UTF-16 requires source_str to be a std::wstring");
            count = WideCharToMultiByte(CP_UTF8, 0, source_str.c_str(), source_str.length(), p_target, count,  NULL, NULL);
        } else {
            throw std::runtime_error("On Windows, can only convert UTF-8 to UTF-16 and vice-versa.");
        }

        if (count == 0) {
            delete[] p_target;
            throw(runtime_error(winerror(GetLastError())));
        }

        ToStringType target_str(p_target, count);
        delete[] p_target;
        return target_str;
    }
#elif defined(__unix__)
    std::string convert_encodings(const std::string& from_encoding, const std::string& to_encoding, const std::string& source_str);
#else
#error Unsupported system
#endif
}

#endif /* ILMENDUR_OS_ENCODING_HPP */

