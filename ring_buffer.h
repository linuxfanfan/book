#ifndef _RING_BUFFER_H
#define _RING_BUFFER_H

static s32 create_ring_buffer(void **pptRingBufHandle, u32 dwBufferLength);
static s32 destory_ring_buffer(void **pptRingBufHandle);
static s32 write_ring_buffer(void **pptRingBufHandle, u8 *pbySrcData, u32 dwDataSize);
static s32 read_ring_buffer(void **pptRingBufHandle, u8 *pbyDstBuffer, u32 dwDataSize);


#endif
