#include "NativeModules.h"
#include "Process.h"
#include "StringUtils.h"
#include "Service.h"
#include "RegistryUtils.h"
#include "shlwapi.h"
#include <tinyxml2.h>
#include "Network.h"
#include "FireWallInfo.h"
ProcessModule::ProcessModule() {
	this->Name = L"Process";
	this->Path = L"Process";
	this->Type = L"Native";
	this->Class = L"GetInfo";
	this->Description = L"Get Process Infomation";
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
}

ResultSet* ProcessModule::ModuleRun() {
	ResultSet* result = new ResultSet();
	std::map<PID, bool> trustMap;
	ProcessManager* mgr = new ProcessManager();
	SystemInfo info;
	auto time1 = info.GetSystemTimeInfo();
	mgr->SetAllProcesses();
	for (auto item : mgr->processesMap) {
		if (item.second->GetImageState()->IsSigned()) {
			trustMap[item.first] = true;
		}
		else {
			trustMap[item.first] = false;
		}
	}
	Sleep(1000);
	for (auto item : mgr->processesMap) {
		result->PushDictOrdered("PID", std::to_string(item.first));
		result->PushDictOrdered("PPID", std::to_string(item.second->parentPID));
		result->PushDictOrdered("Name", StringUtils::ws2s(item.second->GetProcessName()));
		auto process = item.second;
		process->UpdateInfo();
		auto time2 = info.GetSystemTimeInfo();
		auto latest = process->latestCpuState;
		auto old = process->cpuState;
		INT64 p_ktime = (latest->kernelTime.dwLowDateTime - old->kernelTime.dwLowDateTime) + ((latest->kernelTime.dwHighDateTime - old->kernelTime.dwHighDateTime) << 32);
		INT64 p_utime = (latest->userTime.dwLowDateTime - old->userTime.dwLowDateTime) + ((latest->userTime.dwHighDateTime - old->userTime.dwHighDateTime) << 32);
		INT64 s_ktime = (time2.kernelTime.dwLowDateTime - time1.kernelTime.dwLowDateTime) + ((time2.kernelTime.dwHighDateTime - time1.kernelTime.dwHighDateTime) << 32);
		INT64 s_utime = (time2.userTime.dwLowDateTime - time1.userTime.dwLowDateTime) + ((time2.userTime.dwHighDateTime - time1.userTime.dwHighDateTime) << 32);
		FLOAT percent = (float)(p_ktime + p_utime) * 100 / (float)(s_ktime + s_utime);
		result->PushDictOrdered("CPU", std::to_string(percent));
		result->PushDictOrdered("User name", StringUtils::ws2s(item.second->UserName()));
		result->PushDictOrdered("Command line", StringUtils::ws2s(item.second->GetImageState()->cmdline));
		result->PushDictOrdered("File path", StringUtils::ws2s(item.second->GetImageState()->imageFileName));
		result->PushDictOrdered("Start time", StringUtils::ws2s(item.second->GetStartTime().String_utc_to_local()));
		auto memory = item.second->GetMemoryState();
		result->PushDictOrdered("Private bytes", std::to_string(memory->PrivateWorkingSetSize));
		result->PushDictOrdered("Working set", std::to_string(memory->WorkingSetSize));
		if (trustMap.contains(item.first)) {
			if (trustMap[item.first]) {
				result->PushDictOrdered("Verified", "True");
			}
			else {
				result->PushDictOrdered("Verified", "False");
			}
		}
		else {
			result->PushDictOrdered("Verified", "");
		}

		try {
			result->PushDictOrdered("File Hash", item.second->GetImageState()->GetMd5Hash());
		}
		catch (...) {
			result->PushDictOrdered("File Hash", "");
		}
		
	}
	delete mgr;
	result->SetType(DICT);
	return result;
}

ListTestModule::ListTestModule() {
	this->Name = L"ListTest";
	this->Path = L"Test";
	this->Type = L"Native";
	this->Class = L"GetInfo";
	this->Description = L"Test List";
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
}

ResultSet* ListTestModule::ModuleRun() {
	ResultSet* result = new ResultSet();
	result->dataArray = { "abc","def","ghi" };
	result->SetType(ARRAY);
	return result;
}

ServiceModule::ServiceModule() {
	this->Name = L"Services";
	this->Path = L"Service";
	this->Type = L"Native";
	this->Class = L"GetInfo";
	this->Description = L"Get Service Info";
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
}

ResultSet* ServiceModule::ModuleRun() {
	ResultSet* result = new ResultSet();
	ServiceManager* mgr = new ServiceManager();
	mgr->SetAllServices();
	for (auto item : mgr->services) {
		result->PushDictOrdered("serviceName", StringUtils::ws2s(item->GetServiceName()));
		auto userName = item->GetServiceName();
		result->PushDictOrdered("userName", StringUtils::ws2s(userName));
		result->PushDictOrdered("serviceStatus", StringUtils::ws2s(item->GetServiceStatus()));
		auto path = item->GetFilePath();
		result->PushDictOrdered("path", StringUtils::ws2s(path));
		auto description = item->GetDescription();
		result->PushDictOrdered("description", StringUtils::ws2s(description));
		auto pid = item->GetOwningPid();
		result->PushDictOrdered("owningPid", std::to_string(pid));
	}
	delete mgr;
	result->SetType(DICT);
	return result;
}

