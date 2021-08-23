#ifndef DRAW_MODEL_WINDOW_H
#define DRAW_MODEL_WINDOW_H

#include "Drawable/Model.h"
#include "Drawable/ModelWindowBase.h"
#include "imgui.h"

namespace Draw
{
	class ModelWindow : public ModelWindowBase
	{
	public:
		ModelWindow();
		bool PushNode(Node& node);
		void PopNode(Node& node);
		void SetModel(Model* model);
	private:
		Node* pSelectedNode = nullptr;
	};
}

#endif // !DRAW_MODEL_WINDOW_H
