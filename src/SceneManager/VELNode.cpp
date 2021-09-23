#include "SceneManager/VELNode.h"

namespace VEL
{
	Node::Node() : Node(BLANKSTRING) {}

	Node::Node(const String& name) 
		: mParent(0),
		mName(name)
	{
		needUpdate();
	}

	Node::~Node()
	{
	}

	void Node::needUpdate(bool forceParentUpdate)
	{
		mNeedParentUpdate = true;
		mNeedChildUpdate = true;
		mCachedTransformOutOfDate = true;

		// Make sure we're not root and parent hasn't been notified before
		if (mParent && (!mParentNotified || forceParentUpdate))
		{
			mParent->requestUpdate(this, forceParentUpdate);
			mParentNotified = true;
		}

		// all children will be updated
		mChildrenToUpdate.clear();
	}

	void Node::requestUpdate(Node* child, bool forceParentUpdate)
	{
		// If we're already going to update everything this doesn't matter
		if (mNeedChildUpdate)
		{
			return;
		}

		mChildrenToUpdate.insert(child);
		// Request selective update of me, if we didn't do it before
		if (mParent && (!mParentNotified || forceParentUpdate))
		{
			mParent->requestUpdate(this, forceParentUpdate);
			mParentNotified = true;
		}
	}

}