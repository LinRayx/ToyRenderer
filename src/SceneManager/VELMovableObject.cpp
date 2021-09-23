#include "SceneManager/VELMovableObject.h"

namespace VEL
{
	void MovableObject::_notifyAttached(Node* parent, bool isTagPoint)
	{
		assert(!parent || !mParentNode);

		mParentNode = parent;
		mParentIsTagPoint = isTagPoint;
        bool different = (parent != mParentNode);

        // Mark light list being dirty, simply decrease
        // counter by one for minimise overhead
        --mLightListUpdated;

        // Call listener (note, only called if there's something to do)
        if (mListener && different)
        {
            if (mParentNode)
                mListener->objectAttached(this);
            else
                mListener->objectDetached(this);
        }
	}
    MovableObject* MovableObjectFactory::createInstance(const String& name, SceneManager* manager, const NameValuePairList* params)
    {
        MovableObject* m = createInstanceImpl(name, params);
        m->_notifyCreator(this);
        m->_notifyManager(manager);
        return m;
    }
}
