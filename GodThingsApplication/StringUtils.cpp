#include "StringUtils.h"
#include <sstream>
#include <iterator>
#include <locale>
#include <codecvt>

const char* ws = " \t\n\r\f\v";
const wchar_t* t_ws = L" \t\n\r\f\v";
bool StringUtils::HasEnding(std::wstring const& fullString, std::wstring const& ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    }
    else {
        return false;
    }
}

std::wstring StringUtils::ToLower(std::wstring const& str) {
   std::wstring res = str;
   std::transform(
       res.begin(), res.end(),
       res.begin(),
       towlower);
   return res;
}

std::wstring StringUtils::ToUpper(std::wstring const& str) {
    std::wstring res = str;
    std::transform(
        res.begin(), res.end(),
        res.begin(),
        towupper);
    return res;
}

std::wstring StringUtils::StringsJoin(std::vector<std::wstring> &vs, std::wstring &delim) {
    std::wstring res;

    for (auto p = vs.begin();
        p != vs.end(); ++p) {
        res += *p;
        if (p != vs.end() - 1)
            res += delim;
    }
    return res;
}

std::wstring StringUtils::StringsJoin(std::vector<std::wstring>& vs, const wchar_t* delim) {
    std::wstring res;

    for (auto p = vs.begin();
        p != vs.end(); ++p) {
        res += *p;
        if (p != vs.end() - 1)
            res += delim;
    }
    return res;
}

std::string StringUtils::StringsJoin(std::vector<std::string>& vs, const char* delim) {
    std::string res;

    for (auto p = vs.begin();
        p != vs.end(); ++p) {
        res += *p;
        if (p != vs.end() - 1)
            res += delim;
    }
    return res;
}

void StringUtils::replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

void StringUtils::replaceAll(std::wstring& str, const std::wstring& from, const std::wstring& to) {
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::wstring::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

std::vector<std::wstring> StringUtils::StringSplit(std::wstring s, std::wstring delim) {
    size_t pos_start = 0, pos_end, delim_len = delim.length();
    std::wstring token;
    std::vector<std::wstring> res;

    while ((pos_end = s.find(delim, pos_start)) != std::wstring::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}

bool StringUtils::IsNumeric(std::wstring s) {
    for (auto i = s.begin(); i < s.end(); i++) {
        if (!iswdigit(*i)) {
            return false;
        }
    }
    return true;
}
void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](char ch) {
        return !isspace(ch);
        }));
}

void ltrim(std::wstring& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](wchar_t ch) {
        return !iswspace(ch);
        }));
}

void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](char ch) {
        return !isspace(ch);
        }).base(), s.end());
}

void rtrim(std::string& s,std::string _t) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](char ch) {
        return !isspace(ch);
        }).base(), s.end());
}

// trim from end (in place)
void rtrim(std::wstring& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](wchar_t ch) {
        return !iswspace(ch);
        }).base(), s.end());
}

void trim(std::wstring& s) {
    ltrim(s);
    rtrim(s);
}

void trim(std::string& s) {
    ltrim(s);
    rtrim(s);
}

std::wstring ltrim_copy(std::wstring s) {
    ltrim(s);
    return s;
}

// trim from end (copying)
std::wstring rtrim_copy(std::wstring s) {
    rtrim(s);
    return s;
}

std::wstring trim_copy(std::wstring s) {
    trim(s);
    return s;
}

std::wstring StringUtils::Trim(std::wstring s) {
    trim(s, L" \t\n\r\f\v");
    return s;
}

std::string StringUtils::Trim(std::string s) {
    trim(s," \t\n\r\f\v");
    return s;
}

std::wstring StringUtils::Trim(std::wstring s, std::wstring _t) {
    return std::wstring();
}

std::string StringUtils::ws2s(const std::wstring& wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.to_bytes(wstr);
}

std::string StringUtils::ws2s(const wchar_t* wstr){
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.to_bytes(wstr);
}

std::wstring StringUtils::s2ws(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wstr = converter.from_bytes(str);
    return wstr;
}

std::wstring StringUtils::s2ws(const char* str){
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wstr = converter.from_bytes(str);
    return wstr;
}

inline std::string& StringUtils::rtrim(std::string& s, const char* t = ws)
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from beginning of string (left)
inline std::string& StringUtils::ltrim(std::string& s, const char* t = ws)
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from both ends of string (right then left)
inline std::string& StringUtils::trim(std::string& s, const char* t = ws)
{
    return ltrim(rtrim(s, t), t);
}

inline std::wstring& StringUtils::rtrim(std::wstring& s, const wchar_t* t = t_ws)
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from beginning of string (left)
inline std::wstring& StringUtils::ltrim(std::wstring& s, const wchar_t* t = t_ws)
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from both ends of string (right then left)
inline std::wstring& StringUtils::trim(std::wstring& s, const wchar_t* t = t_ws)
{
    return ltrim(rtrim(s, t), t);
}