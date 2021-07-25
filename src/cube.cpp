#include "cube.h"

namespace CubeObject {
	uint32_t get_size() {
        return sizeof(cube_vertices[0]) * cube_vertices.size();
    }
}