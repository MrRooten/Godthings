#include "Network.h"
#include "utils.h"
#include "ntapi.h"
#include "NetworkUtils.h"
#include <Shlwapi.h>

typedef struct _MIB_TCP6ROW2 {
	IN6_ADDR                     LocalAddr;
	DWORD                        dwLocalScopeId;
	DWORD                        dwLocalPort;
	IN6_ADDR                     RemoteAddr;
	DWORD                        dwRemoteScopeId;
	DWORD                        dwRemotePort;
	MIB_TCP_STATE                State;
	DWORD                        dwOwningPid;
	TCP_CONNECTION_OFFLOAD_STATE dwOffloadState;
} MIB_TCP6ROW2, * PMIB_TCP6ROW2;
typedef struct _MIB_TCP6TABLE2 {
	DWORD        dwNumEntries;
	MIB_TCP6ROW2 table[ANY_SIZE];
} MIB_TCP6TABLE2, * PMIB_TCP6TABLE2;
typedef ULONG (*pGetTcp6Table2)(
	PMIB_TCP6TABLE2 TcpTable,
	PULONG         SizePointer,
	BOOL           Order
);
void NetworkManager::ClearConnection() {
	for (auto& conn : this->connections) {
		delete conn;
	}
	this->connections.clear();
	std::vector<Connection*>().swap(this->connections);
}
DWORD NetworkManager::SetTCPConnection() {
	PMIB_TCPTABLE2 pTcpTable = NULL;
	PMIB_TCP6TABLE2 pTcp6Table = NULL;
	DWORD size = 1000;
	DWORD status = 0;
	DWORD num;
	pGetTcp6Table2 GetTcp6Table2 = NULL;
	pTcpTable = (PMIB_TCPTABLE2)LocalAlloc(GPTR, size);
	if (pTcpTable == NULL) {
		LOG_ERROR_REASON(L"Error in Alloc Space");
		goto cleanup;
	}
	
	status = GetTcpTable2(pTcpTable, &size, TRUE);
	if (status != NO_ERROR && status != ERROR_INSUFFICIENT_BUFFER) {
		LOG_ERROR_REASON(L"Error in GetTcpTable2");
		goto cleanup;
	}
	for (int i = 0; i < 8 && status == ERROR_INSUFFICIENT_BUFFER;i++) {
		pTcpTable = (PMIB_TCPTABLE2)LocalReAlloc(pTcpTable, size, GPTR|GMEM_MOVEABLE);
		if (pTcpTable == NULL) {
			LOG_ERROR_REASON(L"Error in Alloc Space");
			goto cleanup;
		}
		status = GetTcpTable2(pTcpTable, &size, TRUE);
	}


	GetTcp6Table2 = (pGetTcp6Table2)GetAnyProc("Iphlpapi.dll", "GetTcp6Table2");
	if (GetTcp6Table2 != NULL) {
		size = 0x1000;
		pTcp6Table = (PMIB_TCP6TABLE2)LocalAlloc(GPTR, size);
		if (pTcp6Table == NULL) {
			LOG_ERROR_REASON(L"Error in Alloc Space");
			goto cleanup;
		}

		status = GetTcp6Table2(pTcp6Table, &size, TRUE);
		if (status != NO_ERROR && status != ERROR_INSUFFICIENT_BUFFER) {
			LOG_ERROR_REASON(L"Error in Get Tcp6Table2");
			goto cleanup;
		}
		for (int i = 0; i < 8 && status == ERROR_INSUFFICIENT_BUFFER; i++) {
			pTcp6Table = (PMIB_TCP6TABLE2)LocalReAlloc(pTcp6Table, size, GPTR | GMEM_MOVEABLE);
			if (pTcpTable == NULL) {
				LOG_ERROR_REASON(L"Error in Alloc Space");
				goto cleanup;
			}
			status = GetTcp6Table2(pTcp6Table, &size, TRUE);
		}
	}
	this->connections.clear();
	num = pTcpTable->dwNumEntries;
	for (size_t i = 0; i < num; i++) {
		Connection* conn = new Connection();
		if (conn == nullptr) {
			LOG_ERROR_REASON(L"Can not new Connection()");
			break;
		}
		conn->localIPv4.S_un.S_addr = pTcpTable->table[i].dwLocalAddr;
		conn->localPort = ntohs(pTcpTable->table[i].dwLocalPort);
		conn->remoteIPv4.S_un.S_addr = pTcpTable->table[i].dwRemoteAddr;
		conn->remotePort = ntohs(pTcpTable->table[i].dwRemotePort);
		conn->owningPid = pTcpTable->table[i].dwOwningPid;
		conn->State = pTcpTable->table[i].dwState;
		conn->ipType = IPType::IPV4;
		this->connections.push_back(conn);
	}
	if (GetTcp6Table2 != NULL) {
		num = pTcp6Table->dwNumEntries;
		for (size_t i = 0; i < num; i++) {
			Connection* conn = new Connection();
			if (conn == nullptr) {
				LOG_ERROR_REASON(L"Can not new Connection()");
				break;
			}
			conn->localIPv6 = pTcp6Table->table[i].LocalAddr;
			conn->localPort = ntohs(pTcp6Table->table[i].dwLocalPort);
			conn->remoteIPv6 = pTcp6Table->table[i].RemoteAddr;
			conn->remotePort = ntohs(pTcp6Table->table[i].dwRemotePort);
			conn->owningPid = pTcp6Table->table[i].dwOwningPid;
			conn->State = pTcpTable->table[i].dwState;
			conn->ipType = IPType::IPV6;
			this->connections.push_back(conn);
		}
	}
cleanup:
	if (pTcpTable != NULL) {
		LocalFree(pTcpTable);
	}

	if (pTcp6Table != NULL) {
		LocalFree(pTcp6Table);
	}
	return status;
}


