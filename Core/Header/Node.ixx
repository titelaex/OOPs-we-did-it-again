export module Core.Node;
import Models.Card;
import <memory>;
import <utility>;
import <optional>;
import <functional>;
export namespace Core {
	class Node : public std::enable_shared_from_this<Node> {
	private:
		std::unique_ptr<Models::Card> m_card;
		std::weak_ptr<Node> m_child1;
		std::weak_ptr<Node> m_child2;
		std::weak_ptr<Node> m_parent1;
		std::weak_ptr<Node> m_parent2;
		void attachParent(const std::shared_ptr<Node>& parent);
	public:
		Node(std::unique_ptr<Models::Card> card);
		Node(const Node&) = delete;
		Node& operator=(const Node&) = delete;
		Node(Node&&) noexcept = default;
		Node& operator=(Node&&) noexcept = default;
		virtual ~Node() = default;
		std::shared_ptr<Node> getParent1() const;
		std::shared_ptr<Node> getParent2() const;
		void setParent1(const std::shared_ptr<Node>& p);
		void setParent2(const std::shared_ptr<Node>& p);
		std::shared_ptr<Node> getChild1() const;
		std::shared_ptr<Node> getChild2() const;
		void setChild1(const std::shared_ptr<Node>& child);
		void setChild2(const std::shared_ptr<Node>& child);
		std::optional<std::reference_wrapper<Models::Card>> getCard();
		std::optional<std::reference_wrapper<const Models::Card>> getCard() const;
		Models::Card* getCardRaw() const; 
		void setCard(std::unique_ptr<Models::Card> card);
		std::unique_ptr<Models::Card> releaseCard();
		bool isAvailable() const;
	};
}