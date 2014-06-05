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
// A "ServerMediaSubsession" subclass for on-demand unicast streaming
// of MPEG-4 video from a WIS GO7007 capture device.
// C++ header

#ifndef _WIS_MPEG4_VIDEO_SERVER_MEDIA_SUBSESSION_HH
#define _WIS_MPEG4_VIDEO_SERVER_MEDIA_SUBSESSION_HH

#ifndef _WIS_SERVER_MEDIA_SUBSESSION_HH
#include "WISServerMediaSubsession.hh"
#endif

class WISMPEG4VideoServerMediaSubsession: public WISServerMediaSubsession {
public:
  static WISMPEG4VideoServerMediaSubsession*
  createNew(UsageEnvironment& env, WISInput& wisInput, unsigned estimatedBitrate);

  void setDoneFlag() { fDoneFlag = ~0; }
  void checkForAuxSDPLine1();

private:
  WISMPEG4VideoServerMediaSubsession(UsageEnvironment& env, WISInput& wisInput,
				     unsigned estimatedBitrate);
      // called only by createNew()
  virtual ~WISMPEG4VideoServerMediaSubsession();

private: // redefined virtual functions
  virtual char const* getAuxSDPLine(RTPSink* rtpSink,
				    FramedSource* inputSource);
  virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
					      unsigned& estBitrate);
  virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                    unsigned char rtpPayloadTypeIfDynamic,
				    FramedSource* inputSource);

private:
  char fDoneFlag; // used when setting up 'SDPlines'
  RTPSink* fDummyRTPSink; // ditto
};

#endif