DWORD NetworkManager::SetUDPConnection() {
	DWORD tableSize = 0;
	PVOID table = NULL;
	PMIB_UDPTABLE_OWNER_MODULE udp4Table;
	PMIB_UDP6TABLE_OWNER_MODULE udp6Table;
	GetExtendedUdpTable(NULL, &tableSize, FALSE, 2, UDP_TABLE_OWNER_MODULE, 0); //NULL, &tableSize, FALSE, AF_INET, UDP_TABLE_OWNER_MODULE, 0
	table = LocalAlloc(GPTR, tableSize);
	if (table == NULL) {
		return GetLastError();
	}

	if (GetExtendedUdpTable(table, &tableSize, FALSE, 2, UDP_TABLE_OWNER_MODULE, 0)!=ERROR_SUCCESS) {
		LocalFree(table);
		return GetLastError();
	}


	udp4Table = (PMIB_UDPTABLE_OWNER_MODULE)table;
	if (udp4Table != NULL) {
		for (DWORD i = 0; i < udp4Table->dwNumEntries; i++) {
			Connection* conn = new Connection();
			if (conn == nullptr) {
				LOG_ERROR_REASON(L"Can not new Connection()");
				break;
			}
			conn->localIPv4 = NetworkUtils::ConvertDWORDToIN_ADDR(udp4Table->table[i].dwLocalAddr);
			conn->localPort = ntohs(udp4Table->table[i].dwLocalPort);
			conn->owningPid = udp4Table->table[i].dwOwningPid;
			conn->protocol = Protocol::UDP;
			conn->ipType = IPType::IPV4;
			this->connections.push_back(conn);
		}
	}

	LocalFree(table);
	table = NULL;
	tableSize = 0;
	udp4Table = NULL;

	GetExtendedUdpTable(NULL, &tableSize, FALSE, 23, UDP_TABLE_OWNER_MODULE, 0); //NULL, &tableSize, FALSE, AF_INET, UDP_TABLE_OWNER_MODULE, 0
	table = LocalAlloc(GPTR, tableSize);
	if (table == NULL) {
		return GetLastError();
	}

	if (GetExtendedUdpTable(table, &tableSize, FALSE, 23, UDP_TABLE_OWNER_MODULE, 0) != ERROR_SUCCESS) {
		LocalFree(table);
		return GetLastError();
	}


	udp6Table = (PMIB_UDP6TABLE_OWNER_MODULE)table;
	if (udp6Table != NULL) {
		for (DWORD i = 0; i < udp6Table->dwNumEntries; i++) {
			Connection* conn = new Connection();
			if (conn == nullptr) {
				LOG_ERROR_REASON(L"Can not new Connection()");
				break;
			}
			conn->localIPv6 = NetworkUtils::ConvertBytesToIN_ADDR6(udp6Table->table[i].ucLocalAddr);
			conn->localPort = ntohs(udp6Table->table[i].dwLocalPort);
			conn->owningPid = udp6Table->table[i].dwOwningPid;
			conn->protocol = Protocol::UDP;
			conn->ipType = IPType::IPV6;
			this->connections.push_back(conn);
		}
	}
	LocalFree(table);
	return 0;
}

