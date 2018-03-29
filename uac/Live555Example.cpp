//
// Created by wu on 18-3-29.
//

#include "Live555Example.h"
#include <BasicUsageEnvironment.hh>
#include <liveMedia.hh>

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
                           char const* streamName, char const* inputFileName);


int main3(int argc, char* argv[])
{
    UsageEnvironment *env;
    char const* descriptionString = "Session streamed by \"testOnDemandRTSPServer\"";
    Boolean reuseFirstSource = False;
    Boolean iFramesOnly = False;

    RTSPServer* rtspServer = RTSPServer::createNew(*env, 8554);

    char const* streamName = "mpeg1or2AudioVideoTest";
    char const* inputFileName = "1.mpg";
    // NOTE: This *must* be a Program Stream; not an Elementary Stream
    ServerMediaSession* sms
            = ServerMediaSession::createNew(*env, streamName, streamName,
                                            descriptionString);
    MPEG1or2FileServerDemux* demux
            = MPEG1or2FileServerDemux::createNew(*env, inputFileName, reuseFirstSource);
    sms->addSubsession(demux->newVideoServerMediaSubsession(iFramesOnly));
    sms->addSubsession(demux->newAudioServerMediaSubsession());
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
}

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
                           char const* streamName, char const* inputFileName) {
    char* url = rtspServer->rtspURL(sms);
    UsageEnvironment& env = rtspServer->envir();
    env << "\n\"" << streamName << "\" stream, from the file \""
        << inputFileName << "\"\n";
    env << "Play this stream using the URL \"" << url << "\"\n";
    delete[] url;
}