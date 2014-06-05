/*
 * Copyright (C) 2005-2006 WIS Technologies International Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and the associated README documentation file (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
// An application that streams audio/video captured by a WIS GO7007,
// using a built-in RTSP server.
// main program

#include <BasicUsageEnvironment.hh>
#include <getopt.h>
#include "Options.hh"
#include "Err.hh"
#include "UnicastStreaming.hh"
#include "MulticastStreaming.hh"
#include "DarwinStreaming.hh"

int main(int argc, char** argv) {
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

  // Print an introduction message:
  *env << "wis-streamer: A streaming server for the WIS GO7007 encoder driver\n";
  *env << "\tFor driver documentation and source code, see: <http://oss.wischip.com/>\n";
  *env << "\tFor server documentation and source code, see: <http://www.live555.com/wis-streamer/>\n";
  *env << "\tBuilt using \"LIVE555 Streaming Media\": <http://www.live555.com/liveMedia/>\n";

  // Parse command-line options:
  checkArgs(*env, argc, argv); 
  
  *env << "Initializing...\n";

  // Initialize the WIS input device:
  WISInput* inputDevice = WISInput::createNew(*env);
  if (inputDevice == NULL) {
    err(*env) << "Failed to create WIS input device\n";
    exit(1);
  }

  // Create the RTSP server:
  RTSPServer* rtspServer = NULL;
  // Normal case: Streaming from a built-in RTSP server:
  rtspServer = RTSPServer::createNew(*env, rtspServerPortNum, authDB);
  if (rtspServer == NULL) {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }

  *env << "...done initializing\n";

  // Create a record describing the media to be streamed:
  ServerMediaSession* sms
    = ServerMediaSession::createNew(*env, "", NULL, streamDescription,
									  streamingMode == STREAMING_MULTICAST_SSM);

  // Configure it for unicast or multicast streaming:
  if (streamingMode == STREAMING_UNICAST) {
	setupUnicastStreaming(*inputDevice, sms);
  } else {
//    setupMulticastStreaming(*inputDevice, sms);
  }

  rtspServer->addServerMediaSession(sms);
  char *url = rtspServer->rtspURL(sms);
  *env << "Play this stream using the URL:\n\t" << url << "\n";
  delete[] url;

  // Begin the LIVE555 event loop:
  env->taskScheduler().doEventLoop(); // does not return

  // If "doEventLoop()" *did* return, we'd now do this to reclaim resources before exiting:
#if 0
  if (streamingMode != STREAMING_UNICAST && streamingMode != STREAMING_UNICAST_THROUGH_DARWIN) {
    reclaimMulticastStreaming();
  }
#endif
  Medium::close(rtspServer); // will also reclaim "sms" and its "ServerMediaSubsession"s
  Medium::close(inputDevice);
  reclaimArgs();
  env->reclaim();
  delete scheduler;

  return 0; // only to prevent compiler warning
}
