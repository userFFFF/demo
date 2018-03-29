#ifndef UAC_PSMUXEXAMPLE_H
#define UAC_PSMUXEXAMPLE_H

#include "libpsmux/inc/Gb28181PsMux.h"
#include <jrtplib3/rtpsession.h>
#include <jrtplib3/rtpudpv4transmitter.h>
#include <jrtplib3/rtpipv4address.h>
#include <jrtplib3/rtpsessionparams.h>
#include <jrtplib3/rtperrors.h>
#include <jrtplib3/rtplibraryversion.h>
using namespace jrtplib;

#include <string>
#include <stdio.h>
#include <string.h>
#define BUF_LEN (1024*1024)

struct PsMuxContext
{
    PsMuxContext();
    ~PsMuxContext();
    virtual void Process(guint8* buf, int len);

    void testMuxSpsPpsI(guint8* buf, int len);

    virtual void OnPsFrameOut(guint8* buf, int len, gint64 pts, gint64 dts) = 0;

private:
    Gb28181PsMux PsMux;
    StreamIdx Idx;
    guint8* pMuxBuf;
    gint64 pts;
    gint64 dts;
};

struct PsProcessRtp : public PsMuxContext
{
    PsProcessRtp(const char *ip, short port);
    ~PsProcessRtp();

    virtual void OnPsFrameOut(guint8* buf, int len, gint64 pts, gint64 dts);

    void checkerror(int rtperr);

    RTPSession sess;
    RTPUDPv4TransmissionParams transparams;
    RTPSessionParams sessparams;
};

struct PsProcessSaveFile : public PsMuxContext
{
    PsProcessSaveFile(std::string DstName);
    ~PsProcessSaveFile();

    virtual void OnPsFrameOut(guint8* buf, int len, gint64 pts, gint64 dts);

    FILE* fp;
};

#endif //UAC_PSMUXEXAMPLE_H
