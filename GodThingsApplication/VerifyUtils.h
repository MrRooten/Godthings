#pragma once
#include "public.h"
#include <stdlib.h>
#include <Softpub.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <stdio.h>
#include <string>
#include "utils.h"
// Link with the Wintrust.lib file.
#pragma comment (lib, "wintrust")
#pragma comment (lib, "Crypt32")
struct SignatureInfomation{
    std::wstring info;
    bool isSignature;

    std::wstring GetInfo() {
        return this->info;
    }

    bool IsSignature() {
        return this->isSignature;
    }
};
SignatureInfomation* VerifyEmbeddedSignature(LPCWSTR pwszSourceFile);

typedef enum _VERIFY_RESULT
{
    VrUnknown = 0,
    VrNoSignature,
    VrTrusted,
    VrExpired,
    VrRevoked,
    VrDistrust,
    VrSecuritySettings,
    VrBadSignature
} VERIFY_RESULT, * PVERIFY_RESULT;

DWORD VerifyCatalogSignature(_In_ LPCWSTR pwszSourceFile,
    _In_ bool UseStrongSigPolicy);

BytesBuffer Sha256(PBYTE bytes, size_t n);
BytesBuffer Sha1(PBYTE bytes, size_t n);
GTString Base64Encode(PBYTE bytes, size_t n);
BytesBuffer Md5File(HANDLE hFile);