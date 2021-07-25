#pragma once

#ifdef __cplusplus
extern "C" {
#endif


	//! Invoke this function exactly once per frame to record the current time.
	//! Only then the other functions defined in this header will be available.
	void record_frame_time();

	//! Retrieves the current estimate of the frame time in seconds. It is the
	//! median of a certain number of previously recorded frame times.
	float get_frame_time();

#ifdef __cplusplus
}
#endif
