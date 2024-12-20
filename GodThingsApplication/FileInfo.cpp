#include "FileInfo.h"
#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)
#define STATUS_BUFFER_OVERFLOW           ((NTSTATUS)0x80000005L)
#define STATUS_BUFFER_TOO_SMALL          ((NTSTATUS)0xC0000023L)
DWORD GetFileBasicInfo(LPCWSTR fileName, PFILE_BASIC_INFO* Pinfo) {
	PFILE_BASIC_INFO info = (PFILE_BASIC_INFO)GlobalAlloc(GPTR, sizeof(FILE_BASIC_INFO));
	if (info == NULL) {
		return GetLastError();
	}

	pNtQueryInformationFile NtQueryInfomationFile = (pNtQueryInformationFile)GetNativeProc("NtQueryInformationFile");
	if (NtQueryInfomationFile == NULL) {
		return GetLastError();
	}

	HANDLE hFile = CreateFileW(fileName,
		GENERIC_READ,
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile == NULL) {
		return GetLastError();
	}
	IO_STATUS_BLOCK block;
	NTSTATUS status = NtQueryInfomationFile(
		hFile,
		&block,
		info,
		sizeof(FILE_BASIC_INFO),
		FileBasicInformation
	);

	return 0;
}

FileInfo::FileInfo(const std::wstring& fileName) {
	this->fileName = fileName;
}

FileInfo::FileInfo(const std::string& fileName) {
	this->fileName = StringUtils::s2ws(fileName);
}

FileInfo::FileInfo(const char* fileName) {
	this->fileName = StringUtils::s2ws(fileName);
}

FileInfo::FileInfo(const wchar_t* fileName) {
	this->fileName = fileName;
}



DWORD FileInfo::SetBasicInfo() {
	if (this->pBasicInfo == NULL) {
		this->pBasicInfo = (PFILE_BASIC_INFORMATION)GlobalAlloc(GPTR, sizeof FILE_BASIC_INFORMATION);
		if (this->pBasicInfo == NULL) {
			return GetLastError();
		}
	}

	ZeroMemory(this->pBasicInfo, sizeof FILE_BASIC_INFORMATION);
	HANDLE hFile = CreateFileW(this->fileName.c_str(),
		GENERIC_READ,
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == NULL) {
		return GetLastError();
	}

	if (this->NtQueryInformationFile == NULL) {
		this->NtQueryInformationFile = (pNtQueryInformationFile)GetNativeProc("NtQueryInformationFile");
		if (this->NtQueryInformationFile == NULL) {
			CloseHandle(hFile);
			return GetLastError();
		}
	}
	IO_STATUS_BLOCK block;
	NTSTATUS status = NtQueryInformationFile(
		hFile,
		&block,
		this->pBasicInfo,
		sizeof(FILE_BASIC_INFORMATION),
		FileBasicInformation
	);
	CloseHandle(hFile);
	return NtStatusHandler(status);
}

