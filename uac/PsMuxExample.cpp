// PsMuxExample.cpp : 定义控制台应用程序的入口点。
//

#include <iostream>
#include "PsMuxExample.h"

//遍历block拆分NALU,直到MaxSlice,不然一直遍历下去
int process_block(guint8* pBlock, int BlockLen, int MaxSlice,  PsMuxContext* PsDst)
{
    static guint8* pStaticBuf = new guint8[BUF_LEN];
    static int StaticBufSize = 0;

    guint8* pCurBlock = NULL;

    int LastBlockLen = 0;

    memcpy(pStaticBuf+StaticBufSize, pBlock, BlockLen);

    LastBlockLen = StaticBufSize+BlockLen;

    guint8* pCurPos = pStaticBuf;

    guint8* NaluStartPos = NULL;
    guint8* NaluEndPos   = NULL;


    //一段数据里最多NALU个数,这样SPS PPS 后的I帧那就不用遍历
    int iSliceNum = 0;

    while (LastBlockLen > 4)
    {
        if(isH264Or265Frame(pCurPos,NULL)){
            if (iSliceNum + 1 >= MaxSlice){//已经到达最大NALU个数,下面的不用找了把剩下的加上就是
                PsDst->Process(pCurPos, LastBlockLen);
                break;
            }

            if (NaluStartPos == NULL){
                NaluStartPos = pCurPos;
            }
            else{
                PsDst->Process(NaluStartPos, pCurPos-NaluStartPos);
                iSliceNum++;
                NaluStartPos = pCurPos;
            }
        }

        pCurPos++;
        LastBlockLen--;
    }

    //有剩下的,保存,和后面的拼起来
    if (NaluStartPos){
        memcpy(pStaticBuf, NaluStartPos, LastBlockLen);
        StaticBufSize = LastBlockLen;
    }
    return 0;
}

int main1(int argc, char* argv[])
{
    Gb28181PsMux PsMuxer;
    int Circle = 0;



    PsProcessSaveFile SaveFile("PsMux.mpeg");
//    PsProcessRtp SaveFile("122.224.82.77", 2054);
    unsigned char pTest[] = {0x00, 0x00, 0x00, 0x01, 0x27, 0x55, 0x66,
        0x00, 0x00, 0x00, 0x01, 0x28, 0x55, 
        0x00, 0x00, 0x00, 0x01, 0x25, 0x66};

    //SaveFile.testMuxSpsPpsI(pTest, sizeof(pTest));

    FILE* fp = fopen("2.x264", "rb");
    if (fp == NULL)
    {
        printf("can't open file %s\n", argv[1]);
        return -1;
    }

    guint8* fReadbuf = new guint8[BUF_LEN];

    while(1)
    {
        int fReadsz = fread(fReadbuf, 1, BUF_LEN, fp);

        if(fReadsz <= 0){

            if (Circle){
                fseek(fp, 0, SEEK_SET);
                continue;
            }
            else{
                break;
            }
        }

        process_block(fReadbuf, fReadsz, 0xffff, &SaveFile);
    }

    delete []fReadbuf;

	return 0;
}

PsMuxContext::PsMuxContext() {
    Idx = PsMux.AddStream(PSMUX_ST_VIDEO_H264);
    pMuxBuf = new guint8[BUF_LEN];
    pts = 0;
    dts = 0;
}

PsMuxContext::~PsMuxContext() {
    delete []pMuxBuf;
}

void PsMuxContext::Process(guint8 *buf, int len) {
    int MuxOutSize = 0;
    int ret = PsMux.MuxH264SingleFrame(buf, len, pts, dts, Idx, pMuxBuf, &MuxOutSize, BUF_LEN);

    if (ret == 0 && MuxOutSize > 0){
        OnPsFrameOut(pMuxBuf, MuxOutSize, pts, dts);
    }
    else if(ret == 3){

    }
    else{
        printf("mux error!\n");
    }

    unsigned char c = 0;
    if (!isH264Or265Frame(buf, &c)){
        return;
    }

    NAL_type Type = getH264NALtype(c);

    if ((Type == NAL_IDR) || (Type == NAL_PFRAME)){
        pts += 3600;
        dts += 3600;
    }
}

void PsMuxContext::testMuxSpsPpsI(guint8 *buf, int len) {
    int MuxOutSize = 0;
    PsMux.MuxH264SpsPpsIFrame(buf, len, 0, 0, Idx, pMuxBuf, &MuxOutSize, BUF_LEN);
}

