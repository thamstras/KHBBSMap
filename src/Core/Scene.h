#pragma once
#include "Math/Transform.h"
#include <vector>
#include <memory>

namespace Core
{
	class Scene
	{
	public:
		struct Node
		{
		private:
			friend class Scene;
			Scene* pScene;
			Node* pParent;
			std::vector<Node*> pChildren;
			void RenderChildren(glm::mat4 parentWorldTransform);
		public:
			Math::Transform transform;
			virtual ~Node();
			virtual void Update();
			virtual void Render();
		};

	private:
		std::vector<std::unique_ptr<Node>> Nodes;
		Node* pRootNode;

	public:
		void AddNode(Node* pNode, Node* pParent);
		void ReplaceRoot(Node* pNode);
		void ReParent(Node* pTarget, Node* newParent);

		void Update();
		void Render();
	};
}