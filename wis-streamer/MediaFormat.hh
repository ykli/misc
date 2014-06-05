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
// Allowed video and audio codecs for streaming from a WIS GO7007 capture device.
// C++ header

#ifndef _MEDIA_FORMAT_HH
#define _MEDIA_FORMAT_HH

enum VideoFormat {
  VFMT_NONE,
  VFMT_H264,
  VFMT_MJPEG,
  VFMT_MPEG1,
  VFMT_MPEG2,
  VFMT_MPEG4
};

enum AudioFormat {
  AFMT_NONE,
  AFMT_PCM_RAW16,
  AFMT_PCM_ULAW,
  AFMT_MPEG2,
  AFMT_AMR,
  AFMT_AAC
};

enum PackageFormat {
  PFMT_SEPARATE_STREAMS, // separate streams for audio & video
  PFMT_TRANSPORT_STREAM // combine audio and video into a Transport Stream
};

#endif