std::vector<std::wstring> FileInfo::GetAttributes() {
	std::vector<std::wstring> res;
	if (this->pBasicInfo == NULL) {
		if (this->SetBasicInfo() != ERROR_SUCCESS) {
			return res;
		}
	}
	
	auto &attrs = this->pBasicInfo->FileAttributes;
	if (attrs & FILE_ATTRIBUTE_ARCHIVE) {
		res.push_back(L"FILE_ATTRIBUTE_ARCHIVE");
	}
	if (attrs & FILE_ATTRIBUTE_COMPRESSED) {
		res.push_back(L"FILE_ATTRIBUTE_COMPRESSED");
	}
	if (attrs & FILE_ATTRIBUTE_DEVICE) {
		res.push_back(L"FILE_ATTRIBUTE_DEVICE");
	}
	if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
		res.push_back(L"FILE_ATTRIBUTE_DIRECTORY");
	}
	if (attrs & FILE_ATTRIBUTE_ENCRYPTED) {
		res.push_back(L"FILE_ATTRIBUTE_ENCRYPTED");
	}
	if (attrs & FILE_ATTRIBUTE_HIDDEN) {
		res.push_back(L"FILE_ATTRIBUTE_HIDDEN");
	}
	if (attrs & FILE_ATTRIBUTE_INTEGRITY_STREAM) {
		res.push_back(L"FILE_ATTRIBUTE_INTEGRITY_STREAM");
	}
	if (attrs & FILE_ATTRIBUTE_NORMAL) {
		res.push_back(L"FILE_ATTRIBUTE_NORMAL");
	}
	if (attrs & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) {
		res.push_back(L"FILE_ATTRIBUTE_NOT_CONTENT_INDEXED");
	}
	if (attrs & FILE_ATTRIBUTE_NO_SCRUB_DATA) {
		res.push_back(L"FILE_ATTRIBUTE_NO_SCRUB_DATA");
	}
	if (attrs & FILE_ATTRIBUTE_OFFLINE) {
		res.push_back(L"FILE_ATTRIBUTE_OFFLINE");
	}
	if (attrs & FILE_ATTRIBUTE_READONLY) {
		res.push_back(L"FILE_ATTRIBUTE_READONLY");
	}
	if (attrs & FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS) {
		res.push_back(L"FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS");
	}
	if (attrs & FILE_ATTRIBUTE_RECALL_ON_OPEN) {
		res.push_back(L"FILE_ATTRIBUTE_RECALL_ON_OPEN");
	}
	if (attrs & FILE_ATTRIBUTE_REPARSE_POINT) {
		res.push_back(L"FILE_ATTRIBUTE_REPARSE_POINT");
	}
	if (attrs & FILE_ATTRIBUTE_SPARSE_FILE) {
		res.push_back(L"FILE_ATTRIBUTE_SPARSE_FILE");
	}
	if (attrs & FILE_ATTRIBUTE_SYSTEM) {
		res.push_back(L"FILE_ATTRIBUTE_SYSTEM");
	}
	if (attrs & FILE_ATTRIBUTE_TEMPORARY) {
		res.push_back(L"FILE_ATTRIBUTE_TEMPORARY");
	}
	if (attrs & FILE_ATTRIBUTE_VIRTUAL) {
		res.push_back(L"FILE_ATTRIBUTE_VIRTUAL");
	}
	return res;
}

DWORD FileInfo::SetStandInfo() {
	if (this->pStandardInfo == NULL) {
		this->pStandardInfo = (PFILE_STANDARD_INFORMATION)GlobalAlloc(GPTR, sizeof FILE_STANDARD_INFORMATION);
		if (this->pStandardInfo == NULL) {
			return GetLastError();
		}
	}

	ZeroMemory(this->pStandardInfo, sizeof FILE_STANDARD_INFORMATION);
	HANDLE hFile = CreateFileW(this->fileName.c_str(),
		GENERIC_READ,
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == NULL) {
		return GetLastError();
	}

	if (this->NtQueryInformationFile == NULL) {
		this->NtQueryInformationFile = (pNtQueryInformationFile)GetNativeProc("NtQueryInformationFile");
		if (this->NtQueryInformationFile == NULL) {
			CloseHandle(hFile);
			return GetLastError();
		}
	}
	IO_STATUS_BLOCK block;
	NTSTATUS status = NtQueryInformationFile(
		hFile,
		&block,
		this->pStandardInfo,
		sizeof(FILE_STANDARD_INFORMATION),
		FileStandardInformation
	);
	CloseHandle(hFile);
	return NtStatusHandler(status);
}

