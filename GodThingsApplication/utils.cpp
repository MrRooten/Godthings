#include "utils.h"
#include "utils.h"
#include "utils.h"
#include "utils.h"
#include "StringUtils.h"

LOG_LEVEL GlobalLogLevel = INFO_LEVEL;

UINT16 MPEBytes::BytesToINT16B(PBYTE bytes) {
	return (bytes[0] << 8) + bytes[1];
}

UINT16 MPEBytes::BytesToINT16L(PBYTE bytes) {
	return (bytes[1] << 8) + bytes[0];
}

UINT32 MPEBytes::BytesToINT32B(PBYTE bytes) {
	return (bytes[0] << 24) + (bytes[1] << 16) + (bytes[2] << 8) + bytes[3];
}

UINT32 MPEBytes::BytesToINT32L(PBYTE bytes) {
	return (bytes[3] << 24) + (bytes[2] << 16) + (bytes[1] << 8) + bytes[0];
}

UINT64 MPEBytes::BytesToINT64B(PBYTE bytes) {
	uint64_t value =
		static_cast<uint64_t>(bytes[7]) |
		static_cast<uint64_t>(bytes[6]) << 8 |
		static_cast<uint64_t>(bytes[5]) << 16 |
		static_cast<uint64_t>(bytes[4]) << 24 |
		static_cast<uint64_t>(bytes[3]) << 32 |
		static_cast<uint64_t>(bytes[2]) << 40 |
		static_cast<uint64_t>(bytes[1]) << 48 |
		static_cast<uint64_t>(bytes[0]) << 56;
	return value;
}

UINT64 MPEBytes::BytesToINT64L(PBYTE bytes) {
	uint64_t value =
		static_cast<uint64_t>(bytes[0]) |
		static_cast<uint64_t>(bytes[1]) << 8 |
		static_cast<uint64_t>(bytes[2]) << 16 |
		static_cast<uint64_t>(bytes[3]) << 24 |
		static_cast<uint64_t>(bytes[4]) << 32 |
		static_cast<uint64_t>(bytes[5]) << 40 |
		static_cast<uint64_t>(bytes[6]) << 48 |
		static_cast<uint64_t>(bytes[7]) << 56;
	return value;
}

MPEBytes MPEBytes::INT16ToBytesB(INT16 integer) {
	BYTE bytes[2];
	bytes[0] = integer >> 8;
	bytes[1] = integer & 0xff;
	return MPEBytes(bytes, 2);
}

MPEBytes MPEBytes::INT32ToBytesB(INT32 integer) {
	BYTE bytes[4];
	bytes[0] = (integer & 0xff000000) >> 24;
	bytes[1] = (integer & 0x00ff0000) >> 16;
	bytes[2] = (integer & 0x0000ff00)>> 8;
	bytes[3] = (integer & 0xff);
	return MPEBytes(bytes, 4);
}

MPEBytes MPEBytes::INT64ToBytesB(INT64 integer) {
	BYTE bytes[8];
	bytes[0] = (BYTE)((integer & 0xff00000000000000) >> 56);
	bytes[1] = (BYTE)((integer & 0x00ff000000000000) >> 48);
	bytes[2] = (BYTE)((integer & 0x0000ff0000000000) >> 40);
	bytes[3] = (BYTE)((integer & 0x000000ff00000000) >> 32);
	bytes[4] = (BYTE)((integer & 0x00000000ff000000)>> 24);
	bytes[5] = (BYTE)((integer & 0x0000000000ff0000)>> 16);
	bytes[6] = (BYTE)((integer & 0x000000000000ff00)>> 8);
	bytes[7] = (BYTE)(integer & 0xff);
	return MPEBytes(bytes, 8);
}

MPEBytes::MPEBytes() {
	this->size = 0;
	this->bytes = (PBYTE)GlobalAlloc(GPTR, 0);
}

MPEBytes::MPEBytes(PBYTE bytes,size_t size) {
	if (this->size == 0) {
		this->bytes = (PBYTE)GlobalAlloc(GPTR, sizeof(BYTE) * size);
	}

	if (this->bytes == NULL) {
		this->error = GetLastError();
		return;
	}

	memcpy(this->bytes, bytes, size * sizeof(BYTE));
	this->size = size;
}

