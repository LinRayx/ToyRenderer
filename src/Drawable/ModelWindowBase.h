#ifndef DRAW_MODEL_WINDOW_BASE_H
#define DRAW_MODEL_WINDOW_BASE_H

namespace Draw
{
	class ModelWindowBase
	{
	public:
		virtual bool PushNode(class Node& node) = 0;
		virtual void PopNode(class Node& node) = 0;
		virtual bool DrawUI() = 0;
	};
}

#endif // !DRAW_MODEL_WINDOW_BASE_H
