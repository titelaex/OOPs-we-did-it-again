export module Node;

import Card;
import <memory>;
import <utility>;

export namespace Core {
	class Node {
	private:
		std::unique_ptr<Models::Card> m_card;
		std::unique_ptr<Node> m_child1;
		std::unique_ptr<Node> m_child2;
	public:
		Node(const Models::Card& card,
			 std::unique_ptr<Node> child1 = nullptr,
			 std::unique_ptr<Node> child2 = nullptr)
			: m_card(std::make_unique<Models::Card>(card)),
			  m_child1(std::move(child1)),
			  m_child2(std::move(child2)) {}

		Node(Models::Card&& card,
			 std::unique_ptr<Node> child1 = nullptr,
			 std::unique_ptr<Node> child2 = nullptr)
			: m_card(std::make_unique<Models::Card>(std::move(card))),
			  m_child1(std::move(child1)),
			  m_child2(std::move(child2)) {}

		Node(const Node& other)
			: m_card(other.m_card ? std::make_unique<Models::Card>(*other.m_card) : nullptr),
			  m_child1(other.m_child1 ? std::make_unique<Node>(*other.m_child1) : nullptr),
			  m_child2(other.m_child2 ? std::make_unique<Node>(*other.m_child2) : nullptr) {}

		Node& operator=(const Node& other) {
			if (this != &other) {
				m_card = other.m_card ? std::make_unique<Models::Card>(*other.m_card) : nullptr;
				m_child1 = other.m_child1 ? std::make_unique<Node>(*other.m_child1) : nullptr;
				m_child2 = other.m_child2 ? std::make_unique<Node>(*other.m_child2) : nullptr;
			}
			return *this;
		}

		Node(Node&&) noexcept = default;
		Node& operator=(Node&&) noexcept = default;

		virtual ~Node() = default;
		virtual void display() const = 0;
	};
} 