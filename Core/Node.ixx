export module Core.Node;

import Models.Card;
import <memory>;
import <utility>;

export namespace Core {
	class Node {
	private:
		std::unique_ptr<Models::Card> m_card;
		Node* m_child1 = nullptr;
		Node* m_child2 = nullptr;
		Node* m_parent1 = nullptr;
		Node* m_parent2 = nullptr;
	public:
		Node(std::unique_ptr<Models::Card> card,
			 Node* child1 = nullptr,
			 Node* child2 = nullptr);

		Node(const Node&) = delete;
		Node& operator=(const Node&) = delete;
		Node(Node&&) noexcept = default;
		Node& operator=(Node&&) noexcept = default;

		virtual ~Node() = default;

		Node* getParent1() const { return m_parent1; }
		Node* getParent2() const { return m_parent2; }
		void setParent1(Node* p) { m_parent1 = p; }
		void setParent2(Node* p) { m_parent2 = p; }

		Node* getChild1() const { return m_child1; }
		Node* getChild2() const { return m_child2; }

		void setChild1(Node* child) { m_child1 = child; if (child) { if (child->getParent1() == nullptr) child->setParent1(this); else child->setParent2(this); } }
		void setChild2(Node* child) { m_child2 = child; if (child) { if (child->getParent1() == nullptr) child->setParent1(this); else child->setParent2(this); } }

		Models::Card* getCard() const { return m_card.get(); }
		void setCard(std::unique_ptr<Models::Card> card) { m_card = std::move(card); }

		virtual void display() const { }
	};
} 