DWORD FileInfo::SetAccessInfo() {
	if (this->pAccessInfo == NULL) {
		this->pAccessInfo = (PFILE_ACCESS_INFORMATION)GlobalAlloc(GPTR, sizeof FILE_ACCESS_INFORMATION);
		if (this->pAccessInfo == NULL) {
			return GetLastError();
		}
	}

	ZeroMemory(this->pAccessInfo, sizeof FILE_ACCESS_INFORMATION);
	HANDLE hFile = CreateFileW(this->fileName.c_str(),
		GENERIC_READ,
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == NULL) {
		return GetLastError();
	}

	if (this->NtQueryInformationFile == NULL) {
		this->NtQueryInformationFile = (pNtQueryInformationFile)GetNativeProc("NtQueryInformationFile");
		if (this->NtQueryInformationFile == NULL) {
			CloseHandle(hFile);
			return GetLastError();
		}
	}
	IO_STATUS_BLOCK block;
	NTSTATUS status = NtQueryInformationFile(
		hFile,
		&block,
		this->pAccessInfo,
		sizeof(FILE_ACCESS_INFORMATION),
		FileAccessInformation
	);
	CloseHandle(hFile);
	return NtStatusHandler(status);
}

DWORD FileInfo::SetStatInfo() {
	if (this->pStatInfo == NULL) {
		this->pStatInfo = (PFILE_STAT_INFORMATION)GlobalAlloc(GPTR, sizeof FILE_STAT_INFORMATION);
		if (this->pStatInfo == NULL) {
			return GetLastError();
		}
	}

	ZeroMemory(this->pStatInfo, sizeof FILE_STAT_INFORMATION);
	HANDLE hFile = CreateFileW(this->fileName.c_str(),
		GENERIC_READ,
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == NULL) {
		return GetLastError();
	}

	if (this->NtQueryInformationFile == NULL) {
		this->NtQueryInformationFile = (pNtQueryInformationFile)GetNativeProc("NtQueryInformationFile");
		if (this->NtQueryInformationFile == NULL) {
			CloseHandle(hFile);
			return GetLastError();
		}
	}
	IO_STATUS_BLOCK block;
	NTSTATUS status = NtQueryInformationFile(
		hFile,
		&block,
		this->pStatInfo,
		sizeof(FILE_STAT_INFORMATION),
		FileStatInformation
	);
	CloseHandle(hFile);
	return NtStatusHandler(status);
}

GTTime FileInfo::GetCreateTime() {
	this->SetStatInfo();
	FILETIME fTime;
	fTime.dwLowDateTime = this->pStatInfo->CreationTime.LowPart;
	fTime.dwHighDateTime = this->pStatInfo->CreationTime.HighPart;
	return GTTime(fTime);
}

GTTime FileInfo::GetChangeTime() {
	this->SetStatInfo();
	this->pStatInfo->ChangeTime;
	auto _ = this->pStatInfo->ChangeTime.QuadPart;
	FILETIME fTime;
	memcpy(&fTime, &_, sizeof(fTime));
	return GTTime(fTime);
}

GTTime FileInfo::GetLastAccessTime() {
	this->SetStatInfo();
	this->pStatInfo->LastAccessTime;
	auto _ = this->pStatInfo->CreationTime.QuadPart;
	FILETIME fTime;
	memcpy(&fTime, &_, sizeof(fTime));
	return GTTime(fTime);
}


GTTime FileInfo::GetLastWriteTime() {
	this->SetStatInfo();
	this->pStatInfo->LastWriteTime;
	auto _ = this->pStatInfo->CreationTime.QuadPart;
	FILETIME fTime;
	memcpy(&fTime, &_, sizeof(fTime));
	return GTTime(fTime);
}

DWORD FileInfo::SetCaseSensitiveInfo() {
	if (this->pCaseSensitiveInfo == NULL) {
		this->pCaseSensitiveInfo = (PFILE_CASE_SENSITIVE_INFORMATION)GlobalAlloc(GPTR, sizeof FILE_CASE_SENSITIVE_INFORMATION);
		if (this->pCaseSensitiveInfo == NULL) {
			return GetLastError();
		}
	}

	ZeroMemory(this->pCaseSensitiveInfo, sizeof FILE_CASE_SENSITIVE_INFORMATION);
	HANDLE hFile = CreateFileW(this->fileName.c_str(),
		GENERIC_READ,
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == NULL) {
		return GetLastError();
	}

	if (this->NtQueryInformationFile == NULL) {
		this->NtQueryInformationFile = (pNtQueryInformationFile)GetNativeProc("NtQueryInformationFile");
		if (this->NtQueryInformationFile == NULL) {
			CloseHandle(hFile);
			return GetLastError();
		}
	}
	IO_STATUS_BLOCK block;
	NTSTATUS status = NtQueryInformationFile(
		hFile,
		&block,
		this->pCaseSensitiveInfo,
		sizeof(FILE_CASE_SENSITIVE_INFORMATION),
		FileCaseSensitiveInformation
	);
	CloseHandle(hFile);
	return NtStatusHandler(status);
}

