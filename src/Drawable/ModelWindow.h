#ifndef DRAW_MODEL_WINDOW_H
#define DRAW_MODEL_WINDOW_H

#include "Drawable/Model.h"
#include "Drawable/ModelWindowBase.h"
#include "imgui.h"
#include <map>
#include "Utils/GloableClass.h"

namespace Draw
{
	class ModelWindow : public ModelWindowBase
	{
	public:
		ModelWindow();
		bool PushNode(Node& node);
		void PopNode(Node& node);
		void SetModel(Model* model);
		bool DrawUI();
	private:
		Model* model = nullptr;

		Node* pSelectedNode = nullptr;

		struct TransformParameters
		{
			float xRot = 0.0f;
			float yRot = 0.0f;
			float zRot = 0.0f;
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
			float xScale = 1.f;
			float yScale = 1.f;
			float zScale = 1.f;
			float scale = 1.f;
		};
		std::map<int, TransformParameters> transformParams;

		TransformParameters& ResolveTransform() noexcept
		{
			const auto id = pSelectedNode->GetId();
			auto i = transformParams.find(id);
			if (i == transformParams.end())
			{
				return LoadTransform(id);
			}
			return i->second;
		}

		TransformParameters& LoadTransform(int id) noexcept
		{
			const auto& applied = pSelectedNode->GetTransform();
			const auto angles = Gloable::ExtractEulerAngles(applied);
			const auto translation = Gloable::ExtractTranslation(applied);
			TransformParameters tp;
			tp.zRot = angles.z;
			tp.xRot = angles.x;
			tp.yRot = angles.y;
			tp.x = translation.x;
			tp.y = translation.y;
			tp.z = translation.z;
			tp.xScale = 1.f;
			tp.yScale = 1.f;
			tp.zScale = 1.f;
			tp.scale = 1.f;
			return transformParams.insert({ id,{ tp } }).first->second;
		}
	};
}

#endif // !DRAW_MODEL_WINDOW_H
