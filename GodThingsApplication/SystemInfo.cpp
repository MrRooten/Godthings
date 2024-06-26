#include "SystemInfo.h"
#include "SystemInfo.h"
#include "SystemInfo.h"
#include "SystemInfo.h"

SystemInfo::SystemInfo() {
	if (NtQuerySystemInformation == NULL) {
		NtQuerySystemInformation = (pNtQuerySystemInformation)GetNativeProc("NtQuerySystemInformation");
		if (NtQuerySystemInformation == NULL) {
			return;
		}
	}
}

SystemInfo::~SystemInfo() {
	if (this->pBasicInfo != NULL) {
		GlobalFree(this->pBasicInfo);
		this->pBasicInfo = NULL;
	}

	if (this->pProcessorInfo != NULL) {
		GlobalFree(this->pProcessorInfo);
		this->pProcessorInfo = NULL;
	}
}
DWORD SystemInfo::SetBasicInfo() {
	if (this->pBasicInfo == NULL)
		this->pBasicInfo = (PSYSTEM_BASIC_INFORMATION)GlobalAlloc(GPTR,sizeof(SYSTEM_BASIC_INFORMATION));

	if (this->pBasicInfo == NULL) {
		return GetLastError();
	}
	DWORD dwSize;
	NTSTATUS status;
	status = NtQuerySystemInformation(SystemBasicInformation, this->pBasicInfo, sizeof(SYSTEM_BASIC_INFORMATION), &dwSize);
	if (status != 0) {
		return -1;
	}
	return 0;
}

INT SystemInfo::GetProcessorNumber() {
	if (this->pBasicInfo != NULL)
		return pBasicInfo->NumberOfProcessors;
	return 0;
}


DWORD SystemInfo::SetProcessorInfo() {
	this->pProcessorInfo = (PSYSTEM_PROCESSOR_INFORMATION)GlobalAlloc(GPTR, sizeof(SYSTEM_PROCESSOR_INFORMATION));
	if (this->pProcessorInfo == NULL) {
		return GetLastError();
	}
	DWORD dwSize = 0;
	NTSTATUS status = 0;
	status = NtQuerySystemInformation(SystemProcessorInformation, this->pProcessorInfo, sizeof(SYSTEM_PROCESSOR_INFORMATION), &dwSize);
	return NtStatusHandler(status);
}

std::wstring SystemInfo::GetProcessorArch() {
	if (this->pProcessorInfo == NULL) {
		DWORD status = SetProcessorInfo();
		if (status != ERROR_SUCCESS) {
			return L"";
		}
	}

	switch (this->pProcessorInfo->ProcessorArchitecture) {
	case PROCESSOR_ALPHA_21064: {
		return L"ALPHA_21064";
	}
	case PROCESSOR_AMD_X8664: {
		return L"AMD_X8664";
	}
	case PROCESSOR_ARCHITECTURE_ALPHA: {
		return L"ALPHA";
	}
	case PROCESSOR_ARCHITECTURE_ALPHA64: {
		return L"ALPHA64";
	}
	case PROCESSOR_ARCHITECTURE_AMD64: {
		return L"AMD64";
	}
	case PROCESSOR_ARCHITECTURE_ARM: {
		return L"ARM";
	}
	case PROCESSOR_ARCHITECTURE_ARM32_ON_WIN64: {
		return L"ARM32_ON_WIN64";
	}
	case PROCESSOR_ARCHITECTURE_ARM64: {
		return L"ARM64";
	}
	case PROCESSOR_ARCHITECTURE_IA32_ON_ARM64: {
		return L"IA32_ON_ARM64";
	}
	case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64: {
		return L"IA32_ON_WIN64";
	}
	case PROCESSOR_ARCHITECTURE_IA64: {
		return L"IA64";

	}
	case PROCESSOR_ARCHITECTURE_INTEL: {
		return L"INTEL";
	}
	case PROCESSOR_ARCHITECTURE_MIPS: {
		return L"MIPS";
	}
	case PROCESSOR_ARCHITECTURE_MSIL: {
		return L"MSIL";
	}
	case PROCESSOR_ARCHITECTURE_NEUTRAL: {
		return L"NEUTRAL";
	}
	}
	return L"";
}

SystemTimeInfo SystemInfo::GetSystemTimeInfo() {
	FILETIME idleTime;
	FILETIME kernelTime;
	FILETIME userTime;
	GetSystemTimes(&idleTime, &kernelTime, &userTime);
	return SystemTimeInfo{
		idleTime,
		kernelTime,
		userTime
	};
}

