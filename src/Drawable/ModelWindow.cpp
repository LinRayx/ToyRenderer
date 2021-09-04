#include "ModelWindow.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

namespace Draw
{
	ModelWindow::ModelWindow()
	{

	}



	bool ModelWindow::PushNode(Node& node)
	{
		const int selectedId = (pSelectedNode == nullptr) ? -1 : pSelectedNode->GetId();
		// build up flags for current node
		const auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow
			 | ((node.GetId() == selectedId) ? ImGuiTreeNodeFlags_Selected : 0)
			| (node.HasChild() ? 0 : ImGuiTreeNodeFlags_Leaf);
		// render this node
		const auto expanded = ImGui::TreeNodeEx(
			(void*)(intptr_t)node.GetId(),
			node_flags, node.GetName().c_str()
		);

		if (ImGui::IsItemClicked()) {
			pSelectedNode = &node;
		}
		return expanded;
	}

	void ModelWindow::PopNode(Node& node)
	{
		ImGui::TreePop();
	}

	void ModelWindow::SetModel(Model* model)
	{
		this->model = model;
	}
	bool ModelWindow::DrawUI()
	{
		if (model == nullptr) return false;
		ImGui::Begin(model->name.c_str());
		ImGui::Columns(2, nullptr, true);
		model->Accept(this);
		ImGui::NextColumn();

		bool dirty = false;
		if (pSelectedNode != nullptr)
		{
			const auto dcheck = [&dirty](bool changed) {dirty = dirty || changed; };
			auto& tf = ResolveTransform();
			ImGui::TextColored({ 0.4f,1.0f,0.6f,1.0f }, "Translation");
			dcheck(ImGui::SliderFloat("X", &tf.x, -60.f, 60.f));
			dcheck(ImGui::SliderFloat("Y", &tf.y, -60.f, 60.f));
			dcheck(ImGui::SliderFloat("Z", &tf.z, -60.f, 60.f));
			ImGui::TextColored({ 0.4f,1.0f,0.6f,1.0f }, "Orientation");
			dcheck(ImGui::SliderAngle("X-rotation", &tf.xRot, -180.0f, 180.0f));
			dcheck(ImGui::SliderAngle("Y-rotation", &tf.yRot, -180.0f, 180.0f));
			dcheck(ImGui::SliderAngle("Z-rotation", &tf.zRot, -180.0f, 180.0f));

			ImGui::TextColored({ 0.4f,1.0f,0.6f,1.0f }, "Scale");
			dcheck(ImGui::SliderFloat("X-scale", &tf.xScale, 0, 10.f));
			dcheck(ImGui::SliderFloat("Y-scale", &tf.yScale, 0, 10.f));
			dcheck(ImGui::SliderFloat("Z-scale", &tf.zScale, 0, 10.0f));
			dcheck(ImGui::SliderFloat("scale", &tf.scale, 0, 10.0f));

			if (dirty)
			{
				glm::mat4 imat = glm::mat4(1.0f);
				imat[1][1] = -1;
				imat = glm::translate(imat, glm::vec3(tf.x, tf.y, tf.z));
				imat = glm::rotate(imat, tf.xRot, glm::vec3(1, 0, 0));
				imat = glm::rotate(imat, tf.yRot, glm::vec3(0, 1, 0));
				imat = glm::rotate(imat, tf.zRot, glm::vec3(0, 0, 1));
				imat = glm::scale(imat, glm::vec3(tf.xScale, tf.yScale, tf.zScale) * glm::vec3(tf.scale));

				pSelectedNode->SetTransform(imat);
			}
			
			if (!pSelectedNode->HasChild())
			{
				dirty = pSelectedNode->AddMaterialUI();
			}

		}
		ImGui::End();

		return dirty;
	}
}