DWORD FileInfo::SetProcessIdsUsingFileInfo() {
	if (this->pProcessIdsUsingFileInfo == NULL) {
		this->pProcessIdsUsingFileInfo = (PFILE_PROCESS_IDS_USING_FILE_INFORMATION)GlobalAlloc(GPTR, sizeof(FILE_PROCESS_IDS_USING_FILE_INFORMATION)+sizeof(ULONG_PTR)*1023);
		if (this->pProcessIdsUsingFileInfo == NULL) {
			return GetLastError();
		}
	}

	ZeroMemory(this->pProcessIdsUsingFileInfo, sizeof FILE_PROCESS_IDS_USING_FILE_INFORMATION);
	HANDLE hFile = CreateFileW(this->fileName.c_str(),
		GENERIC_READ,
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == NULL) {
		return GetLastError();
	}

	if (this->NtQueryInformationFile == NULL) {
		this->NtQueryInformationFile = (pNtQueryInformationFile)GetNativeProc("NtQueryInformationFile");
		if (this->NtQueryInformationFile == NULL) {
			CloseHandle(hFile);
			return GetLastError();
		}
	}
	IO_STATUS_BLOCK block;
	NTSTATUS status = NtQueryInformationFile(
		hFile,
		&block,
		this->pProcessIdsUsingFileInfo,
		sizeof(FILE_PROCESS_IDS_USING_FILE_INFORMATION)+1023*sizeof(ULONG_PTR),
		FileProcessIdsUsingFileInformation
	);

	if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL || status == STATUS_INFO_LENGTH_MISMATCH) {
		this->pProcessIdsUsingFileInfo = (PFILE_PROCESS_IDS_USING_FILE_INFORMATION)\
			GlobalReAlloc(hFile, sizeof(FILE_PROCESS_IDS_USING_FILE_INFORMATION) + sizeof(ULONG_PTR) * 2047,GPTR);
		if (this->pProcessIdsUsingFileInfo == NULL) {
			return GetLastError();
		}
		status = NtQueryInformationFile(
			hFile,
			&block,
			this->pProcessIdsUsingFileInfo,
			sizeof(FILE_PROCESS_IDS_USING_FILE_INFORMATION) + 2047 * sizeof(ULONG_PTR),
			FileProcessIdsUsingFileInformation
		);
	}
	CloseHandle(hFile);
	return NtStatusHandler(status);
}

DWORD FileInfo::SetIoPriorityHintInfo() {
	if (this->pIoPriorityHintInfo == NULL) {
		this->pIoPriorityHintInfo = (PFILE_IO_PRIORITY_HINT_INFORMATION)GlobalAlloc(GPTR, sizeof FILE_IO_PRIORITY_HINT_INFORMATION);
		if (this->pIoPriorityHintInfo == NULL) {
			return GetLastError();
		}
	}

	HANDLE hFile = CreateFileW(this->fileName.c_str(),
		GENERIC_READ,
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == NULL) {
		return GetLastError();
	}

	if (this->NtQueryInformationFile == NULL) {
		this->NtQueryInformationFile = (pNtQueryInformationFile)GetNativeProc("NtQueryInformationFile");
		if (this->NtQueryInformationFile == NULL) {
			CloseHandle(hFile);
			return GetLastError();
		}
	}
	IO_STATUS_BLOCK block;
	NTSTATUS status = NtQueryInformationFile(
		hFile,
		&block,
		this->pIoPriorityHintInfo,
		sizeof(FILE_IO_PRIORITY_HINT_INFORMATION),
		FileIoPriorityHintInformation
	);
	CloseHandle(hFile);
	return NtStatusHandler(status);
}

