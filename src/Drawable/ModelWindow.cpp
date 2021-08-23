#include "ModelWindow.h"

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
		ImGui::Begin("Model");
		ImGui::Columns(2, nullptr, true);
		model->Accept(this);
		ImGui::End();
	}
}