StartupModule::StartupModule() {
	this->Name = L"Startup";
	this->Path = L"Other";
	this->Type = L"Native";
	this->Class = L"GetInfo";
	this->Description = L"Get Startup Programs";
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
}

ResultSet* StartupModule::ModuleRun() {
	ResultSet* result = new ResultSet();

	do {
		HANDLE hFind = INVALID_HANDLE_VALUE;
		LARGE_INTEGER filesize;
		std::wstring path;
		path = std::wstring(L"C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\");

		if (PathFileExistsW(path.c_str()) == FALSE) {
			//wprintf(L"The Startup in menu is not exist");
			break;
		}
		path = path + L"*";
		WIN32_FIND_DATAW ffd;
		hFind = FindFirstFileW(path.c_str(), &ffd);

		if (INVALID_HANDLE_VALUE == hFind) {
			LOG_DEBUG_REASON(L"Can not find first file");
			break;
		}

		// List all the files in the directory with some info about them.

		do
		{
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				//wprintf(L"\t%s   <DIR>\n", ffd.cFileName);
			}
			else
			{
				filesize.LowPart = ffd.nFileSizeLow;
				filesize.HighPart = ffd.nFileSizeHigh;
				//wprintf(L"\t%s   %ld bytes\n", ffd.cFileName, filesize.QuadPart);
				//result->dataDict["fileName"].push_back(StringUtils::ws2s(ffd.cFileName));
				result->PushDictOrdered("fileName", StringUtils::ws2s(ffd.cFileName));
				//result->dataDict["source"].push_back(StringUtils::ws2s(path));
				result->PushDictOrdered("source", StringUtils::ws2s(path));
				//result->dataDict["cmdline"].push_back(StringUtils::ws2s(ffd.cFileName));
				result->PushDictOrdered("cmdline", StringUtils::ws2s(ffd.cFileName));
			}
		} while (FindNextFileW(hFind, &ffd) != 0);
	} while (0);

	do {
		HANDLE hFind = INVALID_HANDLE_VALUE;
		WCHAR appdataPath[101];
		LARGE_INTEGER filesize;
		GetEnvironmentVariableW(L"appdata", appdataPath, 99);
		if (lstrlenW(appdataPath) == 0) {
			break;
		}
		std::wstring path;
		path = appdataPath + std::wstring(L"\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\");

		if (PathFileExistsW(path.c_str()) == FALSE) {
			//wprintf(L"The Startup in menu is not exist");
			break;
		}
		path = path + L"*";
		WIN32_FIND_DATAW ffd;
		hFind = FindFirstFileW(path.c_str(), &ffd);

		if (INVALID_HANDLE_VALUE == hFind) {
			LOG_DEBUG_REASON(L"Can not find first file");
			break;
		}

		// List all the files in the directory with some info about them.

		do
		{
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				//wprintf(L"\t%s   <DIR>\n", ffd.cFileName);
			}
			else
			{
				filesize.LowPart = ffd.nFileSizeLow;
				filesize.HighPart = ffd.nFileSizeHigh;
				//wprintf(L"\t%s   %ld bytes\n", ffd.cFileName, filesize.QuadPart);
				//result->dataDict["fileName"].push_back(StringUtils::ws2s(ffd.cFileName));
				result->PushDictOrdered("fileName", StringUtils::ws2s(ffd.cFileName));
				result->PushDictOrdered("source", StringUtils::ws2s(path));
				result->PushDictOrdered("cmdline", StringUtils::ws2s(ffd.cFileName));
			}
		} while (FindNextFileW(hFind, &ffd) != 0);
	} while (0);

	do {
		RegistryUtils utils(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
		auto a = utils.ListKeyValue();
		for (auto i : a) {
			result->PushDictOrdered("fileName",StringUtils::ws2s(i.first));
			result->PushDictOrdered("cmdline",StringUtils::ws2s(i.second));
			result->PushDictOrdered("source","HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
		}
	} while (0);

	std::vector<LPWSTR> startupKeys = {
		(LPWSTR)L"HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
		(LPWSTR)L"HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
		(LPWSTR)L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
		(LPWSTR)L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
		(LPWSTR)L"HKEY_CURRENT_USER\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\Run",
		(LPWSTR)L"HKEY_CURRENT_USER\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\RunOnce"
	};
	do {
		for (LPWSTR key : startupKeys) {
			RegistryUtils utils(key);
			auto items = utils.ListKeyValue();
			for (auto item : items) {
				result->PushDictOrdered("fileName",StringUtils::ws2s(item.first));
				result->PushDictOrdered("cmdline",StringUtils::ws2s(item.second));
				result->PushDictOrdered("source",StringUtils::ws2s(key));
			}
		}
	} while (0);
	result->SetType(DICT);
	return result;
}

FilesRelateOpenCommandsModule::FilesRelateOpenCommandsModule() {
	this->Name = L"FilesRelate";
	this->Path = L"Registry";
	this->Type = L"Native";
	this->Class = L"GetInfo";
	this->Description = L"Get files relate open programs";
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
}

ResultSet* FilesRelateOpenCommandsModule::ModuleRun() {
	ResultSet* result = new ResultSet();
	RegistryUtils utils(L"HKEY_CLASSES_ROOT");
	auto subkeys = utils.ListSubKeys();
	for (auto subkey : subkeys) {
		if (StringUtils::HasEnding(subkey, L"file")) {
			std::wstring key = L"HKEY_CLASSES_ROOT\\" + subkey + L"\\shell\\open\\command";
			auto a = RegistryUtils::GetValueStatic(key.c_str(), L"");
			if (a.size() == 0) {
				continue;
			}
			result->PushDictOrdered("file",StringUtils::ws2s(subkey));
			result->PushDictOrdered("program",StringUtils::ws2s((LPWSTR)a.c_str()));
		}
	}
	result->SetType(DICT);
	return result;
}

NetworkModule::NetworkModule() {
	this->Name = L"NetworkConnection";
	this->Path = L"Network";
	this->Type = L"Native";
	this->Class = L"GetInfo";
	this->Description = L"Get Network Connection";
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
}

ResultSet* NetworkModule::ModuleRun() {
	ResultSet* result = new ResultSet();
	auto inst = DnsCache::GetInstance();
	NetworkManager mgr;
	mgr.SetTCPConnection();
	ProcessManager proMgr;
	proMgr.UpdateInfo();
	for (auto connection : mgr.connections) {
		auto domain = inst->GetDomain(connection->GetRemoteIPAsString().c_str());
		if (domain == NULL) {
			domain = L"";
		}
		result->PushDictOrdered("local", StringUtils::ws2s(connection->GetLocalIPAsString().c_str()) + ":" + std::to_string(connection->localPort));
		result->PushDictOrdered("remote", StringUtils::ws2s(connection->GetRemoteIPAsString().c_str()) + ":" + std::to_string(connection->remotePort));
		result->PushDictOrdered("domain", StringUtils::ws2s(domain));
		result->PushDictOrdered("state", StringUtils::ws2s(connection->GetStateAsString().c_str()));
		result->PushDictOrdered("pid", std::to_string(connection->owningPid));
		result->PushDictOrdered("process name", StringUtils::ws2s(proMgr.processesMap[connection->owningPid]->GetProcessName()));
		
	}
	result->SetType(DICT);
	return result;
}

Rundll32Backdoor::Rundll32Backdoor() {
	this->Name = L"Rundll32Backdoor";
	this->Path = L"Process";
	this->Type = L"Native";
	this->Class = L"GetInfo";
	this->Description = L"Get Rundll32 Backdoor";
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
}

ResultSet* Rundll32Backdoor::ModuleRun() {
	ResultSet* result = new ResultSet();
	ProcessManager mgr;
	mgr.UpdateInfo();
	for (auto item : mgr.processesMap) {
		auto proName = item.second->processName;
		auto iproName = StringUtils::ToLower(proName);
		if (iproName.find(L"rundll") != -1) {
			Process* process = item.second;
			auto imageState = process->GetImageState();
			std::wstring cmdline = imageState->cmdline;
			result->PushDictOrdered("pid",std::to_string(item.first));
			result->PushDictOrdered("cmdline", StringUtils::ws2s(cmdline).c_str());
			result->report = "Might have rundll32 backdoor";
		}
	}
	result->SetType(DICT);
	return result;
}

ShadowAccount::ShadowAccount() {
	this->Name = L"ShadowAccount";
	this->Path = L"Account";
	this->Type = L"Native";
	this->Class = L"GetInfo";
	this->Description = L"Get Shadow Account";
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
}
#include "AccountInfo.h"
ResultSet* ShadowAccount::ModuleRun() {
	ResultSet* result = new ResultSet();
	GTPrintln(L"Shadow Account Backddor:");
	//AccountInfoManager mgr;
	//mgr.Initialize();
	//auto users = mgr.GetAccountList();
	//for (auto user : users) {
	//	if (StringUtils::HasEnding(user->userName, L"$")) {
	//		//GTPrintln(L"\t%s", user->userName.c_str());
	//		result->PushDictOrdered("username", StringUtils::ws2s(user->userName));
	//		result->report = "Might have shadow account";
	//	}
	//}
	result->SetType(DICT);
	return result;
}

USBHistory::USBHistory() {
	this->Name = L"USBHistory";
	this->Path = L"Registry";
	this->Type = L"Native";
	this->Class = L"GetInfo";
	this->Description = L"Get USB History";
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
}

ResultSet* USBHistory::ModuleRun() {
	ResultSet* result = new ResultSet();
	std::wstring key = L"HKEY_LOCAL_MACHINE\\SYSTEM\\";
	std::wstring timeKey = L"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\DeviceClasses\\{53f56307-b6bf-11d0-94f2-00a0c91efb8b}\\";
	RegistryUtils utils(timeKey.c_str());
	auto subkeys = utils.ListSubKeysChain();
	for (auto& subkey : subkeys) {
		auto time = subkey.GetLastWriteTime();
		//wprintf(L"%s %s\n", subkey.GetKeyName().c_str(), GTTime(time).ToString().c_str());
		result->PushDictOrdered("Device Name", StringUtils::ws2s(subkey.GetKeyName()));
		result->PushDictOrdered("Time", StringUtils::ws2s(GTTime(time).String_utc_to_local()));
	}
	result->SetType(DICT);
	return result;
}

PrefetchModule::PrefetchModule() {
	this->Name = L"Prefetch";
	this->Path = L"File";
	this->Type = L"Native";
	this->Class = L"GetInfo";
	this->Description = L"Get Prefetch files";
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
}

ResultSet* PrefetchModule::ModuleRun() {
	return nullptr;
}

ListSchduleTask::ListSchduleTask() {
	this->Name = L"ListSchduleTask";
	this->Path = L"Other";
	this->Type = L"Native";
	this->Class = L"GetInfo";
	this->Description = L"Get Prefetch files";
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
}
#include "OtherInfo.h"
ResultSet* ListSchduleTask::ModuleRun() {
	ResultSet* result = new ResultSet();
	SchduleTaskMgr* mgr = SchduleTaskMgr::GetMgr();
	auto tasks = mgr->GetTasks();
	for (auto& task : tasks) {
		result->PushDictOrdered("Name", StringUtils::ws2s(task.getName()));
		result->PushDictOrdered("State", StringUtils::ws2s(task.GetState()));
		result->PushDictOrdered("Exec", StringUtils::ws2s(task.GetExec()));
		result->PushDictOrdered("Path", StringUtils::ws2s(task.getPath()));
	}
	result->SetType(DICT);
	delete mgr;
	return result;
}

#include <thread>
#include <chrono>
WatchNetstat::WatchNetstat() {
	this->Name = L"WatchNetstat";
	this->Path = L"Network";
	this->Type = L"LastMode";
	this->Class = L"GetInfo";
	this->Description = L"Read Network stat as quick as possiable";
	this->RunType = ModuleNotAuto;
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
}
BOOL WINAPI consoleHandler(DWORD signal) {
	if (signal == CTRL_C_EVENT) {
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, 0xc);
		exit(0);
	}

	return TRUE;
}

std::vector<std::pair<bool, Connection>> _what_is_second_doesnot_have(std::vector<Connection>& first, std::vector<Connection>& second) {
	std::vector<std::pair<bool,Connection>> res;
	bool flag = false;
	for (auto &s : second) {
		flag = false;
		for (auto &f : first) {
			if (f == s) {
				flag = true;
				break;
			}
			
		}

		if (flag == false) {
			res.push_back(std::pair(true, s));
		}
	}

	for (auto &f : first) {
		flag = false;
		for (auto &s : second) {
			if (f == s) {
				flag = true;
				break;
			}

		}

		if (flag == false) {
			res.push_back(std::pair(false, f));
		}
	}

	return res;
}

std::vector<Connection> _copy(std::vector<Connection*> vs) {
	std::vector<Connection> res;
	for (auto s : vs) {
		Connection c = *s;
		res.push_back(c);
	}

	return res;
}
#include "NetworkUtils.h"
ResultSet* WatchNetstat::ModuleRun() {
	bool running = TRUE;
	if (!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
		return nullptr;
	}
	ProcessManager proMgr;
	proMgr.UpdateInfo();
	NetworkManager mgr;
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	std::vector<Connection> last = _copy(mgr.GetAllConnections());
	for (auto& change : last) {
		GTWString ip = change.GetRemoteIPAsString();
		auto inst = DnsCache::GetInstance();
		LPCWSTR domain = inst->GetDomain(ip.c_str());
		//LPCSTR domain = NULL;
		if (domain == NULL) {
			domain = L"-";
		}
		GTWString protocol = L"";
		if (change.protocol == Protocol::UDP) {
			protocol = L"UDP";
		}
		else {
			protocol = L"TCP";
		}
		wprintf(L"[Base]%s %s:%d %s:%d/%s [%d] %s\n", protocol.c_str(), change.GetLocalIPAsString().c_str(),
				change.localPort,
				change.GetRemoteIPAsString().c_str(), change.remotePort,
			domain,
			change.owningPid, proMgr.processesMap[change.owningPid]->GetProcessName().c_str());
	}
	std::vector<Connection> conns;
	UINT64 i = 0;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	auto inst = DnsCache::GetInstance();
	for (; ;i++) {
		//Sleep(50);
		if (i % 32 == 0) {
			inst->Update();
		}
		std::map<PID, GTWString> procsMap ;
		conns = _copy(mgr.GetAllConnections());
		auto changes = _what_is_second_doesnot_have(conns, last);
		if (changes.size() > 0) {
			procsMap = proMgr.GetProcesses_Light();
		}
		//print changes
		for (auto& change : changes) {
			GTWString protocol = L"";
			if (change.second.protocol == Protocol::UDP) {
				protocol = L"UDP";
			}
			else {
				protocol = L"TCP";
			}
			SYSTEMTIME st;
			GetLocalTime(&st); 
			GTTime t = GTTime::GetTime();
			GTWString ip = change.second.GetRemoteIPAsString();
			auto inst = DnsCache::GetInstance();
			LPCWSTR domain = inst->GetDomain(ip.c_str());
			
			//LPCSTR domain = NULL;
			if (domain == NULL) {
				domain = L"-";
			}
			if (change.first == false) {
				LPCWSTR name = L"";
				if (procsMap.contains(change.second.owningPid)) {
					name = procsMap[change.second.owningPid].c_str();
				}
				if (proMgr.processesMap[change.second.owningPid] != NULL) {
					SetConsoleTextAttribute(hConsole, 0xc);
					wprintf(L"[-]%s %s:%d %s:%d/%s %s [%d] %s %s\n", 
						protocol.c_str(),
						change.second.GetLocalIPAsString().c_str(),
						change.second.localPort,
						change.second.GetRemoteIPAsString().c_str(), change.second.remotePort,
						domain,
						change.second.GetStateAsString().c_str(),
						change.second.owningPid, 
						name,
						t.String().c_str());
				}
				else {
					SetConsoleTextAttribute(hConsole, 0xc);
					wprintf(L"[-]%s %s:%d %s:%d/%s %s [%d] %s %s\n",
						protocol.c_str(),
						change.second.GetLocalIPAsString().c_str(),
						change.second.localPort,
						change.second.GetRemoteIPAsString().c_str(), change.second.remotePort,
						domain,
						change.second.GetStateAsString().c_str(),
						change.second.owningPid,
						L"",
						t.String().c_str());
				}
			}
			else {
				LPCWSTR name = L"";
				if (procsMap.contains(change.second.owningPid)) {
					name = procsMap[change.second.owningPid].c_str();
				}
				if (proMgr.processesMap[change.second.owningPid] != NULL) {
					SetConsoleTextAttribute(hConsole, 0xa);
					wprintf(L"[+]%s %s:%d %s:%d/%s %s [%d] %s %s\n",
						protocol.c_str(),
						change.second.GetLocalIPAsString().c_str(),
						change.second.localPort,
						change.second.GetRemoteIPAsString().c_str(), change.second.remotePort,
						domain,
						change.second.GetStateAsString().c_str(),
						change.second.owningPid,
						name,
						t.String().c_str());
				}
				else {
					SetConsoleTextAttribute(hConsole, 0xa);
					wprintf(L"[+]%s %s:%d %s:%d/%s %s [%d] %s %s\n",
						protocol.c_str(),
						change.second.GetLocalIPAsString().c_str(),
						change.second.localPort,
						change.second.GetRemoteIPAsString().c_str(), change.second.remotePort,
						domain,
						change.second.GetStateAsString().c_str(),
						change.second.owningPid,
						L"",
						t.String().c_str());
				}
			}
		}
		last = conns;
	}
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "[s]" << std::endl;
	std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() << "[ns]" << std::endl;
	return nullptr;
}

MailiousProcessDlls::MailiousProcessDlls() {
	this->Name = L"UnsignedProcessDlls";
	this->Path = L"Process";
	this->Type = L"Native";
	this->Class = L"GetInfo";
	this->RunType = ModuleNeedArgs;
	this->Description = L"List a Dlls of Process that not signed by trust provider";
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
}

ResultSet* MailiousProcessDlls::ModuleRun() {
	ResultSet* result = new ResultSet();
	std::vector<UINT32> pids;
	if (!this->args.contains("pid")) {
		//result->SetErrorMessage("Must set a pid to get dll information: ./GodAgent.exe Process.UnsignedProcessDlls 'pid=${pid}',running all processes");
		LOG_INFO(L"Must set a pid to get dll information: ./GodAgent.exe Process.UnsignedProcessDlls 'pid=${pid}',running all processes");
		this->args["pid"] = "*";
	}

	if (this->args["pid"] == "*") {
		auto mgr = ProcessManager::GetMgr();
		mgr->SetAllProcesses();
		for (auto pid : mgr->processesMap) {
			pids.push_back(pid.first);
		}
	}
	else {
		auto pid = stoi(this->args["pid"]);
		pids.push_back(pid);
	}

	
	for (auto pid : pids) {
		SetLastError(0);
		GTTime* t = NULL;
		if (this->args.contains("date")) {
			t = new GTTime(this->args["date"].c_str());
		}
		Process* p = NULL;
		p = new Process(pid);
		wprintf(L"Running Process %d %s\n", p->GetPID(), p->GetProcessName().c_str());
		if (p == NULL) {
			result->SetErrorMessage("Error: " + StringUtils::ws2s(GetLastErrorAsString()));
			return result;
		}
		auto dlls = p->GetLoadedDlls();
		if (dlls.size() == 0) {
			continue;
		}
		for (auto& dll : dlls) {
			auto path = dll.GetPath();
			auto sign = VerifyEmbeddedSignature(path.c_str());
			FileInfo dllInfo(path.c_str());
			if (!sign->IsSignature()) {
				result->PushDictOrdered("Pid", std::to_string(pid));
				result->PushDictOrdered("Path", StringUtils::ws2s(path));
				result->PushDictOrdered("Reason", "No signature");
				wprintf(L"\tUnsigned dlls %d %s\n", pid, path.c_str());
			}
			delete sign;
		}
		if (p != NULL) {
			delete p;
		}
	}
	ProcessManager::GetMgr()->~ProcessManager();
	result->SetType(DICT);
	return result;
}

MailiousCodeInjection::MailiousCodeInjection() {
	this->Name = L"MailiousCodeInjection";
	this->Path = L"Process";
	this->Type = L"Native";
	this->Class = L"GetInfo";
	this->Description = L"Detect is there are mailious shellcode";
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
}

ResultSet* MailiousCodeInjection::ModuleRun() {
	return nullptr;
}

ValidSvcHost::ValidSvcHost() {
	this->Name = L"ValidSvcHost";
	this->Path = L"Service";
	this->Type = L"Native";
	this->Class = L"GetInfo";
	this->Description = L"Detect if there are svchost that suspicious";
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
}

ResultSet* ValidSvcHost::ModuleRun() {
	return nullptr;
}

RecentRunning::RecentRunning() {
	this->Name = L"RecentRunning";
	this->Path = L"System";
	this->Type = L"Native";
	this->Class = L"GetInfo";
	this->Description = L"Get System recent running process";
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
}

std::wstring ROT13(std::wstring source) {
	std::wstring transformed;
	for (size_t i = 0; i < source.size(); ++i) {
		if ((source[i] >= L'a' && source[i] <= L'z') || (source[i] >= L'A' && source[i] <= L'Z')) {
			if (source[i] >= L'a' && source[i] <= L'z') {
				transformed.append(1, (((source[i] - L'a') + 13) % 26) + L'a');
			} else 
			if (source[i] >= L'A' && source[i] <= L'Z') {
				transformed.append(1, (((source[i] - L'A') + 13) % 26) + L'A');
			}
		}
		else {
			transformed.append(1, source[i]);
		}
	}
	return transformed;
}
#include "PrivilegeUtils.h"
#include "EvtInfo.h"
class ShellCore9707 {
public:
	GTString createTime;
	GTString commandLine;
	ShellCore9707(const wchar_t* xml);
};

ShellCore9707::ShellCore9707(const wchar_t* xml) {
	tinyxml2::XMLDocument doc;
	doc.Parse(StringUtils::ws2s(xml).c_str());
	auto root_element = doc.RootElement();
	auto system_element = root_element->FirstChildElement();
	auto next = system_element->FirstChildElement();
	auto user_element = system_element->NextSiblingElement();
	char* value = NULL;
	char* name = NULL;
	while (next) {
		value = (char*)next->GetText();
		name = (char*)next->Value();

		if (_strcmpi(name, "TimeCreated") == 0) {
			auto attr = next->FindAttribute("SystemTime");
			if (attr != NULL) {
				this->createTime = attr->Value();
			}
		}
		next = next->NextSiblingElement();
	}
	next = user_element->FirstChildElement();
	while (next != NULL) {
		value = (char*)next->GetText();
		name = (char*)next->Value();
		auto attr = next->FindAttribute("Name");
		if (attr != NULL && _strcmpi(attr->Value(), "Command") == 0) {
			this->commandLine = value;
		}

		next = next->NextSiblingElement();
	}
}

DWORD RecentRunningEventLog(Evt* evt, PVOID data) {
	auto evtXml = evt->GetXml();
	auto result = (std::vector<ShellCore9707*>*)data;
	auto xml = StringUtils::ws2s(evt->GetXml().c_str());
	tinyxml2::XMLDocument doc;
	auto error = doc.Parse(xml.c_str());
	auto root = doc.RootElement();
	auto system = root->FirstChildElement();
	tinyxml2::XMLElement* child_system_next = system->FirstChildElement();
	tinyxml2::XMLElement* userdata_next = system->NextSiblingElement();
	char* value = NULL;
	char* name = NULL;

	while (child_system_next) {
		value = (char*)child_system_next->GetText();
		name = (char*)child_system_next->Value();
		if (_strcmpi(name, "EventId") == 0) {
			auto event_id = std::stoi(value);
			if (event_id == 9707) {
				result->push_back(new ShellCore9707(evtXml.c_str()));
				break;
			}
		}
		child_system_next = child_system_next->NextSiblingElement();
	}
	return 0;
}

ResultSet* RecentRunning::ModuleRun() {
	ResultSet* result = new ResultSet();
	std::wstring alluserAssist = L"HKEY_USERS";
	RegistryUtils allUserAssistReg(alluserAssist);
	auto users = allUserAssistReg.ListSubKeys();
	for (auto& user : users) {
		if (user == L".DEFAULT") {
			continue;
		}

		//wprintf(L"%s %s\n", user.c_str(),ConvertSidToUsername(user.c_str()));
		std::wstring userAssist = L"HKEY_USERS\\" + user + L"\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\UserAssist";
		RegistryUtils userAssistReg(userAssist);
		auto subs = userAssistReg.ListSubKeys();
		std::map<std::wstring, UserAssistParser> assistMap;
		for (auto& sub : subs) {
			std::wstring key = userAssist + L"\\" + sub + L"\\count";
			RegistryUtils utils(key);
			auto names = utils.ListValueNames();
			for (auto& _name : names) {
				auto name = ROT13(_name);
				auto buffer = RegistryUtils::GetValueStatic(key.c_str(), _name.c_str());
				UserAssistParser parser(buffer);
				assistMap[name] = parser;
				result->PushDictOrdered("exec", StringUtils::ws2s(parser.GetLastRun()));
				result->PushDictOrdered("type", "UserAssist");
				result->PushDictOrdered("name", StringUtils::ws2s(name));
				result->PushDictOrdered("path", "");
				
			}
		}
	}

	GTDir d(L"C:\\Windows\\Prefetch");

	auto files = d.ListFiles();
	std::vector<std::wstring> pfs;
	for (auto& f : files) {
		if (f.ends_with(L".pf")) {
			pfs.push_back(f);
		}
	}
	files.clear();
	for (auto& pf : pfs) {
		auto s = L"C:\\Windows\\Prefetch\\" + pf;
		auto exeName = pf.substr(0, pf.find_last_of(L"-"));
		PrefetchFile* f = new PrefetchFile(s);
		f->Parse();
		GTWString path;
		while (f->HasMoreFileMetrics()) {
			auto metrics = f->NextFileMetrics();
			if (metrics.filename.ends_with(exeName)) {
				path = metrics.filename;
			}
		}
		//wprintf(L"%s\n", pf.c_str());
		auto times = f->GetExecTime();
		for (auto& time : times) {
			if (time.year < 1970) {
				continue;
			}
			//wprintf(L"\t%s\n", time.ToString().c_str());
			result->PushDictOrdered("exec", StringUtils::ws2s(time.String_utc_to_local()));
			result->PushDictOrdered("type", "Prefetch");
			result->PushDictOrdered("name", StringUtils::ws2s(exeName));
			result->PushDictOrdered("path", StringUtils::ws2s(path));
			
		}
		delete f;
	}
	EvtInfo info;
	EvtFilter filter;
	filter.ids = L"9707";
	filter.logName = L"Microsoft-Windows-Shell-Core/Operational";
	std::vector<ShellCore9707*> events;
	if (this->args.contains("path")) {
		info.EnumEventLogs(filter, RecentRunningEventLog, &events, false, (wchar_t*)StringUtils::s2ws(this->args["path"]).c_str());
	}
	else {
		info.EnumEventLogs(filter, RecentRunningEventLog, &events, false, NULL);
	}
	for (auto& e : events) {
		auto t = GTTime::FromISO8601(StringUtils::s2ws(e->createTime));
		result->PushDictOrdered("exec", StringUtils::ws2s(t.String_utc_to_local()));
		result->PushDictOrdered("type", "ShellCore-Run");
		result->PushDictOrdered("name", e->commandLine);
		result->PushDictOrdered("path", "");
		
	}
	
	result->SetType(DICT);
	return result;
}

MRUList::MRUList() {
	this->Name = L"MRUList";
	this->Path = L"Registry";
	this->Type = L"Native";
	this->Class = L"GetInfo";
	this->Description = L"Most Recent user list";
	this->RunType = ModuleNotImplement;
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
}

ResultSet* MRUList::ModuleRun() {
	return nullptr;
}

Accounts::Accounts() {
	this->Name = L"Accounts";
	this->Path = L"Account";
	this->Type = L"Native";
	this->Class = L"GetInfo";
	this->Description = L"Get Accounts";
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
}

ResultSet* Accounts::ModuleRun() {
	ResultSet* result = new ResultSet();
	/*AccountInfoManager mgr;
	mgr.Initialize();
	auto users = mgr.GetAccountList();
	for (auto user : users) {
		result->PushDictOrdered("Uid", std::to_string(user->userId));
		result->PushDictOrdered("Username", StringUtils::ws2s(user->userName));
		result->PushDictOrdered("LastLogon", StringUtils::ws2s(user->GetLastLogon().String_utc_to_local()));
		result->PushDictOrdered("LastLogoff", StringUtils::ws2s(user->GetLastLogoff().String_utc_to_local()));
		result->PushDictOrdered("Comment", StringUtils::ws2s(user->GetComment()));
		result->PushDictOrdered("LogonServer", StringUtils::ws2s(user->GetLogonServer()));
	}*/
	result->SetType(DICT);
	return result;
}

RecentApps::RecentApps() {
	this->Name = L"RecentApps";
	this->Path = L"Registry";
	this->Type = L"Native";
	this->Class = L"GetInfo";
	this->Description = L"Recent Apps";
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
}

ResultSet* RecentApps::ModuleRun() {
	
	auto s = L"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Search\\RecentApps";
	RegistryUtils utils(s);
	auto keys = utils.ListSubKeys();
	for (auto& key : keys) {
		std::wstring target = s + std::wstring(L"\\") + key;
		RegistryUtils key(target.c_str());
		auto ks = key.ListValueNames();
		for (auto& k : ks) {
			auto value = RegistryUtils::GetValueStatic(target.c_str(), k.c_str());
		}
	}
	return nullptr;
}

FwRules::FwRules() {
	this->Name = L"FwRules";
	this->Path = L"FireWall";
	this->Type = L"Native";
	this->Class = L"GetInfo";
	this->Description = L"List Fire Wall Rules";
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
}

ResultSet* FwRules::ModuleRun() {
	auto result = new ResultSet();
	FwRuleMgr::IterateFwRule([result](FwRule* rule) -> bool {
		result->PushDictOrdered("Name", StringUtils::ws2s(rule->GetName()));
		result->PushDictOrdered("ServiceName", StringUtils::ws2s(rule->GetServiceName()));
		result->PushDictOrdered("AppName", StringUtils::ws2s(rule->GetAppName()));
		result->PushDictOrdered("LocalAddr", StringUtils::ws2s(rule->GetLocalAddresses()));
		result->PushDictOrdered("LocalPorts", StringUtils::ws2s(rule->GetLocalPorts()));
		result->PushDictOrdered("RemoteAddr", StringUtils::ws2s(rule->GetRemoteAddresses()));
		result->PushDictOrdered("RemotePorts", StringUtils::ws2s(rule->GetRemotePorts()));
		result->PushDictOrdered("Description", StringUtils::ws2s(rule->GetDescription()));
		result->PushDictOrdered("Protocol", StringUtils::ws2s(rule->GetProtocol()));
		result->PushDictOrdered("Action", StringUtils::ws2s(rule->GetAction().WString()));
		result->PushDictOrdered("Direction", StringUtils::ws2s(rule->GetDirection().WString()));
		return true;
		});
	result->SetType(DICT);
	return result;
}

GetInjectedThread::GetInjectedThread() {
	this->Name = L"GetInjectedThread";
	this->Path = L"Thread";
	this->Type = L"Native";
	this->Class = L"GetInfo";
	this->Description = L"List Injected Thread";
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
}

std::string GetMemroyType(DWORD type) {
	std::string result = "";
	if (type == MEM_IMAGE) {
		result = "MEM_IMAGE";
	}
	else if (type == MEM_MAPPED) {
		result = "MEM_MAPPED";
	}
	else if (type == MEM_PRIVATE) {
		result = "MEM_PRIVATE";
	}
	else {
		result = "MEM_UNKNOWN";
	}

	return result;
}

std::string GetMemoryState(DWORD state) {
	std::string result = "";
	if (state == MEM_COMMIT) {
		result += "MEM_COMMIT";
	}
	else if (state == MEM_FREE) {
		result += "MEM_FREE";
	}
	else if (state == MEM_RESERVE) {
		result += "MEM_RESERVE";
	}
	else {
		result = std::to_string(state);
	}

	return result;
}

ResultSet* GetInjectedThread::ModuleRun() {
	auto result = new ResultSet();
	ProcessManager mgr;
	mgr.SetAllThreads();
	mgr.SetAllProcesses();
	for (auto pair : mgr.threadsMap) {
		auto pid = pair.first;
		auto threads = mgr.threadsMap[pid];
		//printf("%d\n", pid);
		auto process = mgr.processesMap[pid];
		for (auto t : threads) {
			try {
				auto baseAddr = t->GetBaseAddress();
				auto info = process->QueryMemoryInfo(baseAddr);
				if (info.Type == MEM_IMAGE && info.State == MEM_COMMIT) {
					continue;
				}
				result->PushDictOrdered("Pid", std::to_string(pid));
				result->PushDictOrdered("Name", StringUtils::ws2s(process->GetProcessName()));
				result->PushDictOrdered("Tid", std::to_string(t->threadId));
				result->PushDictOrdered("Type", GetMemroyType(info.Type));
				result->PushDictOrdered("State", GetMemoryState(info.State));
				//printf("info: %x\n", info.Type);
			}
			catch (...) {
				continue;
			}
		}
	}
	result->SetType(DICT);
	return result;
}