std::vector<Connection*> NetworkManager::GetConnectionsByPid(DWORD pid) {
	this->ClearConnection();
	this->SetTCPConnection();
	this->SetUDPConnection();
	std::vector<Connection*> result;
	for (auto& conn : this->connections) {
		if (conn->owningPid == pid) {
			result.push_back(conn);
		}
	}
	return result;
}

std::vector<Connection*> NetworkManager::GetUDPConnections() {
	this->ClearConnection();
	this->SetUDPConnection();
	return this->connections;
}

std::vector<Connection*> NetworkManager::GetTCPConnections() {
	this->ClearConnection();
	this->SetTCPConnection();
	return this->connections;
}

std::vector<Connection*> NetworkManager::GetAllConnections() {
	this->ClearConnection();
	this->SetTCPConnection();
	this->SetUDPConnection();
	return this->connections;
}

VOID NetworkManager::UpdateTCPConnections() {

}


NetworkManager::~NetworkManager() {
	this->ClearConnection();
}

std::wstring ConvertIP(DWORD ip)
{
	unsigned char a, b, c, d;
	d = ip & 0xFF;
	c = (ip >> 8) & 0xFF;
	b = (ip >> 16) & 0xFF;
	a = (ip >> 24) & 0xFF;

	std::wstring conv;
	WCHAR buffer[40];
	wnsprintfW(buffer, 40, L"%u.%u.%u.%u", d, c, b, a);
	conv = buffer;
	return conv;
}
#include "StringUtils.h"

std::wstring ConvertIPv6(const IN6_ADDR* pAddr) {
	struct in6_addr a;
	memcpy(&a.u, &pAddr->u, sizeof(struct in6_addr));
	char buf[40];
	sprintf(buf, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
			a.s6_addr[0], a.s6_addr[1], a.s6_addr[2], a.s6_addr[3],
			a.s6_addr[4], a.s6_addr[5], a.s6_addr[6], a.s6_addr[7],
			a.s6_addr[8], a.s6_addr[9], a.s6_addr[10], a.s6_addr[11],
			a.s6_addr[12], a.s6_addr[13], a.s6_addr[14], a.s6_addr[15]);
	return StringUtils::s2ws(buf);

}
std::wstring Connection::GetLocalIPAsString() {
	if (this->ipType == IPType::IPV4) {
		return ConvertIP(this->localIPv4.S_un.S_addr);
	}
	return ConvertIPv6(&this->localIPv6);
}

std::wstring Connection::GetRemoteIPAsString() {
	if (this->ipType == IPType::IPV4) {
		return ConvertIP(this->remoteIPv4.S_un.S_addr);
	}
	return ConvertIPv6(&this->remoteIPv6);
}

std::wstring Connection::GetStateAsString() {
	if (this->State == MIB_TCP_STATE_CLOSED) {
		return L"MIB_TCP_STATE_CLOSED";
	}
	else if (this->State == MIB_TCP_STATE_LISTEN) {
		return L"MIB_TCP_STATE_LISTEN";
	}
	else if (this->State == MIB_TCP_STATE_ESTAB) {
		return L"MIB_TCP_STATE_ESTAB";
	}
	else if (this->State == MIB_TCP_STATE_SYN_RCVD) {
		return L"MIB_TCP_STATE_SYN_RCVD";
	}
	else if (this->State == MIB_TCP_STATE_SYN_SENT) {
		return L"MIB_TCP_STATE_SYN_SENT";
	}
	else if (this->State == MIB_TCP_STATE_FIN_WAIT1) {
		return L"MIB_TCP_STATE_FIN_WAIT1";
	}
	else if (this->State == MIB_TCP_STATE_FIN_WAIT2) {
		return L"MIB_TCP_STATE_FIN_WAIT2";
	}
	else if (this->State == MIB_TCP_STATE_CLOSE_WAIT) {
		return L"MIB_TCP_STATE_CLOSE_WAIT";
	}
	else if (this->State == MIB_TCP_STATE_CLOSING) {
		return L"MIB_TCP_STATE_CLOSING";
	}
	else if (this->State == MIB_TCP_STATE_LAST_ACK) {
		return L"MIB_TCP_STATE_LAST_ACK";
	}
	else if (this->State == MIB_TCP_STATE_TIME_WAIT) {
		return L"MIB_TCP_STATE_TIME_WAIT";
	}
	else if (this->State == MIB_TCP_STATE_DELETE_TCB) {
		return L"MIB_TCP_STATE_DELETE_TCB";
	}
	return L"";
}

