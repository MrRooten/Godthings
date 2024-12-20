#include <thread>
#include "Module.h"

#include "StringUtils.h"
#include "TestNativeModule.h"
#include "NativeModules.h"

#ifdef PYTHON_ENABLE

std::wstring GetDictItemString(PyObject* dict, const char* key) {
	PyObject* Key = PyUnicode_FromString(key);
	PyObject* Item = PyDict_GetItem(dict, Key);
	PyObject* ItemString = PyUnicode_AsEncodedString(Item, "utf-8", "~E~");
	char* bytes = PyBytes_AS_STRING(ItemString);
	std::wstring res = StringUtils::s2ws(bytes);
	Py_XDECREF(Item);
	Py_XDECREF(ItemString);
	Py_XDECREF(bytes);
	return res;
}
#endif // PYTHON_ENABLE

std::wstring helpPath(std::wstring path) {
	auto tmps = StringUtils::StringSplit(path, L"\\");
	return StringUtils::StringsJoin(tmps, L"\\\\");
}

#ifdef PYTHON_ENABLE
void PythonModule::_LoadMetaData() {
	std::vector<PyObject*> args;
	std::string file = StringUtils::ws2s(this->Path);
	std::string function = "meta_data";
	PythonUtils::RunFunction([](LPVOID pObject)->int {
		return 0;
		}, file.c_str(), "meta_data", args);
	PythonUtils::RunFunction([this](LPVOID pObject)->int {
		this->Description = GetDictItemString((PyObject*)pObject, "Description");

		this->Class = GetDictItemString((PyObject*)pObject, "Class");

		this->Name = GetDictItemString((PyObject*)pObject, "Name");
		Py_XDECREF(pObject);
		return 0;
		}, file.c_str(), function.c_str(), args);
		
	
	return;
}
#endif // PYTHON_ENABLE
void PythonModule::Initialize() {

}

PythonModule::PythonModule(std::wstring path) {
#ifdef  PYTHON_ENABLE
	this->Path = path;
	this->_LoadMetaData();
	auto mgr = ModuleMgr::GetMgr();
	mgr->RegisterModule(this);
#endif //  PYTHON_ENABLE
}

VOID Module::SetArgs(const Args& args) {
	this->args = args;
}
#ifdef PYTHON_ENABLE
VOID Module::SetVM(PyInterpreterState* state) {
	this->state = state;
}
#endif

#ifdef PYTHON_ENABLE
ResultSet* PythonModule::ModuleRun() {
	//this->_locker.lock();
	PyObject* pyArgs = PyDict_New();
	ResultSet* result;
	for (auto arg : args) {
		PyObject* pyKeyString = PyUnicode_FromString(arg.first.c_str());
		PyObject* pyValueString = PyUnicode_FromString(arg.second.c_str());
		PyDict_SetItem(pyArgs, pyKeyString, pyValueString);
	}
	std::vector<PyObject*> vPyArgs;
	vPyArgs.push_back(pyArgs);
	auto file = StringUtils::ws2s(this->Path);
	std::string function = "module_run";

	auto ProcModule = [&result](LPVOID pArgs)->int {
		PyObject* pObject = (PyObject*)pArgs;
		PyObject* pyError;
		if (PyExceptionClass_Check(pObject)) {
			result = new ResultSet();
			result->SetType(ERROR_MESSAGE);
			result->SetErrorMessage(PythonUtils::GetObjectString(pObject));
		}
		else if (PyDict_Check(pObject)) {
			PyObject* key, * value;
			Py_ssize_t pos = 0;
			result = new ResultSet();
			while (PyDict_Next(pObject, &pos, &key, &value)) {
				if (!PyUnicode_Check(key)) {
					result->dataDict.clear();
					result->SetErrorMessage("The key type must be str");
					return 0;
				}

				if (!PyList_Check(value)) {
					result->dataDict.clear();
					result->SetErrorMessage("The value type must be list");
					return 0;
				}

				Py_ssize_t size = PyList_Size(value);
				PyObject* item;
				std::string key_stdstring = PythonUtils::PyStringToString(key);
				auto& valueArray = result->dataDict[key_stdstring];
				for (Py_ssize_t pos = 0; pos < size; pos++) {
					item = PyList_GetItem(value, pos);
					if (!PyUnicode_Check(item)) {
						result->dataDict.clear();
						result->SetErrorMessage("The list value's item must be str");
						return 0;
					}
					valueArray.push_back(PythonUtils::PyStringToString(item));
				}
			}
			result->SetType(DICT);
		}
		else if (PyList_Check(pObject)) {
			result = new ResultSet();
			Py_ssize_t size = PyList_Size(pObject);
			PyObject* item;
			for (Py_ssize_t pos = 0; pos < size; pos++) {
				item = PyList_GetItem(pObject, pos);
				if (!PyUnicode_Check(item)) {
					result->dataArray.clear();
					result->SetErrorMessage("The list value's item must be str");
					return 0;
				}
				result->dataArray.push_back(PythonUtils::PyStringToString(item));
			}
			result->SetType(ARRAY);
		}
		else if (PyUnicode_Check(pObject)) {
			result = new ResultSet();
			result->data = PythonUtils::PyStringToString(pObject);
			result->SetType(TEXT_STRING);
		}
		else if (pObject == NULL) {
			pyError = PythonUtils::GetLastError();
			result = new ResultSet();
			if (pyError != NULL) {
				result->SetErrorMessage(PythonUtils::GetObjectString(pyError));
			}
			else {
				result->SetErrorMessage("Unknown error");
			}
		}
		else {
			result = new ResultSet();
			try {
				result->data = PythonUtils::GetObjectString(pObject);
				result->SetType(TEXT_STRING);
			}
			catch (...) {
				result->SetErrorMessage("Unknown error");
			}
		}
		return 0;
	};
	//sub_interpreter::thread_scope scope(this->state);
#ifdef PYTHON_ENABLE
	PythonUtils::RunFunction(ProcModule,file.c_str(), function.c_str(), vPyArgs);
#endif
	if (this->pPromise != NULL) {
		this->pPromise->set_value_at_thread_exit(result);
	}
	this->_result_set = result;
	//this->_locker.unlock();
	return result;
}

