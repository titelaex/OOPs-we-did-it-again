module Core.Node;
import Models.Card;
import <utility>;
import <memory>;
import Core.Node;

namespace Core {

Node::Node(std::shared_ptr<Models::Card> card)
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

std::shared_ptr<const Models::Card> Node::getCard() const
{
    return m_card;
}

void Node::setCard(std::shared_ptr<Models::Card> card)
{
    m_card = std::move(card);
}

std::shared_ptr<Models::Card> Node::releaseCard()
{
    return std::move(m_card);
}

bool Node::isAvailable() const
{
    if (!m_card) return false;
    auto c1 = m_child1.lock();
    auto c2 = m_child2.lock();
    bool c1Empty = (!c1) || (c1->getCard() == nullptr);
    bool c2Empty = (!c2) || (c2->getCard() == nullptr);
    return c1Empty && c2Empty;
}

std::shared_ptr<Node> Node::getParent1() const
{
    return m_parent1.lock();
}

std::shared_ptr<Node> Node::getParent2() const
{
    return m_parent2.lock();
}

std::shared_ptr<Node> Node::getChild1() const
{
    return m_child1;
}

std::shared_ptr<Node> Node::getChild2() const
{
    return m_child2;
}

}