DWORD FileInfo::SetLinksInfo() {
	DWORD status;
	HANDLE hFile = NULL;
	IO_STATUS_BLOCK block;
	NTSTATUS NtStatus;
	DWORD size = 0x100;
	if (this->pLinksInfo == NULL) {
		this->pLinksInfo = (PFILE_LINKS_INFORMATION)LocalAlloc(GPTR, size);
		if (this->pLinksInfo == NULL) {
			status = GetLastError();
			goto CleanUp;
		}
	}

	hFile = CreateFileW(this->fileName.c_str(),
		GENERIC_READ,
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile == NULL) {
		status = GetLastError();
		goto CleanUp;
	}

	if (this->NtQueryInformationFile == NULL) {
		this->NtQueryInformationFile = (pNtQueryInformationFile)GetNativeProc("NtQueryInformationFile");
		if (this->NtQueryInformationFile == NULL) {
			CloseHandle(hFile);
			return GetLastError();
		}
	}
	
	NtStatus = NtQueryInformationFile(
		hFile,
		&block,
		this->pLinksInfo,
		size,
		FileHardLinkInformation
	);
	if (NtStatus != STATUS_BUFFER_OVERFLOW && NtStatus != STATUS_BUFFER_TOO_SMALL && NtStatus != STATUS_INFO_LENGTH_MISMATCH) {
		status = NtStatusHandler(NtStatus);
		goto CleanUp;
	}

	if (NtStatus == STATUS_BUFFER_OVERFLOW || NtStatus == STATUS_BUFFER_TOO_SMALL || NtStatus == STATUS_INFO_LENGTH_MISMATCH) {
		int i = 0;
		do {
			size = size * 2;
			this->pLinksInfo = (PFILE_LINKS_INFORMATION)LocalReAlloc(this->pLinksInfo, size, GPTR | GMEM_MOVEABLE);
			if (this->pLinksInfo == NULL) {
				status = GetLastError();
				goto CleanUp;
			}
			NtStatus = NtQueryInformationFile(
				hFile,
				&block,
				this->pLinksInfo,
				size,
				FileHardLinkInformation
			);
			i++;
		} while ((NtStatus == STATUS_BUFFER_OVERFLOW || NtStatus == STATUS_BUFFER_TOO_SMALL || NtStatus == STATUS_INFO_LENGTH_MISMATCH)&&i < 10);
	}
	status = NtStatusHandler(NtStatus);
CleanUp:
	if (hFile != NULL) {
		CloseHandle(hFile);
	}


	return status;
}

//DWORD PEFileInfo::Parse() {
//	return 0;
//}

#include <fstream>
#include <iostream>
void PrefetchFile::_process_section_a() {
	if (_a_index < _num_entries_a) {
		WCHAR _filename[1024] = { 0 };
		if (this->_format_version == 17) {
			uint32_t offset = this->_section_a + _a_index * 20;
			_curMetrics._a_start_time = MPEBytes::BytesToINT32L(this->_bytes.first + offset);
			_curMetrics._a_duration = MPEBytes::BytesToINT32L(this->_bytes.first + offset + 4);
			_curMetrics._a_average_duration_s = "";
			_curMetrics._a_filename_offset = MPEBytes::BytesToINT32L(this->_bytes.first + offset + 8);
			_curMetrics._a_filename_nb_char = MPEBytes::BytesToINT32L(this->_bytes.first + offset + 12);
			CopyMemory(_filename, this->_bytes.first + this->_section_c + _curMetrics._a_filename_offset, _curMetrics._a_filename_nb_char*sizeof(wchar_t));
			_curMetrics.filename = _filename;
			
		}
		else {
			uint32_t offset = this->_section_a + _a_index * 32;
			_curMetrics._a_start_time = MPEBytes::BytesToINT32L(this->_bytes.first + offset);
			_curMetrics._a_duration = MPEBytes::BytesToINT32L(this->_bytes.first + offset + 4);
			_curMetrics._a_average_duration_s = MPEBytes::BytesToINT32L(this->_bytes.first + offset + 8);
			_curMetrics._a_filename_offset = MPEBytes::BytesToINT32L(this->_bytes.first + offset + 12);
			_curMetrics._a_filename_nb_char = MPEBytes::BytesToINT32L(this->_bytes.first + offset + 16);
			CopyMemory(_filename, this->_bytes.first + this->_section_c + _curMetrics._a_filename_offset, _curMetrics._a_filename_nb_char*sizeof(wchar_t));
			_curMetrics.filename = _filename;
		}
		_a_index++;
	}

}