#endif

VOID PythonModule::SetPromise(std::promise<ResultSet*>& promise) {
	this->pPromise = &promise;
}

void NativeModule::Initialize() {

}

VOID NativeModule::SetPromise(std::promise<ResultSet*>& promise) {
	this->pPromise = &promise;
}

ModuleMgr* ModuleMgr::ModManager;
ModuleMgr* ModuleMgr::GetMgr() {
	if (ModuleMgr::ModManager == NULL) {
		ModuleMgr::ModManager = new ModuleMgr();
		ModuleMgr::ModManager->LoadModules();
	}
	return ModuleMgr::ModManager;
}

DWORD ModuleMgr::RegisterModule(Module* mod) {
	this->modules.push_back(mod);
	return 0;
}

#include "FileUtils.h"
#include <filesystem>
DWORD ModuleMgr::LoadModules() {
	ProcessModule* processModule = new ProcessModule();
	ServiceModule* serviceModule = new ServiceModule();
	ListTestModule* listTestModule = new ListTestModule();
	StartupModule* startupModule = new StartupModule();
	FilesRelateOpenCommandsModule* relateProgram = new FilesRelateOpenCommandsModule();
	NetworkModule* network = new NetworkModule();
	Rundll32Backdoor* rundll = new Rundll32Backdoor();
	ShadowAccount* shadowAccount = new ShadowAccount();
	USBHistory* usbHistory = new USBHistory();
	ListSchduleTask* task = new ListSchduleTask(); 
	WatchNetstat* loop = new WatchNetstat();
	MailiousProcessDlls* unsignedDlls = new MailiousProcessDlls();
	new RecentRunning();
	new Accounts();
	new FwRules();
	new GetInjectedThread();
#ifdef  PYTHON_ENABLE

	std::wstring currPath = std::filesystem::current_path().native().c_str();
	std::wstring pluginPath = currPath + L"\\plugins\\*";
	Dir dir(pluginPath.c_str());
	auto files = dir.ListFiles();
	for (auto& f : files) {
		try {
			auto f2 = f.substr(0, f.size() - 3);
			std::wstring pyPluginPath = currPath + L"\\plugins\\" + f2;
			new PythonModule(pyPluginPath.c_str());
		}
		catch (...) {
			continue;
		}
	}
#endif //  PYTHON_ENABLE
	return 0;
}

std::string ModuleMgr::GetModulesJson() {
	Json::Value resJson;
	for (auto mod : this->modules) {
		resJson[StringUtils::ws2s(mod->Name)] = mod->GetModuleMetaJson();
	}
	Json::FastWriter fastWriter;
	return fastWriter.write(resJson);
}

Module* ModuleMgr::FindModuleByName(const char* modName) {
	return this->FindModuleByName(StringUtils::s2ws(modName).c_str());
}