DWORD SystemInfo::SetPerformanceInfo() {
	this->pPerformanceInfo = (PSYSTEM_PERFORMANCE_INFORMATION)GlobalAlloc(GPTR, sizeof(SYSTEM_PERFORMANCE_INFORMATION));
	if (this->pPerformanceInfo == NULL) {
		return GetLastError();
	}
	DWORD dwSize;
	NTSTATUS status;
	status = NtQuerySystemInformation(SystemPerformanceInformation, this->pPerformanceInfo, sizeof(SYSTEM_PERFORMANCE_INFORMATION), &dwSize);
	return NtStatusHandler(status);
}

DWORD SystemInfo::SetTimeOfDayInfo() {
	if (this->pTimeOfDayInfo == NULL)
		this->pTimeOfDayInfo = (PSYSTEM_TIMEOFDAY_INFORMATION)GlobalAlloc(GPTR, sizeof(SYSTEM_TIMEOFDAY_INFORMATION));

	if (this->pTimeOfDayInfo == NULL) {
		return GetLastError();
	}
	DWORD dwSize = 0;
	NTSTATUS status = 0;
	status = NtQuerySystemInformation(SystemTimeOfDayInformation, this->pTimeOfDayInfo, sizeof SYSTEM_TIMEOFDAY_INFORMATION, &dwSize);
	return NtStatusHandler(status);
}

DWORD SystemInfo::SetProcessorPerformanceInfo() {
	if (this->pProcessPerformanceInfo == NULL)
		this->pProcessPerformanceInfo = (PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION)\
			GlobalAlloc(GPTR, sizeof SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION);
	if (this->pProcessPerformanceInfo == NULL) {
		return GetLastError();
	}
	DWORD dwSize = 0;
	NTSTATUS status = 0;
	status = NtQuerySystemInformation(SystemProcessorPerformanceInformation,
		this->pProcessPerformanceInfo,
		sizeof SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION,
		&dwSize);
	return NtStatusHandler(status);
}


DWORD SystemInfo::SetFlagsInfo() {
	if (this->pFlagsInfo == NULL) {
		this->pFlagsInfo = (PSYSTEM_FLAGS_INFORMATION)\
			LocalAlloc(GPTR, sizeof SYSTEM_FLAGS_INFORMATION);
	}

	if (this->pFlagsInfo == NULL) {
		return GetLastError();
	}
	DWORD dwSize = 0;
	NTSTATUS status = 0;
	status = NtQuerySystemInformation(SystemFlagsInformation,
		this->pFlagsInfo,
		sizeof SYSTEM_FLAGS_INFORMATION,
		&dwSize);
	return NtStatusHandler(status);
}
#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)
#define STATUS_UNSUCCESSFUL           ((NTSTATUS)0xC0000001L)
#include "ObjectInfo.h"
DWORD SystemInfo::SetSystemHandles()
{
	static ULONG initialBufferSize = 0x10000;
	NTSTATUS status;
	PVOID buffer;
	ULONG bufferSize;
	ULONG returnLength = 0;
	ULONG attempts = 0;

	bufferSize = initialBufferSize;
	buffer = malloc(bufferSize);

	status = NtQuerySystemInformation(
		SystemHandleInformation,
		buffer,
		bufferSize,
		&returnLength
	);

	while (status == STATUS_INFO_LENGTH_MISMATCH && attempts < 10)
	{
		free(buffer);
		bufferSize = returnLength;
		buffer = malloc(bufferSize);

		status = NtQuerySystemInformation(
			SystemHandleInformation,
			buffer,
			bufferSize,
			&returnLength
		);

		attempts++;
	}

	if (status != 0)
	{
		// Fall back to using the previous code that we've used since Windows XP (dmex)
		bufferSize = initialBufferSize;
		buffer = malloc(bufferSize);

		while ((status = NtQuerySystemInformation(
			SystemHandleInformation,
			buffer,
			bufferSize,
			NULL
		)) == STATUS_INFO_LENGTH_MISMATCH)
		{
			free(buffer);
			bufferSize *= 2;

			// Fail if we're resizing the buffer to something very large.
			if (bufferSize > (256 * 1024 * 1024))
				return 0xC0000009A; //STATUS_INSUFFICIENT_RESOURCES

			buffer = malloc(bufferSize);
		}
	}

	if (status != 0)
	{
		free(buffer);
		return status;
	}

	if (bufferSize <= 0x200000) initialBufferSize = bufferSize;
	this->pSystemHandleInfoEx = (PSYSTEM_HANDLE_INFORMATION)buffer;
	return status;
}