bool operator==(const Connection& conn1, const Connection& conn2) {
	if (conn1.ipType != conn1.ipType) {
		return false;
	}

	if (conn1.localPort != conn2.localPort) {
		return false;
	}

	if (conn1.remotePort != conn2.remotePort) {
		return false;
	}

	if (conn1.State != conn2.State) {
		return false;
	}

	if (conn1.owningPid != conn2.owningPid) {
		return false;
	}

	if (conn1.protocol != conn2.protocol) {
		return false;
	}

	if (conn1.ipType == IPType::IPV4) {
		if (memcmp(&conn1.localIPv4,&conn2.localIPv4,sizeof(IN_ADDR)) != 0) {
			return false;
		}

		if (memcmp(&conn1.remoteIPv4, &conn2.remoteIPv4, sizeof(IN_ADDR)) != 0) {
			return false;
		}
	}
	else if (conn1.ipType == IPType::IPV6) {
		if (memcmp(&conn1.localIPv6, &conn2.localIPv6, sizeof(IN6_ADDR)) != 0) {
			return false;
		}

		if (memcmp(&conn1.remoteIPv6, &conn2.remoteIPv6, sizeof(IN6_ADDR)) != 0) {
			return false;
		}
	}

	return true;
}
#include <windns.h>

typedef struct _DNS_SVCB_OPTION {
	WORD wCode;
	WORD wDataLength;
	BYTE bData[ANYSIZE_ARRAY];
} DNS_SVCB_OPTION, * PDNS_SVCB_OPTION;

typedef struct _DNS_SVCB_DATA {
	WORD wPriority;
	WORD wPort;
	BYTE chPriorityNameLength;
	BYTE chServiceNameLength;
	BYTE chTargetNameLength;
	BYTE chOptions;
	BYTE bPriorityName[ANYSIZE_ARRAY];
	BYTE bServiceName[ANYSIZE_ARRAY];
	BYTE bTargetName[ANYSIZE_ARRAY];
	DNS_SVCB_OPTION Options[ANYSIZE_ARRAY];
} DNS_SVCB_DATA, * PDNS_SVCB_DATA;

