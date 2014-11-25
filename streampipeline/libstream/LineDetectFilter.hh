#ifndef __LINEDETECTFILTER_HH__
#define __LINEDETECTFILTER_HH__

#include "LineDetect.hh"
#include "Filter.hh"
#include "WriteResult.hh"

class LineDetectFilter: public Filter, public WriteResult {
	public:
		static LineDetectFilter* createNew(int w, int h, char *ret_store_path);
		void doProcess(frame_t* frame_list, int nFrames, uint32_t *params);

	private:
		LineDetectFilter(int w, int h, char *ret_store_path);
		~LineDetectFilter();

		LineDetect *linedetect;
};

#endif
