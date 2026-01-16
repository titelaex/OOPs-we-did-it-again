export module Core.Node;
import Models.Card;
import <memory>;
import <utility>;
export namespace Core {
	class Node : public std::enable_shared_from_this<Node> {
	private:
		std::shared_ptr<Models::Card> m_card;
		std::weak_ptr<Node> m_child1;
		std::weak_ptr<Node> m_child2;
		std::weak_ptr<Node> m_parent1;
		std::weak_ptr<Node> m_parent2;
		void attachParent(const std::shared_ptr<Node>& parent);
	public:
		Node(std::shared_ptr<Models::Card> card);
		Node(const Node&) = delete;
		Node& operator=(const Node&) = delete;
		Node(Node&&) noexcept = default;
		Node& operator=(Node&&) noexcept = default;
		virtual ~Node() = default;
		std::shared_ptr<Node> getParent1() const { return m_parent1.lock(); }
		std::shared_ptr<Node> getParent2() const { return m_parent2.lock(); }
		void setParent1(const std::shared_ptr<Node>& p) { m_parent1 = p; }
		void setParent2(const std::shared_ptr<Node>& p) { m_parent2 = p; }
		std::shared_ptr<Node> getChild1() const { return m_child1.lock(); }
		std::shared_ptr<Node> getChild2() const { return m_child2.lock(); }
		void setChild1(const std::shared_ptr<Node>& child);
		void setChild2(const std::shared_ptr<Node>& child);
		std::shared_ptr<const Models::Card> getCard() const;
		void setCard(std::shared_ptr<Models::Card> card);
		std::shared_ptr<Models::Card> releaseCard();
		bool isAvailable() const;
	};
} } 