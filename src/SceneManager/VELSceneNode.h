#ifndef VEL_SCENE_NODE_H
#define VEL_SCENE_NODE_H

#include "SceneManager/VELNode.h"
#include "SceneManager/VELSceneManager.h"
#include "Utils/VELPrerequisites.h"
#include "SceneManager/VELEntity.h"

namespace VEL
{
	class SceneNode : public Node
	{
	public:
		typedef std::vector<MovableObject*> ObjectMap;
	protected:
		ObjectMap mObjectsByName;

		SceneManager* mCreator;
		SceneNode* mAutoTrackTarget;

		size_t mGloableIndex;

		void updateFromParentImpl(void) const;

		/** See Node */
		void setParent(Node* parent);

		/// Tracking offset for fine tuning
		glm::vec3 mAutoTrackOffset;
		/// Local 'normal' direction vector
		glm::vec3 mAutoTrackLocalDirection;
		/// Fixed axis to yaw around
		glm::vec3 mYawFixedAxis;
	public:
		/** Constructor, only to be called by the creator SceneManager.
		@remarks
			Creates a node with a generated name.
		*/
		SceneNode(SceneManager* creator);
		/** Constructor, only to be called by the creator SceneManager.
		@remarks
			Creates a node with a specified name.
		*/
		SceneNode(SceneManager* creator, const String& name);
		~SceneNode();

		virtual void attachObject(MovableObject* obj);
		size_t numAttachedObjects(void) const { return mObjectsByName.size(); }
		MovableObject* getAttachedObject(size_t index) const { return mObjectsByName.at(index); }
		MovableObject* getAttachedObject(const String& name) const;

		virtual MovableObject* detachObject(unsigned short index);
		virtual void detachObject(MovableObject* obj);
		virtual MovableObject* detachObject(const String& name);
		virtual void detachAllObjects(void);
	};
}

#endif // !VEL_SCENE_NODE_H
