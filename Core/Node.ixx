
export module Core.Node;

import Models.Card;
import <memory>;
import <utility>;

export namespace Core {
	class Node {
	private:
		std::unique_ptr<Models::Card> m_card;
		std::unique_ptr<Node> m_child1;
		std::unique_ptr<Node> m_child2;
	public:
		Node(std::unique_ptr<Models::Card> card,
			 std::unique_ptr<Node> child1 = nullptr,
			 std::unique_ptr<Node> child2 = nullptr);

		Node(const Node&) = delete;
		Node& operator=(const Node&) = delete;
		Node(Node&&) noexcept = default;
		Node& operator=(Node&&) noexcept = default;

		virtual ~Node() = default;
	};
} 