void PrefetchFile::_process_section_c()
{
}


PrefetchFile* PrefetchFile::Open(std::wstring file, bool is_compressed) {
	PrefetchFile* result = NULL;
	try {
		result = new PrefetchFile(file);
	}
	catch (std::exception ex) {
		delete result;
		return NULL;
	}

	return result;
}

PrefetchFile::PrefetchFile(std::wstring& file) : FileInfo(file) {
	auto f = GTFileUtils::Open(file.c_str(), L"r");

	if (f == NULL) {
		throw std::exception();
	}

	this->f = f;
	this->_a_index = 0;
}

pRtlDecompressBufferEx RtlDecompressBufferEx = NULL;
pRtlGetCompressionWorkSpaceSize RtlGetCompressionWorkSpaceSize = NULL;
DWORD PrefetchFile::Parse() {
	bool is_compressed = true;
	this->_bytes = f->ReadAll();
	if (memcmp(this->_bytes.first, "MAM\x04", 4) == 0) {
		is_compressed = true;
	}
	else {
		is_compressed = false;
	}
	NTSTATUS status = 0;
	if (is_compressed) {
		if (RtlDecompressBufferEx == NULL) {
			RtlDecompressBufferEx = (pRtlDecompressBufferEx)GetNativeProc("RtlDecompressBufferEx");
			if (RtlDecompressBufferEx == NULL) {
				return GetLastError();
			}
		}
		BytesBuffer header = NewBytesBuffer(this->_bytes.first, 8);
		BytesBuffer content = NewBytesBuffer(this->_bytes.first + 8, _bytes.second - 8);
		uint32_t signature, uncompressed_size = 0;
		signature = MPEBytes::BytesToINT32L((PBYTE)header.data());
		uncompressed_size = MPEBytes::BytesToINT32L((PBYTE)(header.data() + 4));
		uint16_t algo = (signature & 0x0f000000) >> 24;
		PBYTE uncompressed_buffer = (PBYTE)LocalAlloc(GPTR, uncompressed_size);
		if (uncompressed_buffer == NULL) {
			return GetLastError();
		}
		if (RtlGetCompressionWorkSpaceSize == NULL) {
			RtlGetCompressionWorkSpaceSize = (pRtlGetCompressionWorkSpaceSize)GetNativeProc("RtlGetCompressionWorkSpaceSize");
			if (RtlGetCompressionWorkSpaceSize == NULL) {
				return GetLastError();
			}
		}

		ULONG cbuf_work_size = 0;
		ULONG cfrag_work_size = 0;
		status = RtlGetCompressionWorkSpaceSize(algo, &cbuf_work_size, &cfrag_work_size);
		if (status != 0) {
			LocalFree(uncompressed_buffer);
			return NtStatusHandler(status);
		}
		PBYTE workspace = (PBYTE)LocalAlloc(GPTR, cfrag_work_size * sizeof(BYTE));
		if (workspace == NULL) {
			LocalFree(uncompressed_buffer);
			return GetLastError();
		}
		ULONG final_uncom_size = 0;
		status = RtlDecompressBufferEx(
			algo,
			uncompressed_buffer,
			uncompressed_size,
			(PBYTE)content.data(),
			(ULONG)content.size(),
			&final_uncom_size,
			workspace
		);
		if (status != 0) {
			LocalFree(uncompressed_buffer);
			LocalFree(workspace);
			return NtStatusHandler(status);
		}
		LocalFree(this->_bytes.first);
		this->_bytes.first = uncompressed_buffer;
		this->_bytes.second = final_uncom_size;
	}
	this->_format_version = MPEBytes::BytesToINT32L(this->_bytes.first);
	this->_file_size = MPEBytes::BytesToINT32L(this->_bytes.first + 0x0c);
	this->_exec_name = NewBytesBuffer(this->_bytes.first + 0x0010, 60);
	this->_prefetch_hash = NewBytesBuffer(this->_bytes.first + 0x004c, 4);
	this->_section_a = MPEBytes::BytesToINT32L(this->_bytes.first + 0x54);
	this->_num_entries_a = MPEBytes::BytesToINT32L(this->_bytes.first + 0x58);
	this->_section_b = MPEBytes::BytesToINT32L(this->_bytes.first + 0x005c);
	this->_num_entries_b = MPEBytes::BytesToINT32L(this->_bytes.first + 0x0060);
	this->_section_c = MPEBytes::BytesToINT32L(this->_bytes.first + 0x0064);
	this->_length_c = MPEBytes::BytesToINT32L(this->_bytes.first + 0x0068);
	this->_section_d = MPEBytes::BytesToINT32L(this->_bytes.first + 0x006c);
	this->_num_entries_d = MPEBytes::BytesToINT32L(this->_bytes.first + 0x0070);
	this->_length_d = MPEBytes::BytesToINT32L(this->_bytes.first + 0x0074);
	if (this->_format_version == 17) {
		this->_latest_exec_date = NewBytesBuffer(this->_bytes.first + 0x0078, 8);
		this->_exec_count = MPEBytes::BytesToINT32L(this->_bytes.first + 0x0090);
	}
	else if (this->_format_version == 23) {
		this->_latest_exec_date = NewBytesBuffer(this->_bytes.first + 0x0080, 8);
		this->_exec_count = MPEBytes::BytesToINT32L(this->_bytes.first + 0x0090);
	}
	else if (this->_format_version == 26) {
		this->_latest_exec_date = NewBytesBuffer(this->_bytes.first + 0x0080, 64);
		this->_exec_count = MPEBytes::BytesToINT32L(this->_bytes.first + 0x00d0);
	}
	else if (this->_format_version == 30) {
		this->_latest_exec_date = NewBytesBuffer(this->_bytes.first + 0x0080, 64);
		this->_exec_count = MPEBytes::BytesToINT32L(this->_bytes.first + 0x00c8);
	}
	return 0;
}

