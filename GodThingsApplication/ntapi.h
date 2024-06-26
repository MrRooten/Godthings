#pragma once
#ifndef _NTAPI_H
#define _NTAPI_H
#include "public.h"


#define FIELD_OFFSET(type, field)    ((LONG)(LONG_PTR)&(((type *)0)->field))
#define UFIELD_OFFSET(type, field)    ((DWORD)(LONG_PTR)&(((type *)0)->field))

typedef struct _UNICODE_STRING123 {
    USHORT Length;
    USHORT MaximumLength;
    PWCH   Buffer;
} UNICODE_STRING123, * PUNICODE_STRING123;

typedef struct _OBJECT_ATTRIBUTES {
    ULONG           Length;
    HANDLE          RootDirectory;
    PUNICODE_STRING123 ObjectName;
    ULONG           Attributes;
    PVOID           SecurityDescriptor;
    PVOID           SecurityQualityOfService;
}  OBJECT_ATTRIBUTES, * POBJECT_ATTRIBUTES;

typedef struct _CLIENT_ID
{
    PVOID UniqueProcess;
    PVOID UniqueThread;
} CLIENT_ID, * PCLIENT_ID;

#define InitOA(ptr, root, attrib, name, desc, qos) { (ptr)->Length = sizeof(OBJECT_ATTRIBUTES);  (ptr)->RootDirectory = root; (ptr)->Attributes = attrib; (ptr)->ObjectName = name; (ptr)->SecurityDescriptor = desc; (ptr)->SecurityQualityOfService = qos; }
typedef NTSYSAPI NTSTATUS(* __ntOpenProcess)(
    PHANDLE            ProcessHandle,
    ACCESS_MASK        DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PCLIENT_ID         ClientId
    );

typedef NTSYSAPI NTSTATUS(* __ntReadVirtualMemory)(
    HANDLE ProcessHandle,
    PVOID BaseAddress,
    PVOID Buffer,
    ULONG NumberOfBytesToRead,
    PULONG NumberOfBytesReaded
    );

typedef ULONG (*pRtlNtStatusToDosError)(
    NTSTATUS Status
);

typedef struct _IO_STATUS_BLOCK
{
    union
    {
        NTSTATUS Status;
        PVOID Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef VOID(NTAPI* PIO_APC_ROUTINE)(
    _In_ PVOID ApcContext,
    _In_ PIO_STATUS_BLOCK IoStatusBlock,
    _In_ ULONG Reserved
    );

typedef NTSTATUS
(*pNtDeviceIoControlFile)(
    _In_ HANDLE FileHandle,
    _In_opt_ HANDLE Event,
    _In_opt_ PIO_APC_ROUTINE ApcRoutine,
    _In_opt_ PVOID ApcContext,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _In_ ULONG IoControlCode,
    _In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
    _In_ ULONG InputBufferLength,
    _Out_writes_bytes_opt_(OutputBufferLength) PVOID OutputBuffer,
    _In_ ULONG OutputBufferLength
);
static pRtlNtStatusToDosError RtlNtStatusToDosError = NULL;

typedef NTSTATUS 
(*pRtlDecompressBufferEx)(
    _In_  USHORT CompressionFormat,
    _Out_ PUCHAR UncompressedBuffer,
    _In_  ULONG  UncompressedBufferSize,
    _In_  PUCHAR CompressedBuffer,
    _In_  ULONG  CompressedBufferSize,
    _In_  PULONG FinalUncompressedSize,
    _In_  PVOID  WorkSpace
);
typedef NTSTATUS 
(*pRtlGetCompressionWorkSpaceSize)(
    _In_  USHORT CompressionFormatAndEngine,
    _Out_ PULONG CompressBufferWorkSpaceSize,
    _Out_ PULONG CompressFragmentWorkSpaceSize
);
typedef LONG KPRIORITY;

DWORD NtStatusHandler(NTSTATUS status);

void* GetNativeProc(const CHAR* procName);

void* GetAnyProc(const CHAR* mod, const CHAR* procName);

#endif // !_NTAPI_H