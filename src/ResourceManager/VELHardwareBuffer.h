#ifndef VEL_HARDWARE_BUFFER_H
#define VEL_HARDWARE_BUFFER_H
#include "MemoryManager/VELMemoryAllocatorConfig.h"
#include "Utils/VELCommand.h"

namespace VEL
{
    /// Enums describing buffer usage
    enum HardwareBufferUsage : uint8
    {
        /** Memory mappable on host and cached
         * @par Usage
         * results of some computations, e.g. screen capture
         */
        HBU_GPU_TO_CPU = 1,
        /** CPU (system) memory
         * This is the least optimal buffer setting.
         * @par Usage
         * Staging copy of resources used as transfer source.
         */
         HBU_CPU_ONLY = 2,
         /** Indicates the application will never read the contents of the buffer back,
         it will only ever write data. Locking a buffer with this flag will ALWAYS
         return a pointer to new, blank memory rather than the memory associated
         with the contents of the buffer; this avoids DMA stalls because you can
         write to a new memory area while the previous one is being used.

         However, you may read from it¡¯s shadow buffer if you set one up
         */
         HBU_DETAIL_WRITE_ONLY = 4,
         /** Device-local GPU (video) memory. No need to be mappable on host.
          * This is the optimal buffer usage setting.
          * @par Usage
          * Resources transferred from host once (immutable) - e.g. most textures, vertex buffers
          */
          HBU_GPU_ONLY = HBU_GPU_TO_CPU | HBU_DETAIL_WRITE_ONLY,
          /** Mappable on host and preferably fast to access by GPU.
           * @par Usage
           * Resources written frequently by host (dynamic) - e.g. uniform buffers updated every frame
           */
           HBU_CPU_TO_GPU = HBU_CPU_ONLY | HBU_DETAIL_WRITE_ONLY,
    };

	class HardwareBuffer : public BufferAlloc
	{
	public:
		typedef uint8 Usage;

        /// Rather use HardwareBufferUsage
        enum UsageEnum
        {
            /// same as #HBU_GPU_TO_CPU
            HBU_STATIC = HBU_GPU_TO_CPU,
            /// same as #HBU_CPU_ONLY
            HBU_DYNAMIC = HBU_CPU_ONLY,
            /// @deprecated use #HBU_DETAIL_WRITE_ONLY
            HBU_WRITE_ONLY = HBU_DETAIL_WRITE_ONLY,
            /// @deprecated do not use
            HBU_DISCARDABLE = 8,
            /// same as #HBU_GPU_ONLY
            HBU_STATIC_WRITE_ONLY = HBU_GPU_ONLY,
            /// same as #HBU_CPU_TO_GPU
            HBU_DYNAMIC_WRITE_ONLY = HBU_CPU_TO_GPU,
            /// @deprecated do not use
            HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE = HBU_CPU_TO_GPU,
        };
	};
}

#endif // !VEL_HARDWARE_BUFFER_H