POSVERSIONINFOEXW SystemInfo::GetSystemVersion() {
	POSVERSIONINFOEXW version = nullptr;
	//GetVersionExW((LPOSVERSIONINFOW)&version);
	return version;
}
#include "NtProcessInfo.h"
#include "ProcessUtils.h"
#include "Process.h"
std::map<DWORD, std::set<std::pair<FileType, GTWString>>> SystemInfo::GetSystemLoadedFiles() {
	if (this->pSystemHandleInfoEx != NULL) {
		LocalFree(pSystemHandleInfoEx);
	}
	std::map<DWORD, std::set<std::pair<FileType, GTWString>>> result;
	this->SetSystemHandles();
	pNtDuplicateObject NtDuplicateObject = (pNtDuplicateObject)GetNativeProc("NtDuplicateObject");
	if (NtDuplicateObject == NULL) {
		LOG_ERROR_REASON(L"NtDuplicateObject failed to load");
		return result;
	}
	ProcessManager* mgr = ProcessManager::GetMgr();
	mgr->SetAllProcesses();
	std::map<DWORD, std::set<GTWString>> resultSet;
	for (ULONG i = 0; i < this->pSystemHandleInfoEx->NumberOfHandles; i++) {
		auto pid = this->pSystemHandleInfoEx->Handles[i].UniqueProcessId;
		if (mgr->processesMap.contains(pid) == false) {
			continue;
		}
		HANDLE hObject = NULL;
		auto status = NtDuplicateObject(
			mgr->processesMap[pid]->GetCachedHandle(PROCESS_DUP_HANDLE),
			(HANDLE)this->pSystemHandleInfoEx->Handles[i].HandleValue,
			GetCurrentProcess(),
			&hObject,
			0,
			0,
			0
		);

		SetLastError(NtStatusHandler(status));
		if (status != 0) {
			continue;
		}
		auto t_name = ObjectInfo::GetTypeName(hObject);
		if (_wcsicmp(t_name.c_str(), L"Directory") == 0 || _wcsicmp(t_name.c_str(), L"File") == 0 || _wcsicmp(t_name.c_str(), L"Key")) {
			WCHAR filename[MAX_PATH];
			auto a = GetFileType(hObject);
			if (a == FILE_TYPE_PIPE || a == FILE_TYPE_CHAR) {
				CloseHandle(hObject);
				continue;
			}
			//status = GetFinalPathNameByHandleW(hObject, filename, MAX_PATH, VOLUME_NAME_NT);
			auto filename2 = ObjectInfo::GetObjectName(hObject);
			if (status != 0) {
				CloseHandle(hObject);
				continue;
			}

			if (result.contains(pid) == false) {
				std::set<std::pair<FileType, GTWString>> v;
				result[pid] = v;
			}
			FileType ft;
			if (_wcsicmp(t_name.c_str(), L"Directory") == 0) {
				ft = SysDirectory;
			}
			else if (_wcsicmp(t_name.c_str(), L"File") == 0) {
				ft = SysFile;
			}
			else {
				ft = SysKey;
			}
			auto processName = mgr->processesMap[pid]->GetProcessName();
			
			result[pid].insert(std::pair<FileType, GTWString>(ft, filename2));
		}
		
		CloseHandle(hObject);
	}

	return result;
}

VOID SystemInfo::IterateSystemHandle(std::function<void(DWORD, HANDLE)> handler, std::set<DWORD>& pids) {
	if (this->pSystemHandleInfoEx != NULL) {
		LocalFree(pSystemHandleInfoEx);
	}
	this->SetSystemHandles();
	pNtDuplicateObject NtDuplicateObject = (pNtDuplicateObject)GetNativeProc("NtDuplicateObject");
	if (NtDuplicateObject == NULL) {
		LOG_ERROR_REASON(L"NtDuplicateObject failed to load");
		return ;
	}
	ProcessManager* mgr = ProcessManager::GetMgr();
	for (ULONG i = 0; i < this->pSystemHandleInfoEx->NumberOfHandles; i++) {
		auto pid = this->pSystemHandleInfoEx->Handles[i].UniqueProcessId;
		if (mgr->processesMap.contains(pid) == false) {
			continue;
		}

		if (pids.contains(pid) == false) {
			continue;
		}
		HANDLE hObject = NULL;
		auto status = NtDuplicateObject(
			mgr->processesMap[pid]->GetCachedHandle(PROCESS_DUP_HANDLE),
			(HANDLE)this->pSystemHandleInfoEx->Handles[i].HandleValue,
			GetCurrentProcess(),
			&hObject,
			0,
			0,
			0
		);

		SetLastError(NtStatusHandler(status));
		if (status != 0) {
			continue;
		}
		handler(pid, hObject);

		CloseHandle(hObject);
	}
	return VOID();
}

GTWString SystemInfo::GetCPU()
{
	return GTWString();
}


DWORD SystemInfo::SetPoolTag()
{
	return 0;
}
