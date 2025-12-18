module Core.Node;

import Models.Card;
import <utility>;
import <memory>;

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

std::unique_ptr<Models::Card> Node::releaseCard()
{
    return std::move(m_card);
}

bool Node::isAvailable() const
{
    if (!m_card) return false;
    auto p1 = m_parent1.lock();
    auto p2 = m_parent2.lock();
    bool p1Empty = (!p1) || (p1->getCard() == nullptr);
    bool p2Empty = (!p2) || (p2->getCard() == nullptr);
    return p1Empty && p2Empty;
}

}