Module* ModuleMgr::FindModuleByName(const wchar_t* modName) {
	Module* res = NULL;
	for (auto& mod : this->modules) {
		if (mod->Name == modName) {
			res = mod;
			break;
		}
	}
	return res;
}

void ResultSet::PushDictOrdered(std::string key, std::string value) {
	if (this->dataDict.count(key) == 0) {
		this->_map_order.push_back(key);
	}
	this->dataDict[key].push_back(value);
}

std::vector<std::string>& ResultSet::GetMapOrder() {
	return this->_map_order;
}

ResultSet::ResultSet() {
	this->type = NONE;
}

ResultSet::ResultSet(std::string const &data) {
	this->type = TEXT_STRING;
	this->data = data;
}

ResultSet::ResultSet(Dict const& data) {
	this->type = DICT;
	this->dataDict = data;
}

ResultSet::ResultSet(Array const& data) {
	this->type = ARRAY;
	this->dataArray = data;
}

std::string ResultSet::ToJsonString() {
	Json::FastWriter fastWriter;
	std::string output = fastWriter.write(this->ToJsonObject());
	return output;
}

Json::Value ResultSet::ToJsonObject() {
	Json::Value value;
	if (this->type == ARRAY) {
		Json::Value data;
		for (int i = 0; i < this->dataArray.size(); i++) {
			data[i] = this->dataArray[i];
		}
		value["Data"] = data;
		value["Type"] = "array";
	}
	else if (this->type == TEXT_STRING) {
		Json::Value data;
		data = this->data;
		value["Data"] = data;
		value["Type"] = "text_string";
	}
	else if (this->type == DICT) {
		Json::Value data;
		/*for (auto& item : this->dataDict) {
			Json::Value arrayValue;
			for (size_t i = 0; i < item.second.size(); i++) {
				arrayValue[i] = item.second[i];
			}
			data[item.first] = arrayValue;
		}*/
		if (this->_map_order.size() == 0) {
			for (auto& item : this->dataDict) {
				this->_map_order.push_back(item.first);
			}
		}
		for (auto& key : this->_map_order) {
			Json::Value arrayValue;
			auto& v = dataDict[key];
			size_t len = v.size();
			for (int i = 0; i < len; i++) {
				arrayValue[i] = v[i];
			}
			data[key] = arrayValue;
		}

		Json::Value order;
		for (int i = 0; i < _map_order.size();i++) {
			order[i] = _map_order[i];
		}
		value["Data"] = data;
		value["Type"] = "dict";
		value["Order"] = order;
	}
	else if (this->type == ERROR_MESSAGE) {
		value["Data"] = this->data;
		value["Type"] = "error";
	}
	else {
		value["Data"] = this->data;
		value["Type"] = "unknown";
	}
	if (this->report.size() != 0) {
		value["Report"] = this->report;
	}
	return value;
}

std::string _csv_escape(std::string& s) {
	std::string from = "\"";
	std::string to = "\"\"";
	StringUtils::replaceAll(s, from, to);
	s = "\"" + s + "\"";
	return s;
}
std::string ResultSet::ToCsvString() {
	auto res = this;
	std::string result;
	if (res->type == ERROR_MESSAGE) {
		return "";
	}
	if (res == nullptr) {
		return "";
	}
	auto json = res->ToJsonObject();
	auto data = json["Data"];
	int size = 0;
	auto orders = res->GetMapOrder();
	std::string first_line = "";
	std::vector<std::string> _tmp;
	for (auto& key : orders) {
		_tmp.push_back(key);
		size = data[key].size();
	}
	result += StringUtils::StringsJoin(_tmp, ",") + "\r\n";
	_tmp.clear();
	for (int i = 0; i < size; i++) {
		std::vector<std::string> values;
		for (auto& key : orders) {
			auto member = data[key][i];
			std::string value = member.asCString();
			value = _csv_escape(value);
			values.push_back(value);
		}
		auto s = StringUtils::StringsJoin(values, ",") + "\r\n";
		result += s;
	}
	return result;
}

Json::Value Module::GetModuleMetaJson() {
	Json::Value metaJson;
	metaJson["Name"] = StringUtils::ws2s(this->Name);
	metaJson["Type"] = StringUtils::ws2s(this->Type);
	metaJson["Class"] = StringUtils::ws2s(this->Class);
	metaJson["Path"] = StringUtils::ws2s(this->Path);
	metaJson["Description"] = StringUtils::ws2s(this->Description);
	return metaJson;
}