typedef struct _DnsRecordA2 {
	struct _DnsRecordA* pNext;
	PSTR               pName;
	WORD               wType;
	WORD               wDataLength;
	union {
		DWORD            DW;
		DNS_RECORD_FLAGS S;
	} Flags;
	DWORD              dwTtl;
	DWORD              dwReserved;
	union {
		DNS_A_DATA          A;
		DNS_SOA_DATAA       SOA;
		DNS_SOA_DATAA       Soa;
		DNS_PTR_DATAA       PTR;
		DNS_PTR_DATAA       Ptr;
		DNS_PTR_DATAA       NS;
		DNS_PTR_DATAA       Ns;
		DNS_PTR_DATAA       CNAME;
		DNS_PTR_DATAA       Cname;
		DNS_PTR_DATAA       DNAME;
		DNS_PTR_DATAA       Dname;
		DNS_PTR_DATAA       MB;
		DNS_PTR_DATAA       Mb;
		DNS_PTR_DATAA       MD;
		DNS_PTR_DATAA       Md;
		DNS_PTR_DATAA       MF;
		DNS_PTR_DATAA       Mf;
		DNS_PTR_DATAA       MG;
		DNS_PTR_DATAA       Mg;
		DNS_PTR_DATAA       MR;
		DNS_PTR_DATAA       Mr;
		DNS_MINFO_DATAA     MINFO;
		DNS_MINFO_DATAA     Minfo;
		DNS_MINFO_DATAA     RP;
		DNS_MINFO_DATAA     Rp;
		DNS_MX_DATAA        MX;
		DNS_MX_DATAA        Mx;
		DNS_MX_DATAA        AFSDB;
		DNS_MX_DATAA        Afsdb;
		DNS_MX_DATAA        RT;
		DNS_MX_DATAA        Rt;
		DNS_TXT_DATAA       HINFO;
		DNS_TXT_DATAA       Hinfo;
		DNS_TXT_DATAA       ISDN;
		DNS_TXT_DATAA       Isdn;
		DNS_TXT_DATAA       TXT;
		DNS_TXT_DATAA       Txt;
		DNS_TXT_DATAA       X25;
		DNS_NULL_DATA       Null;
		DNS_WKS_DATA        WKS;
		DNS_WKS_DATA        Wks;
		DNS_AAAA_DATA       AAAA;
		DNS_KEY_DATA        KEY;
		DNS_KEY_DATA        Key;
		DNS_SIG_DATAA       SIG;
		DNS_SIG_DATAA       Sig;
		DNS_ATMA_DATA       ATMA;
		DNS_ATMA_DATA       Atma;
		DNS_NXT_DATAA       NXT;
		DNS_NXT_DATAA       Nxt;
		DNS_SRV_DATAA       SRV;
		DNS_SRV_DATAA       Srv;
		DNS_NAPTR_DATAA     NAPTR;
		DNS_NAPTR_DATAA     Naptr;
		DNS_OPT_DATA        OPT;
		DNS_OPT_DATA        Opt;
		DNS_DS_DATA         DS;
		DNS_DS_DATA         Ds;
		DNS_RRSIG_DATAA     RRSIG;
		DNS_RRSIG_DATAA     Rrsig;
		DNS_NSEC_DATAA      NSEC;
		DNS_NSEC_DATAA      Nsec;
		DNS_DNSKEY_DATA     DNSKEY;
		DNS_DNSKEY_DATA     Dnskey;
		DNS_TKEY_DATAA      TKEY;
		DNS_TKEY_DATAA      Tkey;
		DNS_TSIG_DATAA      TSIG;
		DNS_TSIG_DATAA      Tsig;
		DNS_WINS_DATA       WINS;
		DNS_WINS_DATA       Wins;
		DNS_WINSR_DATAA     WINSR;
		DNS_WINSR_DATAA     WinsR;
		DNS_WINSR_DATAA     NBSTAT;
		DNS_WINSR_DATAA     Nbstat;
		DNS_DHCID_DATA      DHCID;
		DNS_NSEC3_DATA      NSEC3;
		DNS_NSEC3_DATA      Nsec3;
		DNS_NSEC3PARAM_DATA NSEC3PARAM;
		DNS_NSEC3PARAM_DATA Nsec3Param;
		DNS_TLSA_DATA       TLSA;
		DNS_TLSA_DATA       Tlsa;
		DNS_SVCB_DATA       SVCB;
		DNS_SVCB_DATA       Svcb;
		DNS_UNKNOWN_DATA    UNKNOWN;
		DNS_UNKNOWN_DATA    Unknown;
		PBYTE               pDataPtr;
	} Data;
} DNS_RECORDA2, * PDNS_RECORDA2;



typedef struct _DNS_CACHE_ENTRY {
	struct _DNS_CACHE_ENTRY* pNext; // Pointer to next entry
	PWSTR pszName; // DNS Record Name
	unsigned short wType; // DNS Record Type
	unsigned short wDataLength; // Not referenced
	unsigned long dwFlags; // DNS Record FlagsB
	union {
		DNS_RECORDA Data;
		DNS_RECORDA DataUTF8;
		DNS_RECORDA DataAscii;
	};
} DNSCACHEENTRY, * PDNSCACHEENTRY;

typedef int(WINAPI* DNS_GET_CACHE_DATA_TABLE)(PDNSCACHEENTRY);

