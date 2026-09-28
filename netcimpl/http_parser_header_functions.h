
void clearHeadersDataInfo(HeadersDataInfo *pInfo)
{
    if (pInfo != 0)
    {
        memset(pInfo, 0, sizeof(HeadersDataInfo));
    }
}

void printHeadersDataInfo(HeadersDataInfo *pInfo)
{
    if (pInfo != 0)
    {
        printf("HI: size:%d | used:%d | chunks:%d\n", pInfo->size, pInfo->used, pInfo->num_chunks);
    }
    else
    {
        printf("HI: (nil)\n");
    }
}

int getHeadersDataInfo(HttpRequest *pReq, HeadersDataInfo *pInfo)
{
    if (pReq != 0 && pInfo != 0)
    {
        void *headers = pReq->__headers;
        if (headers != 0)
        {
            unsigned int size = 0, used = 0, chunks = 0;

            while (headers != 0)
            {
                HeadersDataHeader *pH = (HeadersDataHeader *)headers;
                used += pH->size_and_used & 0xFFFF;
                size += pH->size_and_used >> 16;
                ++chunks;
                headers = pH->next;
            }
            pInfo->size = size;
            pInfo->used = used;
            pInfo->num_chunks = chunks;
        }
        return 0;
    }
    return 1;
}