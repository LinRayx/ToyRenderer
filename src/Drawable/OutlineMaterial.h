#ifndef OUTLINE_MATERIAL
#define OUTLINE_MATERIAL

#include "Drawable/MaterialBase.h"

namespace Draw
{
	class OutlineMaterial : public MaterialBase
	{
	public:
		OutlineMaterial();
		virtual void Compile();
	};
}

#endif // !OUTLINE_MATERIAL
