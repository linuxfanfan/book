#include "ring_buffer.h"

typedef struct {
    u8              *pbyData;         /* data���׵�ַ */
    u32             dwBufferLen;      /* buffer�ĳ��� */
    u32             dwWriteLocation;  /* д�������λ�� */
    u32             dwReadLocation;   /* ���������λ�� */
    u32             dwDataToRead;         /* ��Ҫ����ȡ�������� */
    pthread_mutex_t tRingBufferMutex;     /* �߳��� */
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

    if( (ring_buf_bw + dwDataSize) <= ring_buf_len  ) //��ǰдλ��+����size <= ring_buf_len
    {
    	memcpy(ring_buf_source + ring_buf_bw, pbySrcData, dwDataSize);
    }
    else
    {
        len = ring_buf_len - ring_buf_bw;
        memcpy(ring_buf_source + ring_buf_bw, pbySrcData, len);
        memcpy(ring_buf_source, pbySrcData + len, dwDataSize - len);
         /* ����ע�⣬��Ϊ��buffer��ʼ�������Ѿ��ڵ�һ��memcpy������
                                     ��������len��ô�࣬����ʣ�µ�����Ӧ�ô�buffer+len��ʼ��*/
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

    if( (ring_buf_br + dwDataSize ) <= ring_buf_len ) //��ǰ��λ��+����size <= ring_buf_len
    {
    	memcpy(pbyBuffer, ring_buf_source + ring_buf_br, dwDataSize);
    }
    else
    {
    	len = ring_buf_len - ring_buf_br;
    	memcpy(pbyBuffer, ring_buf_source + ring_buf_br, len);
    	memcpy(pbyBuffer + len, ring_buf_source, dwDataSize - len); //�������ring_buffer˼·���ƣ�Ҳֻ���������л��ĸ�����
    }

    ptRingBufHandle->dwReadLocation = (ptRingBufHandle->dwReadLocation + dwDataSize) % ring_buf_len;
    ptRingBufHandle->dwDataToRead   -= dwDataSize;

    pthread_mutex_unlock(&(ptRingBufHandle->tRingBufferMutex));
    return 0;
}


