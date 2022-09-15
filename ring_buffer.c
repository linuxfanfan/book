#include "ring_buffer.h"

typedef struct {
    u8              *pbyData;         /* data的首地址 */
    u32             dwBufferLen;      /* buffer的长度 */
    u32             dwWriteLocation;  /* 写操作光标位置 */
    u32             dwReadLocation;   /* 读操作光标位置 */
    u32             dwDataToRead;         /* 需要被读取的数据量 */
    pthread_mutex_t tRingBufferMutex;     /* 线程锁 */
} TRingBuffer;

static s32 create_ring_buffer(void **pptRingBufHandle, u32 dwBufferLength)
{
    TRingBuffer *ptRingBufHandle = NULL;
    if ((pptRingBufHandle == NULL) || (dwBufferLength <= 0))
    {
        printf("input param error,pptRingBufHandle=%p,pbyBufferAddr=%p,dwBufferLength=%d\n",
                        pptRingBufHandle, dwBufferLength);
        return -1;
    }

    ptRingBufHandle = (TRingBuffer *)malloc(sizeof(TRingBuffer));
    if (ptRingBufHandle == NULL)
    {
        printf("create_ring_buffer error\n");
        return -2;
    }

    memset(ptRingBufHandle, 0, sizeof(TRingBuffer));
    ptRingBufHandle->pbyData = (u8 *)malloc(dwBufferLength);
    if (ptRingBufHandle->pbyData == NULL)
    {
        printf("malloc data error\n");
        return -3;
    }
    ptRingBufHandle->dwBufferLen      = dwBufferLength;
    pthread_mutex_init (&(ptRingBufHandle->tRingBufferMutex), NULL);
    printf("create_ring_buffer success,ptRingBuffer->pbyData=%p,ptRingBuffer->dwDataLen=%d\n",
        ptRingBufHandle->pbyData, ptRingBufHandle->dwBufferLen);

    *pptRingBufHandle = ptRingBufHandle;

    return 0;
}

static s32 destory_ring_buffer(void **pptRingBufHandle)
{
    TRingBuffer *ptRingBufHandle = NULL;
    if (pptRingBufHandle == NULL)
    {
        printf("input param error,pptRingBufHandle=%p\n", pptRingBufHandle);
        return -1;
    }

    ptRingBufHandle = *pptRingBufHandle;

    if (ptRingBufHandle->pbyData == NULL)
    {
        printf("input param error,ptRingBufHandle->pbyData=%p\n", ptRingBufHandle->pbyData);
        return -2;
    }
    free(ptRingBufHandle->pbyData);
    free(ptRingBufHandle);
    *pptRingBufHandle = NULL;

    return 0;
}

static s32 write_ring_buffer(void **pptRingBufHandle, u8 *pbySrcData, u32 dwDataSize)
{
    TRingBuffer *ptRingBufHandle = (TRingBuffer *)(*pptRingBufHandle);
    pthread_mutex_lock(&(ptRingBufHandle->tRingBufferMutex));
    u32 len                  = 0;
    u32 ring_buf_bw          = ptRingBufHandle->dwWriteLocation;
    u32 ring_buf_len         = ptRingBufHandle->dwBufferLen;
    u8 *ring_buf_source      = ptRingBufHandle->pbyData;

    if( (ring_buf_bw + dwDataSize) <= ring_buf_len  ) //当前写位置+数据size <= ring_buf_len
    {
    	memcpy(ring_buf_source + ring_buf_bw, pbySrcData, dwDataSize);
    }
    else
    {
        len = ring_buf_len - ring_buf_bw;
        memcpy(ring_buf_source + ring_buf_bw, pbySrcData, len);
        memcpy(ring_buf_source, pbySrcData + len, dwDataSize - len);
         /* 这里注意，因为从buffer开始的数据已经在第一个memcpy函数中
                                     被拷贝了len这么多，所以剩下的数据应该从buffer+len开始。*/
    }

    ptRingBufHandle->dwWriteLocation = (ptRingBufHandle->dwWriteLocation + dwDataSize) % ring_buf_len;
    ptRingBufHandle->dwDataToRead        += dwDataSize;
    printf("ptRingBufHandle->dwDataToRead=%d\n", ptRingBufHandle->dwDataToRead);
    pthread_mutex_unlock(&(ptRingBufHandle->tRingBufferMutex));
    return 0;
}

static s32 read_ring_buffer(void **pptRingBufHandle, u8 *pbyDstBuffer, u32 dwDataSize)
{
    TRingBuffer *ptRingBufHandle = (TRingBuffer *)(*pptRingBufHandle);
    pthread_mutex_lock(&(ptRingBufHandle->tRingBufferMutex));
    if (ptRingBufHandle->dwDataToRead <= dwDataSize)
    {
        pthread_mutex_unlock(&(ptRingBufHandle->tRingBufferMutex));
        return 1;
    }
    u32 len                  = 0;
    u32 ring_buf_br          = ptRingBufHandle->dwReadLocation;
    u32 ring_buf_len         = ptRingBufHandle->dwBufferLen;
    u8  *ring_buf_source     = ptRingBufHandle->pbyData;

    if( (ring_buf_br + dwDataSize ) <= ring_buf_len ) //当前读位置+数据size <= ring_buf_len
    {
    	memcpy(pbyBuffer, ring_buf_source + ring_buf_br, dwDataSize);
    }
    else
    {
    	len = ring_buf_len - ring_buf_br;
    	memcpy(pbyBuffer, ring_buf_source + ring_buf_br, len);
    	memcpy(pbyBuffer + len, ring_buf_source, dwDataSize - len); //这里跟往ring_buffer思路类似，也只有这样才有环的概念了
    }

    ptRingBufHandle->dwReadLocation = (ptRingBufHandle->dwReadLocation + dwDataSize) % ring_buf_len;
    ptRingBufHandle->dwDataToRead   -= dwDataSize;

    pthread_mutex_unlock(&(ptRingBufHandle->tRingBufferMutex));
    return 0;
}


