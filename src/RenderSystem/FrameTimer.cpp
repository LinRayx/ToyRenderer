#include "FrameTimer.h"
#include <GLFW/glfw3.h>

//! How many past frame times are used to compute the median
#define FRAME_TIME_COUNT 100

//! A ring buffer of glfwGetTime() values in past invocations of 
//! record_frame_time(). Invalid entries are zero.
static double g_recorded_times[FRAME_TIME_COUNT] = { 0.0 };
//! The most recently written entry in the ring buffer record_times
static uint32_t g_recorded_time_index = FRAME_TIME_COUNT - 1;

int compare_floats(const void* lhs_pointer, const void* rhs_pointer) {
	float lhs = *((float*)lhs_pointer);
	float rhs = *((float*)rhs_pointer);
	return  (lhs < rhs) ? -1 : ((lhs == rhs) ? 0 : 1);
}

void RenderSystem::FrameTimer::Record()
{
	++g_recorded_time_index;
	if (g_recorded_time_index >= FRAME_TIME_COUNT)
		g_recorded_time_index -= FRAME_TIME_COUNT;
	g_recorded_times[g_recorded_time_index] = glfwGetTime();
}

float RenderSystem::FrameTimer::Get()
{
	// List valid frame times from previous frames
	float frame_times[FRAME_TIME_COUNT];
	float recorded_sum = 0.0f;
	uint32_t recorded_count = 0;
	for (int32_t i = 0; i != FRAME_TIME_COUNT - 1; ++i) {
		int32_t lhs = (g_recorded_time_index + FRAME_TIME_COUNT - i) % FRAME_TIME_COUNT;
		int32_t rhs = (g_recorded_time_index + FRAME_TIME_COUNT - i - 1) % FRAME_TIME_COUNT;
		if (g_recorded_times[lhs] != 0.0 && g_recorded_times[rhs] != 0.0) {
			frame_times[recorded_count] = (float)(g_recorded_times[lhs] - g_recorded_times[rhs]);
			recorded_sum += frame_times[recorded_count];
			++recorded_count;
		}
	}
	if (recorded_count == 0)
		return 0.0f;
	// Sort
	qsort(frame_times, recorded_count, sizeof(frame_times[0]), compare_floats);
	// Compare the median to the mean and warn if there is a big discrepancy
	float median = frame_times[recorded_count / 2];
	float mean = recorded_sum / recorded_count;
	//if (median < mean * 0.96f || median > mean * 1.04f)
	//	printf("Warning: Frame time median is %.2f ms but frame time mean is %.2f ms.\n", median * 1000.0f, mean * 1000.0f);
	// Return the median
	return median;
}
