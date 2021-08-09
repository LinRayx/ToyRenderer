#ifndef FRAME_TIMER_H
#define FRAME_TIMER_H

#include <iostream>

namespace RenderSystem
{
#define FRAME_TIME_COUNT 100
	class FrameTimer
	{
	public:
		void Record();
		float Get();

	private:
		double g_recorded_times[FRAME_TIME_COUNT] = { 0.0 };
		uint32_t g_recorded_time_index = FRAME_TIME_COUNT - 1;
	};
}

#endif // !FRAME_TIMER_H
