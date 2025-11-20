export module Core.Node;

import Models.Card;
import <memory>;
import <utility>;

export namespace Core {
	class Node {
	private:
		std::shared_ptr<Models::Card> m_card; // shared to allow external creation
		std::unique_ptr<Node> m_child1;
		std::unique_ptr<Node> m_child2;
	public:
		Node(std::shared_ptr<Models::Card> card,
			 std::unique_ptr<Node> child1 = nullptr,
			 std::unique_ptr<Node> child2 = nullptr)
			: m_card(std::move(card)),
			  m_child1(std::move(child1)),
			  m_child2(std::move(child2)) {}

		Node(const Node&) = delete; // abstract: no copy
		Node& operator=(const Node&) = delete;
		Node(Node&&) noexcept = default;
		Node& operator=(Node&&) noexcept = default;

		virtual ~Node() = default;
		virtual void display() const = 0;
	};
} 