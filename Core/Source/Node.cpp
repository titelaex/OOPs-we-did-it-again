module Core.Node;
import Models.Card;
import <utility>;
import <memory>;
import <optional>;
import <functional>;
import Core.Node;

namespace Core {

Node::Node(std::unique_ptr<Models::Card> card)
    : m_card(std::move(card))
{
}

void Node::attachParent(const std::shared_ptr<Node>& parent)
{
    if (!parent) return;
    if (m_parent1.expired()) {
        m_parent1 = parent;
    }
    else if (m_parent2.expired()) {
        m_parent2 = parent;
    }
    else {
        m_parent2 = parent;
    }
}

void Node::setChild1(const std::shared_ptr<Node>& child)
{
    m_child1 = child;
    if (child) {
        child->attachParent(shared_from_this());
    }
}

void Node::setChild2(const std::shared_ptr<Node>& child)
{
    m_child2 = child;
    if (child) {
        child->attachParent(shared_from_this());
    }
}

std::optional<std::reference_wrapper<Models::Card>> Node::getCard()
{
	if (!m_card) return std::nullopt;
	return std::ref(*m_card);
}

std::optional<std::reference_wrapper<const Models::Card>> Node::getCard() const
{
    if (!m_card) return std::nullopt;
	return std::cref(*m_card);
}

Models::Card* Node::getCardRaw() const
{
    return m_card.get();
}

void Node::setCard(std::unique_ptr<Models::Card> card)
{
    m_card = std::move(card);
}

std::unique_ptr<Models::Card> Node::releaseCard()
{
    return std::move(m_card);
}

bool Node::isAvailable() const
{
    if (!m_card) return false;
    auto c1 = m_child1.lock();
    auto c2 = m_child2.lock();
    bool c1Empty = (!c1) || !c1->getCard().has_value();
    bool c2Empty = (!c2) || !c2->getCard().has_value();
    return c1Empty && c2Empty;
}

std::shared_ptr<Node> Node::getParent1() const {
		return m_parent1.lock();
	}

std::shared_ptr<Node> Node::getParent2() const {
		return m_parent2.lock();
	}

void Node::setParent1(const std::shared_ptr<Node>& p) {
		m_parent1 = p;
	}

void Node::setParent2(const std::shared_ptr<Node>& p) {
		m_parent2 = p;
	}

std::shared_ptr<Node> Node::getChild1() const {
		return m_child1.lock();
	}

std::shared_ptr<Node> Node::getChild2() const {
		return m_child2.lock();
	}

}
