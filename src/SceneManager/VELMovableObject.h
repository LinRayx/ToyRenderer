#ifndef VEL_MOVABLE_OBJECT_H
#define VEL_MOVABLE_OBJECT_H

#include "Utils/VELCommand.h"
#include "MemoryManager/VELMemoryAllocatorConfig.h"

namespace VEL
{
	class MovableObjectFactory;

	class MovableObject
	{
	public:
		class Listener
		{
		public:
			Listener(void) {}
			virtual ~Listener() {}
			/** MovableObject is being destroyed */
			virtual void objectDestroyed(MovableObject*) {}
			/** MovableObject has been attached to a node */
			virtual void objectAttached(MovableObject*) {}
			/** MovableObject has been detached from a node */
			virtual void objectDetached(MovableObject*) {}
			/** MovableObject has been moved */
			virtual void objectMoved(MovableObject*) {}
		};

		/** Returns the name of this object. */
		const String& getName(void) const { return mName; }

	protected:
		String mName;
		Node* mParentNode;
		bool mParentIsTagPoint : 1;
		mutable ulong mLightListUpdated;
		Listener* mListener;
		MovableObjectFactory* mCreator;
		SceneManager* mManager;
	public:

		bool isAttached(void) const { return (mParentNode != NULL); }
		virtual void _notifyAttached(Node* parent, bool isTagPoint = false);
		virtual void _notifyCreator(MovableObjectFactory* fact) { mCreator = fact; }
		virtual void _notifyManager(SceneManager* man) { mManager = man; }
	};

	class MovableObjectFactory : public MovableAlloc
	{
	private:
		/// Type flag, allocated if requested
		uint32 mTypeFlag;

		/// Internal implementation of create method - must be overridden
		virtual MovableObject* createInstanceImpl(
			const String& name, const NameValuePairList* params = 0) = 0;
	public:
		MovableObjectFactory() : mTypeFlag(0xFFFFFFFF) {}
		virtual ~MovableObjectFactory() {}

		MovableObject* createInstance(
			const String& name, SceneManager* manager,
			const NameValuePairList* params = 0);
	};
}

#endif // !VEL_MOVABLE_OBJECT_H
