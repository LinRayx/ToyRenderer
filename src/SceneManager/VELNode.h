#ifndef VEL_NODE_H
#define VEL_NODE_H

#include "MemoryManager/VELMemoryAllocatorConfig.h"
#include "Utils/VELCommand.h"
#include <glm/glm.hpp>

namespace VEL
{
	class Node : public NodeAlloc
	{
        enum TransformSpace
        {
            /// Transform is relative to the local space
            TS_LOCAL,
            /// Transform is relative to the space of the parent node
            TS_PARENT,
            /// Transform is relative to world space
            TS_WORLD
        };
        typedef std::vector<Node*> ChildNodeMap;

        /** Listener which gets called back on Node events.
        */
        class Listener
        {
        public:
            Listener() {}
            virtual ~Listener() {}
            /** Called when a node gets updated.
            @remarks
                Note that this happens when the node's derived update happens,
                not every time a method altering it's state occurs. There may
                be several state-changing calls but only one of these calls,
                when the node graph is fully updated.
            */
            virtual void nodeUpdated(const Node*) {}
            /** Node is being destroyed */
            virtual void nodeDestroyed(const Node*) {}
            /** Node has been attached to a parent */
            virtual void nodeAttached(const Node*) {}
            /** Node has been detached from a parent */
            virtual void nodeDetached(const Node*) {}
        };
    protected:
        /// Pointer to parent node
        Node* mParent;
        /// Collection of pointers to direct children
        ChildNodeMap mChildren;

        typedef std::set<Node*> ChildUpdateSet;
        /// List of children which need updating, used if self is not out of date but children are
        ChildUpdateSet mChildrenToUpdate;
        /// Friendly name of this node
        String mName;

        /// Flag to indicate own transform from parent is out of date
        mutable bool mNeedParentUpdate : 1;
        /// Flag indicating that all children need to be updated
        bool mNeedChildUpdate : 1;
        /// Flag indicating that parent has been notified about update request
        bool mParentNotified : 1;
        /// Flag indicating that the node has been queued for update
        bool mQueuedForUpdate : 1;
        /// Stores whether this node inherits orientation from it's parent
        bool mInheritOrientation : 1;
        /// Stores whether this node inherits scale from it's parent
        bool mInheritScale : 1;
        mutable bool mCachedTransformOutOfDate : 1;
    public:
        /// Constructor, should only be called by parent, not directly.
        Node();
        /** Constructor, should only be called by parent, not directly.
        @remarks
            Assigned a name.
        */
        Node(const String& name);

        virtual ~Node();

        /** Returns the name of the node. */
        const String& getName(void) const { return mName; }

        /** Gets this node's parent (NULL if this is the root).
        */
        Node* getParent(void) const { return mParent; }

        virtual void needUpdate(bool forceParentUpdate = false);
        void requestUpdate(Node* child, bool forceParentUpdate = false);
	};
}

#endif // !VEL_NODE_H
