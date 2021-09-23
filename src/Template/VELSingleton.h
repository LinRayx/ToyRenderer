#ifndef VEL_SINGLETON_H
#define VEL_SINGLETON_H

#include "Exception/VELException.h"

namespace VEL
{
	template< typename T > class Singleton
	{
	private:
		Singleton(const Singleton<T>&);
		Singleton& operator=(const Singleton<T>&);
	protected:
		static T* msSingleton;
	public:
		Singleton(void)
		{
			VELAssert(!msSingleton, "There can be only one singleton");
			msSingleton = static_cast<T*>(this);
		}
		~Singleton()
		{
			assert(msSingleton);
			msSingleton = 0;
		}
		/// Get the singleton instance
		static T& getSingleton(void)
		{
			assert(msSingleton);
			return (*msSingleton);
		}
		/// @copydoc getSingleton
		static T* getSingletonPtr(void) { return msSingleton; }
	};
}

#endif // !VEL_SINGLETON_H
