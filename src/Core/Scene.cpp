#include "Scene.h"

using namespace Core;

void Scene::Update()
{
	for (auto& node : Nodes)
		node->Update();
}

void Scene::Render()
{
	pRootNode->RenderChildren(glm::mat4(1.0f));
}

void Scene::AddNode(Node* pNode, Node* pParent)
{
	// TODO: if (pNode == nullptr || pParent == nullptr) throw;
	// TODO: if (pParent->pScene != this) throw;
	// TODO: if (pNode->pScene != nullptr) throw;

	Nodes.push_back(std::unique_ptr<Node>(pNode));
	pNode->pScene = this;
	pNode->pParent = pParent;
	pParent->pChildren.push_back(pNode);
}

void Scene::Node::RenderChildren(glm::mat4 parentWorldTransform)
{
	glm::mat4 worldTransform = this->transform.Matrix() * parentWorldTransform;
	// TODO: this->Render(RenderContext, worldTransform);
	this->Render();
	for (auto& child : pChildren)
		child->RenderChildren(worldTransform);
}