PBYTE MPEBytes::ToBytes() {
	return this->bytes;
}

VOID MPEBytes::AddBytes(PBYTE bytes,size_t size) {
	this->bytes = (PBYTE)GlobalReAlloc(this->bytes, this->size + size, GMEM_MOVEABLE);
	if (this->bytes == NULL) {
		this->error = GetLastError();
		return;
	}

	memcpy(this->bytes + this->size, bytes, size);
	this->size += size;
}

VOID MPEBytes::AddBytes(MPEBytes& mpeBytes) {
	PBYTE bytes = mpeBytes.ToBytes();
	size_t size = mpeBytes.size;
	this->bytes = (PBYTE)GlobalReAlloc(this->bytes, this->size + size, GMEM_MOVEABLE);
	if (this->bytes == NULL) {
		this->error = GetLastError();
		return;
	}

	memcpy(this->bytes + this->size, bytes, size);
	this->size += size;
}

PBYTE& MPEBytes::GetBytes() {
	return this->bytes;
}

MPEBytes::~MPEBytes() {
	GlobalFree((HGLOBAL)this->bytes);
}

VOID GTPrintln(const WCHAR* messageFormat, ...) {
	wchar_t buffer[1024] = { 0 };
	va_list vaList;//equal to Format + sizeof(FOrmat)
	va_start(vaList, messageFormat);
	_vsnwprintf(buffer, 1024, messageFormat, vaList);
	va_end(vaList);
	
	if (sizeof(TCHAR) != sizeof(CHAR)) {
		wprintf(L"%s\n", buffer);
	}
	else {
		const char* c = StringUtils::ws2s(buffer).c_str();
		printf_s("%s\n", c);
	}
}

VOID SetGloablLogLevel(LOG_LEVEL level) {
	GlobalLogLevel = level;
}

VOID Logln(LOG_LEVEL logLevel, const WCHAR* messageFormat, ...) {
	wchar_t buffer[1024] = { 0 };
	va_list vaList;//equal to Format + sizeof(FOrmat)
	va_start(vaList, messageFormat);
	_vsnwprintf_s(buffer, 1024, messageFormat, vaList);
	va_end(vaList);
	if (logLevel <= GlobalLogLevel) {
		if (logLevel == DEBUG_LEVEL) {
			wprintf_s(L"[DBG]:%s\n",buffer);
		}
		else if (logLevel == INFO_LEVEL) {
			wprintf_s(L"[INF]:%s\n", buffer);
		}
		else if (logLevel == WARNING_LEVEL) {
			wprintf_s(L"[WRN]:%s\n", buffer);
		}
		else if (logLevel == ERROR_LEVEL) {
			wprintf_s(L"[ERR]:%s\n", buffer);
		}
	}
}



static wchar_t message[100];
LPWSTR GetLastErrorAsString() {
	//Get the error message ID, if any.
	DWORD errorMessageID = ::GetLastError();
	ZeroMemory(message, sizeof(WCHAR) * 100);
	WCHAR *messageBuffer = message;
	//Ask Win32 to give us the string version of that message ID.
	//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
	size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPWSTR)&messageBuffer, 0, NULL);

	//Copy the error message into a std::string.
	std::wstring message(messageBuffer, size);

	//Free the Win32's string's buffer.
	return messageBuffer;
}

