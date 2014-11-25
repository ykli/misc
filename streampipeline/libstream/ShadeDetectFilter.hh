#ifndef __SHADEDETECTFILTER_HH__
#define __SHADEDETECTFILTER_HH__

#include "ShadeDetect.hh"
#include "Filter.hh"
#include "WriteResult.hh"

class ShadeDetectFilter: public Filter, public WriteResult {
	public:
		static ShadeDetectFilter* createNew(int w, int h, char *ret_store_path);
		void doProcess(frame_t* frame_list, int nFrames, uint32_t *params);

	private:
		ShadeDetectFilter(int w, int h, char *ret_store_path);
		~ShadeDetectFilter();

		ShadeDetect *shadedetect;
};

#endif
