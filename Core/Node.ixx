export module Node;

import Card;
import <memory>; // Add this line to import std::unique_ptr

export namespace Core {
	class Node {
	private:
		std::unique_ptr<Card> m_card;
		std::unique_ptr<Node> m_child1;
		std::unique_ptr<Node> m_child2;
	public:
		Node(Card card, Node child1, Node child2)
		{
			m_card = std::make_unique<Card>(card);
			m_child1 = std::make_unique<Node>(child1);
			m_child2 = std::make_unique<Node>(child2);
		}

		Node(Card card) 
		{
			m_card = std::make_unique<Card>(card);
			m_child1 = nullptr;
			m_child2 = nullptr;
		}

		Node(Card card, Node child1) 
		{
			m_card = std::make_unique<Card>(card);
			m_child1 = std::make_unique<Node>(child1);
			m_child2 = nullptr;
		}

		Node(Card card, Node child2)
		{
			m_card = std::make_unique<Card>(card);
			m_child1 = nullptr;
			m_child2 = std::make_unique<Node>(child2);
		}
		virtual ~Node() = default;
		virtual void display() const = 0;
	};
} 