#define MAX_RTP_PKT_LENGTH 1360
PsProcessRtp::PsProcessRtp(const char *ip, short port)
{
    uint16_t portbase = 0;
    uint32_t destip = inet_addr(ip);
    uint16_t destport = port;

    sessparams.SetOwnTimestampUnit(1.0/10.0);

    sessparams.SetAcceptOwnPackets(true);
    transparams.SetPortbase(portbase);

    int status = sess.Create(sessparams,&transparams);
    checkerror(status);

    destip = ntohl(destip);
    RTPIPv4Address addr(destip,destport);

    status = sess.AddDestination(addr);
    checkerror(status);

    sess.SetDefaultPayloadType(96);//设置传输类型
    sess.SetDefaultMark(true);      //设置位
    sess.SetTimestampUnit(1.0/9000.0); //设置采样间隔
    sess.SetDefaultTimestampIncrement(3600);//设置时间戳增加间隔
}

PsProcessRtp::~PsProcessRtp()
{
    sess.BYEDestroy(RTPTime(10,0),0,0);
}

void PsProcessRtp::OnPsFrameOut(guint8* buf, int len, gint64 pts, gint64 dts)
{
    char sendbuf[1430];   //发送的数据缓冲
    memset(sendbuf,0,1430);

    int status = 0;

    if(len <= MAX_RTP_PKT_LENGTH) {
        memcpy(sendbuf,buf,len);
        status = sess.SendPacket((void *)sendbuf,len);
        checkerror(status);
    } else {
        //设置标志位Mark为0
        sess.SetDefaultMark(false);
        //printf("buflen = %d\n",buflen);
        //得到该需要用多少长度为MAX_RTP_PKT_LENGTH字节的RTP包来发送
        int k=0,l=0;
        k = len / MAX_RTP_PKT_LENGTH;
        l = len % MAX_RTP_PKT_LENGTH;
        int t=0;//用指示当前发送的是第几个分片RTP包

        char nalHeader = buf[0]; // NALU
        while( t < k || ( t==k && l>0 ) )
        {
            if( (0 == t ) || ( t<k && 0!=t ) )//第一包到最后包的前一包
            {
                memcpy(sendbuf,&buf[t*MAX_RTP_PKT_LENGTH],MAX_RTP_PKT_LENGTH);
                status = sess.SendPacket((void *)sendbuf,MAX_RTP_PKT_LENGTH);
                checkerror(status);
                t++;
            }
                //最后一包
            else if( ( k==t && l>0 ) || ( t== (k-1) && l==0 ))
            {
                //设置标志位Mark为1
                sess.SetDefaultMark(true);

                int iSendLen;
                if ( l > 0)
                {
                    iSendLen = len - t*MAX_RTP_PKT_LENGTH;
                }
                else
                    iSendLen = MAX_RTP_PKT_LENGTH;

                //sendbuf[0] = (nalHeader & 0x60)|28;
                //sendbuf[1] = (nalHeader & 0x1f);
                //sendbuf[1] |= 0x40;

                //memcpy(sendbuf+2,&pSendbuf[t*MAX_RTP_PKT_LENGTH],iSendLen);
                //status = this->SendPacket((void *)sendbuf,iSendLen+2);

                memcpy(sendbuf,&buf[t*MAX_RTP_PKT_LENGTH],iSendLen);
                status = sess.SendPacket((void *)sendbuf,iSendLen);

                checkerror(status);
                t++;
            }
        }
    }

    RTPTime::Wait(RTPTime(1,0));
}

void PsProcessRtp::checkerror(int rtperr)
{
    if (rtperr < 0)
    {
        std::cout << "ERROR: " << RTPGetErrorString(rtperr) << std::endl;
        exit(-1);
    }
}

PsProcessSaveFile::PsProcessSaveFile(std::string DstName)
{
    fp = fopen(DstName.c_str(), "wb+");
}

PsProcessSaveFile::~PsProcessSaveFile()
{
    if (fp){
        fclose(fp);
    }
}

void PsProcessSaveFile::OnPsFrameOut(guint8* buf, int len, gint64 pts, gint64 dts) {
    if (len > 0 && fp) {
        fwrite(buf, len, 1, fp);
        fflush(fp);
    }
}