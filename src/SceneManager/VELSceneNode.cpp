#include "SceneManager/VELSceneNode.h"

namespace VEL
{
    struct MovableObjectNameExists {
        const String& name;
        bool operator()(const MovableObject* mo) {
            return mo->getName() == name;
        }
    };

    SceneNode::SceneNode(SceneManager* creator) : SceneNode(creator, BLANKSTRING)
    {
    }
    SceneNode::SceneNode(SceneManager* creator, const String& name)
        : Node(name)
        , mCreator(creator)
    {
        needUpdate();
    }
    SceneNode::~SceneNode()
    {
    }
    void SceneNode::attachObject(MovableObject* obj)
    {
        VELAssert(!obj->isAttached(), "Object already attached to a SceneNode or a Bone");

        // Also add to name index
        MovableObjectNameExists pred = { obj->getName() };
        ObjectMap::iterator it = std::find_if(mObjectsByName.begin(), mObjectsByName.end(), pred);
        VELAssert(it == mObjectsByName.end(), "Object was not attached because an object of the "
            "same name was already attached to this node.");

        mObjectsByName.push_back(obj);

        // Make sure bounds get updated (must go right to the top)
        needUpdate();
    }
    MovableObject* SceneNode::getAttachedObject(const String& name) const
    {
        // Look up 
        MovableObjectNameExists pred = { name };
        auto i = std::find_if(mObjectsByName.begin(), mObjectsByName.end(), pred);

        if (i == mObjectsByName.end())
        {
            VEL_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Attached object " +
                name + " not found.", "SceneNode::getAttachedObject");
        }

        return *i;
    }
    MovableObject* SceneNode::detachObject(unsigned short index)
    {
        VELAssert(index < mObjectsByName.size(), "out of bounds");
        ObjectMap::iterator i = mObjectsByName.begin();
        i += index;

        MovableObject* ret = *i;
        std::swap(*i, mObjectsByName.back());
        mObjectsByName.pop_back();

        ret->_notifyAttached((SceneNode*)0);

        // Make sure bounds get updated (must go right to the top)
        needUpdate();

        return ret;
    }
    void SceneNode::detachObject(MovableObject* obj)
    {
        ObjectMap::iterator i, iend;
        iend = mObjectsByName.end();
        for (i = mObjectsByName.begin(); i != iend; ++i)
        {
            if (*i == obj)
            {
                std::swap(*i, mObjectsByName.back());
                mObjectsByName.pop_back();
                break;
            }
        }
        obj->_notifyAttached((SceneNode*)0);

        // Make sure bounds get updated (must go right to the top)
        needUpdate();
    }
    MovableObject* SceneNode::detachObject(const String& name)
    {
        MovableObjectNameExists pred = { name };
        ObjectMap::iterator it = std::find_if(mObjectsByName.begin(), mObjectsByName.end(), pred);

        if (it == mObjectsByName.end())
        {
            VEL_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Object " + name + " is not attached "
                "to this node.", "SceneNode::detachObject");
        }

        MovableObject* ret = *it;
        std::swap(*it, mObjectsByName.back());
        mObjectsByName.pop_back();

        ret->_notifyAttached((SceneNode*)0);
        // Make sure bounds get updated (must go right to the top)
        needUpdate();

        return ret;
    }
    void SceneNode::detachAllObjects(void)
    {
        for (ObjectMap::iterator itr = mObjectsByName.begin(); itr != mObjectsByName.end(); ++itr)
        {
            (*itr)->_notifyAttached((SceneNode*)0);
        }
        mObjectsByName.clear();
        // Make sure bounds get updated (must go right to the top)
        needUpdate();
    }
}