typedef void (WINAPI* P_DnsApiFree)(PVOID pData);
void UpdateDNS(void)
{

	PDNSCACHEENTRY pEntry = (PDNSCACHEENTRY)malloc(sizeof(DNSCACHEENTRY));
	// Loading DLL
	HINSTANCE hLib = LoadLibrary(TEXT("DNSAPI.dll"));
	// Get function address
	DNS_GET_CACHE_DATA_TABLE DnsGetCacheDataTable = (DNS_GET_CACHE_DATA_TABLE)GetProcAddress(hLib, "DnsGetCacheDataTableA");
	P_DnsApiFree pDnsApiFree = (P_DnsApiFree)GetProcAddress(hLib, "DnsApiFree");
	int stat = DnsGetCacheDataTable(pEntry);
	printf("stat = %d\n", stat);
	auto p = pEntry->pNext;
	while (p) {
		wprintf(L"%s : %d \n", (p->pszName), (p->wType));
		auto _p = p;
		p = p->pNext;
		pDnsApiFree(p);
	}

	FreeLibrary(hLib);
}
DnsCache* DnsCache::single = NULL;
DnsCache* DnsCache::GetInstance() {
	if (DnsCache::single != NULL) {
		return DnsCache::single;
	}
	auto cache = new DnsCache();
	PDNSCACHEENTRY pEntry = (PDNSCACHEENTRY)malloc(sizeof(DNSCACHEENTRY));

	if (pEntry == NULL) {
		return NULL;
	}
	// Loading DLL
	HINSTANCE hLib = LoadLibrary(TEXT("DNSAPI.dll"));
	// Get function address
	DNS_GET_CACHE_DATA_TABLE DnsGetCacheDataTable = (DNS_GET_CACHE_DATA_TABLE)GetProcAddress(hLib, "DnsGetCacheDataTable");
	P_DnsApiFree pDnsApiFree = (P_DnsApiFree)GetProcAddress(hLib, "DnsApiFree");
	int stat = DnsGetCacheDataTable(pEntry);
	auto p = pEntry->pNext;
	while (p) {
		//wprintf(L"%s : %d \n", (p->pszName), (p->wType));
		GTString ip = (LPSTR)inet_ntoa(*(PIN_ADDR)&p->Data.Data.A);
		if (p->wType == DNS_TYPE_A || p->wType == DNS_TYPE_AAAA) {
			if (p->pszName == INVALID_HANDLE_VALUE || p->pszName == NULL) {
				auto _p = p;
				p = p->pNext;
				continue;
			}
			GTWString domain = p->pszName;
			cache->_cache[ip] = StringUtils::ws2s(p->pszName);
		}
		auto _p = p;
		p = p->pNext;
		pDnsApiFree(_p);
	}
	free(pEntry);
	if (hLib != NULL) {
		FreeLibrary(hLib);
	}
	DnsCache::single = cache;
	return cache;
}

void DnsCache::Update() {
	PDNSCACHEENTRY pEntry = (PDNSCACHEENTRY)malloc(sizeof(DNSCACHEENTRY));
	if (pEntry == NULL) {
		return;
	}
	// Loading DLL
	HINSTANCE hLib = LoadLibrary(TEXT("DNSAPI.dll"));
	// Get function address
	DNS_GET_CACHE_DATA_TABLE DnsGetCacheDataTable = (DNS_GET_CACHE_DATA_TABLE)GetProcAddress(hLib, "DnsGetCacheDataTable");
	P_DnsApiFree pDnsApiFree = (P_DnsApiFree)GetProcAddress(hLib, "DnsApiFree");
	int stat = DnsGetCacheDataTable(pEntry);
	auto p = pEntry->pNext;
	while (p) {
		if (p->wType == DNS_TYPE_A || p->wType == DNS_TYPE_AAAA) {
			GTString ip = (LPSTR)inet_ntoa(*(PIN_ADDR)&p->Data.Data.A);
			if (this->_cache.contains(ip) == false) {
				if (p->pszName == INVALID_HANDLE_VALUE || p->pszName == NULL) {
					auto _p = p;
					p = p->pNext;
					continue;
				}
				GTWString domain = p->pszName;
				this->_cache[ip] = StringUtils::ws2s(p->pszName);
			}
		}
		auto _p = p;
		p = p->pNext;
		pDnsApiFree(_p);
	}
	free(pEntry);
	if (hLib != NULL) {
		FreeLibrary(hLib);
	}
}

LPCSTR DnsCache::GetDomain(const char* ip) {
	if (ip == NULL) {
		return ip;
	}

	GTString _s_ip = ip;
	if (this->_cache.contains(_s_ip)) {
		return this->_cache[_s_ip].c_str();
	}

	return NULL;
}