FileMetrics& PrefetchFile::NextFileMetrics() {
	_process_section_a();
	return _curMetrics;
}

std::vector<GTTime> PrefetchFile::GetExecTime() {
	std::vector<GTTime> ts;
	if (this->_latest_exec_date.size() == 64) {
		FILETIME t[8];
		CopyMemory(t, this->_latest_exec_date.data(), 64);
		for (int i = 0; i < 8; i++) {
			ts.push_back(GTTime(t[i]));
		}
	}
	else {
		FILETIME t;
		CopyMemory(&t, this->_latest_exec_date.data(), 8);
		ts.push_back(GTTime(t));
	}
	return ts;
}

bool PrefetchFile::HasMoreFileMetrics() {
	return this->_a_index < this->_num_entries_a;
}

PrefetchFile::~PrefetchFile() {
	if (f != NULL) {
		delete f;
	}

	if (_bytes.first != NULL) {
		delete _bytes.first;
	}

}

EvtxFile* EvtxFile::Open(std::wstring file) {
	EvtxFile* result = NULL;
	try {
		result = new EvtxFile(file);
	}
	catch (std::exception ex) {
		delete result;
		return NULL;
	}

	return result;
}

EvtxFile::EvtxFile(std::wstring& file) {
	this->f = GTFileUtils::Open(file.c_str(), L"r");
	if (this->f == NULL) {
		throw std::exception();
	}
	this->chk_index = 0;
}

EvtxFile::EvtxFile()
{
}


