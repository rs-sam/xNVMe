// SPDX-FileCopyrightText: Samsung Electronics Co., Ltd
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif
#ifndef __INTERNAL_XNVME_BE_WINDOWS_NVME_H
#define __INTERNAL_XNVME_BE_WINDOWS_NVME_H

#define _NTSCSI_USER_MODE_
#include <xnvme_dev.h>
#include <windows.h>
#include <ddk/scsi.h>
#include <ntddscsi.h>

#define SPT_CDB_LENGTH    32
#define SPT_SENSE_LENGTH  32
#define SPTWB_DATA_LENGTH 512

struct scsi_pass_through_direct_with_buffer {
	SCSI_PASS_THROUGH_DIRECT sptd;
	ULONG filler; ///< realign buffers to double word boundary
	UCHAR uc_sense_buf[SPT_SENSE_LENGTH];
};

//
// Parameter for IOCTL_STORAGE_PROTOCOL_COMMAND
// Buffer layout: <STORAGE_PROTOCOL_COMMAND> <Command> [Error Info Buffer] [Data-to-Device Buffer] [Data-from-Device Buffer]
//
#define STORAGE_PROTOCOL_STRUCTURE_VERSION              0x1

typedef struct _STORAGE_PROTOCOL_COMMAND {
    DWORD   Version;                        // STORAGE_PROTOCOL_STRUCTURE_VERSION
    DWORD   Length;                         // sizeof(STORAGE_PROTOCOL_COMMAND)
    STORAGE_PROTOCOL_TYPE  ProtocolType;
    DWORD   Flags;                          // Flags for the request
    DWORD   ReturnStatus;                   // return value
    DWORD   ErrorCode;                      // return value, optional
	DWORD   CommandLength;                  // non-zero value should be set by caller
    DWORD   ErrorInfoLength;                // optional, can be zero
    DWORD   DataToDeviceTransferLength;     // optional, can be zero. Used by WRITE type of request.
    DWORD   DataFromDeviceTransferLength;   // optional, can be zero. Used by READ type of request.
	DWORD   TimeOutValue;                   // in unit of seconds
    DWORD   ErrorInfoOffset;                // offsets need to be pointer aligned
    DWORD   DataToDeviceBufferOffset;       // offsets need to be pointer aligned
    DWORD   DataFromDeviceBufferOffset;     // offsets need to be pointer aligned
    DWORD   CommandSpecific;                // optional information passed along with Command.
    DWORD   Reserved0;
    DWORD   FixedProtocolReturnData;        // return data, optional. Some protocol, such as NVMe, may return a small amount data (DWORD0 from completion queue entry) without the need of separate device data transfer.
    DWORD   Reserved1[3];
    BYTE    Command[1];
} STORAGE_PROTOCOL_COMMAND, *PSTORAGE_PROTOCOL_COMMAND;;

//
// Bit-mask values for STORAGE_PROTOCOL_COMMAND - "Flags" field.
//
#define STORAGE_PROTOCOL_COMMAND_FLAG_ADAPTER_REQUEST    0x80000000     // Flag indicates the request targeting to adapter instead of device.

//
// Status values for STORAGE_PROTOCOL_COMMAND - "ReturnStatus" field.
//
#define STORAGE_PROTOCOL_STATUS_PENDING                 0x0
#define STORAGE_PROTOCOL_STATUS_SUCCESS                 0x1
#define STORAGE_PROTOCOL_STATUS_ERROR                   0x2
#define STORAGE_PROTOCOL_STATUS_INVALID_REQUEST         0x3
#define STORAGE_PROTOCOL_STATUS_NO_DEVICE               0x4
#define STORAGE_PROTOCOL_STATUS_BUSY                    0x5
#define STORAGE_PROTOCOL_STATUS_DATA_OVERRUN            0x6
#define STORAGE_PROTOCOL_STATUS_INSUFFICIENT_RESOURCES  0x7
#define STORAGE_PROTOCOL_STATUS_THROTTLED_REQUEST       0x8

#define STORAGE_PROTOCOL_STATUS_NOT_SUPPORTED           0xFF

//
// Command Length for Storage Protocols.
//
#define STORAGE_PROTOCOL_COMMAND_LENGTH_NVME            0x40            // NVMe commands are always 64 bytes.

//
// Command Specific Information for Storage Protocols - "CommandSpecific" field.
//
#define STORAGE_PROTOCOL_SPECIFIC_NVME_ADMIN_COMMAND    0x01
#define STORAGE_PROTOCOL_SPECIFIC_NVME_NVM_COMMAND      0x02

/**
 * Pass a NVMe IO Command through to the device via the Windows NVMe Kernel driver
 *
 * @param ctx Pointer to command context (::xnvme_cmd_ctx)
 * @param dbuf pointer to data-payload
 * @param dbuf_nbytes size of data-payload in bytes
 * @param mbuf pointer to meta-payload
 * @param mbuf_nbytes size of the meta-payload in bytes
 *
 * @return On success, 0 is returned. On error, negative `errno` is returned.
 */
int
xnvme_be_windows_sync_nvme_cmd_io(struct xnvme_cmd_ctx *ctx, void *dbuf, size_t dbuf_nbytes,
				  void *mbuf, size_t mbuf_nbytes);

/**
 * Pass a NVMe Admin Command through to the device via the Windows NVMe Kernel driver
 *
 * @param ctx Pointer to command context (::xnvme_cmd_ctx)
 * @param dbuf pointer to data-payload
 * @param dbuf_nbytes size of data-payload in bytes
 * @param mbuf pointer to meta-payload
 * @param mbuf_nbytes size of the meta-payload in bytes
 *
 * @return On success, 0 is returned. On error, negative `errno` is returned.
 */
int
xnvme_be_windows_nvme_cmd_admin(struct xnvme_cmd_ctx *ctx, void *dbuf, size_t dbuf_nbytes,
				void *mbuf, size_t mbuf_nbytes);

int
xnvme_be_windows_block_cmd_admin(struct xnvme_cmd_ctx *ctx, void *dbuf,
				 size_t XNVME_UNUSED(dbuf_nbytes), void *XNVME_UNUSED(mbuf),
				 size_t XNVME_UNUSED(mbuf_nbytes));

#endif /* __INTERNAL_XNVME_BE_WINDOWS_NVME_H */