std::wstring GetLastErrorAsStringThreadSafe() {
	//Get the error message ID, if any.
	DWORD errorMessageID = ::GetLastError();
	ZeroMemory(message, sizeof(WCHAR) * 100);
	WCHAR* messageBuffer = message;
	//Ask Win32 to give us the string version of that message ID.
	//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
	size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

	//Copy the error message into a std::string.
	std::wstring message(messageBuffer, size);

	//Free the Win32's string's buffer.
	return message;
}
std::wstring s2ws(const std::string& str) {
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

std::wstring GTTime::String_utc_to_local() {
	FILETIME pUTC;
	FileTimeToLocalFileTime(&this->fTime, &pUTC);
	GTTime t(pUTC);
	WCHAR buf[100];
	swprintf_s(buf, L"%d-%02d-%02d %02d:%02d:%02d", t.year, t.month, t.day, t.hour, t.minute, t.second);
	return buf;
}

std::wstring GTTime::String() {
	WCHAR buf[100];
	swprintf_s(buf, L"%d-%02d-%02d %02d:%02d:%02d", this->year, this->month, this->day, this->hour, this->minute, this->second);
	return buf;
}

GTTime GTTime::GetTime() {
	SYSTEMTIME st;
	GetLocalTime(&st); //获取本地时间
	return GTTime(st);
}

FILETIME UnixTimeToFileTime(time_t t) {
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll;
	FILETIME ft;

	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	ft.dwLowDateTime = (DWORD)ll;
	ft.dwHighDateTime = ll >> 32;
	return ft;
}

GTTime GTTime::FromTimeStamp(UINT32 timestamp) {
	time_t t = timestamp;
	auto ft = UnixTimeToFileTime(t);
	return GTTime(ft);
}

GTTime GTTime::FromTimeStamp64(UINT64 time) {
	FILETIME ft;
	ft.dwLowDateTime = time & 0xffffffff;
	ft.dwHighDateTime = time >> 32;
	return GTTime(ft);
}
GTTime GTTime::FromISO8601(GTWString time) {
	GTTime t;
	swscanf(time.c_str(), L"%d-%d-%dT%d:%d:%d.%dZ", &t.year, &t.month, &t.day, &t.hour, &t.minute, &t.second,&t.millisecond);
	SYSTEMTIME s_time;
	s_time.wYear = t.year;
	s_time.wMonth = t.month;
	s_time.wDay = t.day;
	s_time.wHour = t.hour;
	s_time.wMinute = t.minute;
	s_time.wSecond = t.second;
	//s_time.wMilliseconds = t.millisecond;
	auto s = SystemTimeToFileTime(&s_time, &t.fTime);
	return t;
}
ULONG64 GTTime::ToNowULONG64() {
	ULONG64 res = 0;
	res += this->millisecond;
	res += this->second * 1000;
	res += this->minute * 60 * 1000;
	res += this->hour * 60 * 60 * 1000;
	res += this->day * 24 * 60 * 60 * 1000;
	return res;
}
bool GTTime::operator<(GTTime& other) {
	if (this->year < other.year) {
		return true;
	}

	if (this->month < other.month) {
		return true;
	}

	if (this->day < other.day) {
		return true;
	}

	if (this->hour < other.hour) {
		return true;
	}

	if (this->minute < other.minute) {
		return true;
	}

	if (this->second < other.second) {
		return true;
	}

	if (this->millisecond < other.millisecond) {
		return true;
	}
	return false;
}
bool GTTime::operator>(GTTime& other) {
	if (*this < other) {
		return false;
	}

	if (*this == other) {
		return false;
	}
	return true;
}
bool GTTime::operator==(GTTime& other) {
	if (this->year != other.year) {
		return false;
	}

	if (this->month != other.month) {
		return false;
	}

	if (this->day != other.day) {
		return false;
	}

	if (this->hour != other.hour) {
		return false;
	}

	if (this->minute != other.minute) {
		return false;
	}

	if (this->second != other.second) {
		return false;
	}

	if (this->millisecond != other.millisecond) {
		return false;
	}
	return true;
}
bool GTTime::operator>=(GTTime& other) {
	if (*this == other) {
		return true;
	}

	if (*this > other) {
		return true;
	}

	return false;
}
bool GTTime::operator<=(GTTime& other) {
	if (*this == other) {
		return true;
	}

	if (*this < other) {
		return true;
	}

	return false;
}

#include <cstdlib>
#include <ctime>
#include <string>

#ifdef _WIN32
#define timegm _mkgmtime
#endif

inline int ParseInt(const wchar_t* value)
{
	return std::wcstol(value, nullptr, 10);
}

// ParseISO8601 returns milliseconds since 1970
#include <chrono>
std::time_t ParseISO8601(const std::wstring& input)
{
	constexpr const size_t expectedLength = sizeof("1234-12-12T12:12:12Z") - 1;
	static_assert(expectedLength == 20, "Unexpected ISO 8601 date/time length");

	if (input.length() < expectedLength)
	{
		return 0;
	}

	std::tm time = { 0 };
	time.tm_year = ParseInt(&input[0]) - 1900;
	time.tm_mon = ParseInt(&input[5]) - 1;
	time.tm_mday = ParseInt(&input[8]);
	time.tm_hour = ParseInt(&input[11]);
	time.tm_min = ParseInt(&input[14]);
	time.tm_sec = ParseInt(&input[17]);
	time.tm_isdst = 0;
	const int millis = input.length() > 20 ? ParseInt(&input[20]) : 0;
	return timegm(&time) * 1000 + millis;
}

INT64 GTTime::operator-(GTTime& other) {
	auto t = this->ToISO8601();
	auto t2 = other.ToISO8601();
	auto iso8601_t1 = ParseISO8601(t);
	auto iso8601_t2 = ParseISO8601(t2);
	auto s1 = std::chrono::seconds(iso8601_t1);
	auto s2 = std::chrono::seconds(iso8601_t2);
	auto delta = s1 - s2;
	auto c = delta.count();
	return delta.count();
}
GTTime::GTTime(FILETIME &filetime) {
	this->fTime = filetime;
	SYSTEMTIME utc;
	FileTimeToSystemTime(std::addressof(filetime), std::addressof(utc));
	this->sysTime = utc;
	std::ostringstream stm;
	const auto w2 = std::setw(2);
	this->year = utc.wYear;
	this->month = utc.wMonth;
	this->day = utc.wDay;
	this->hour = utc.wHour;
	this->minute = utc.wMinute;
	this->second = utc.wSecond;
	this->millisecond = utc.wMilliseconds;
}
GTTime::GTTime(SYSTEMTIME &utc) {
	std::ostringstream stm;
	const auto w2 = std::setw(2);
	SystemTimeToFileTime(&utc, &this->fTime);
	this->sysTime = utc;
	this->year = utc.wYear;
	this->month = utc.wMonth;
	this->day = utc.wDay;
	this->hour = utc.wHour;
	this->minute = utc.wMinute;
	this->second = utc.wSecond;
	this->millisecond = utc.wMilliseconds;
}
GTTime::GTTime(const char* time) {
	std::wstring s = StringUtils::s2ws(time);
	new (this)GTTime(s.c_str());
}
GTTime::GTTime(const wchar_t* time) {
	std::wstring s = time;
	auto ss = StringUtils::StringSplit(s, L"-");
	auto len = ss.size();
	if (len > 0) {
		this->year = std::stoi(ss[0]);
	}

	if (len > 1) {
		this->month = std::stoi(ss[1]);
	}

	if (len > 2) {
		this->day = std::stoi(ss[2]);
	}

	if (len > 3) {
		this->hour = std::stoi(ss[3]);
	}

	if (len > 4) {
		this->minute = std::stoi(ss[4]);
	}

	if (len > 5) {
		this->second = std::stoi(ss[5]);
	}

	if (len > 6) {
		this->second = std::stoi(ss[6]);
	}
}
GTTime::GTTime() {
	
}

std::wstring _helperISO8601(GTTime &t) {
	WCHAR buf[100];
	swprintf_s(buf, L"%d-%02d-%02dT%02d:%02d:%02d:%3dZ", t.year, t.month, t.day, t.hour, t.minute, t.second,t.millisecond);
	return buf;
}

std::wstring GTTime::ToISO8601() {
	FILETIME pUTC;
	FileTimeToLocalFileTime(&this->fTime, &pUTC);
	GTTime t(pUTC);
	return _helperISO8601(t);
}

char* GTException::what() {
	return (char*)this->msg.c_str();
}

GTException::GTException(const char* msg) {
	this->msg = msg;
}