DWORD EvtxFile::Parse() {
	auto bytes = this->f->ReadAll();
	this->bytes = bytes.first;
	this->signature = NewBytesBuffer(bytes.first, 8);
	this->first_chunk_number = NewBytesBuffer(bytes.first + 8, 8);
	this->last_chunk_number = NewBytesBuffer(bytes.first + 16, 8);
	this->next_record_id = NewBytesBuffer(bytes.first + 24, 8);
	this->header_size = MPEBytes::BytesToINT32L(bytes.first + 32);
	this->minor_format_version = MPEBytes::BytesToINT16L(bytes.first + 36);
	this->major_format_version = MPEBytes::BytesToINT16L(bytes.first + 38);
	this->header_block_size = MPEBytes::BytesToINT16L(bytes.first + 40);
	this->number_of_chunks = MPEBytes::BytesToINT16L(bytes.first + 42);
	this->file_flags = MPEBytes::BytesToINT32L(bytes.first + 120);
	this->checksum = MPEBytes::BytesToINT32L(bytes.first + 124);
	return 0;
}

EvtxChunk& EvtxFile::NextChunk() {
	if (this->chk_index < this->number_of_chunks) {
		this->curChunk = EvtxChunk(this->bytes, this->chk_index * 0x10000 + 0x1000);
		this->chk_index++;
	}
	return this->curChunk;
}

EvtxFile::~EvtxFile() {

}


EvtxChunk::EvtxChunk(PBYTE bytes, uint32_t offset) {
	this->cur_record_offset = 0x200;
	this->evtx_bytes = bytes;
	this->this_offset = offset;
	PBYTE chunk = bytes + offset;
	auto s = GTFileUtils::Open(L"./test.chunk",L"w");
	s->WriteBytes(0, chunk, 0x2000);
	this->signature = NewBytesBuffer(chunk, 8);
	this->first_event_record_number = MPEBytes::BytesToINT64L(chunk + 8);
	this->last_event_record_number = MPEBytes::BytesToINT64L(chunk + 16);
	this->first_event_record_id = MPEBytes::BytesToINT64L(chunk + 24);
	this->last_event_record_id = MPEBytes::BytesToINT64L(chunk + 32);
	this->header_size = MPEBytes::BytesToINT32L(chunk + 40);
	this->last_event_record_offset = MPEBytes::BytesToINT32L(chunk + 44);
	this->free_space_offset = MPEBytes::BytesToINT32L(chunk + 48);
	this->checksum = MPEBytes::BytesToINT32L(chunk + 52);
	this->checksum = MPEBytes::BytesToINT32L(chunk + 124);
}

EvtxChunk::EvtxChunk()
{
}

EvtxEventRecord& EvtxChunk::NextRecord() {
	if (this->cur_record_offset != this->last_event_record_offset) {
		this->curRecord = EvtxEventRecord(this->evtx_bytes, this->this_offset + cur_record_offset);
		this->cur_record_offset = cur_record_offset + curRecord.GetSize();
	}
	return this->curRecord;
}

bool EvtxChunk::HasMoreRecords() {
	return this->cur_record_offset <= this->last_event_record_offset;
}


EvtxEventRecord::EvtxEventRecord(PBYTE bytes, uint32_t offset) {
	this->evtx_bytes = bytes;
	PBYTE bs = bytes + offset;
	this->signature = NewBytesBuffer(bs,4);
	this->size = MPEBytes::BytesToINT32L(bs + 4);
	this->event_record_id = MPEBytes::BytesToINT64L(bs + 8);
	this->written_date = MPEBytes::BytesToINT32L(bs + 16);
	this->event_offset = offset + 24;
	this->body.first = bs + 24;
	this->body.second = this->size - 28;

}

EvtxEventRecord::EvtxEventRecord() {

}



uint32_t EvtxEventRecord::GetSize() {
	return this->size;
}

GTString EvtxEventRecord::GetXML() {
	return GTString();
}

BytesBuffer EvtxEventRecord::GetBody() {
	return NewBytesBuffer(this->body.first, this->body.second);
}

FileMetrics::FileMetrics() {
}


GTString BinaryXML::String() {
	return GTString();
}
