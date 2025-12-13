module Core.Node;

import Models.Card;
import <utility>;
import <memory>;

import Core.Node;

namespace Core {

Node::Node(std::unique_ptr<Models::Card> card, Node* child1, Node* child2)
    : m_card(std::move(card)), m_child1(child1), m_child2(child2)
{
    if (m_child1) { if (m_child1->getParent1() == nullptr) m_child1->setParent1(this); else m_child1->setParent2(this); }
    if (m_child2) { if (m_child2->getParent1() == nullptr) m_child2->setParent1(this); else m_child2->setParent2(this); }
}

std::unique_ptr<Models::Card> Node::releaseCard()
{
    return std::move(m_card);
}

bool Node::isAvailable() const
{
    if (!m_card) return false;
    auto p1 = getParent1();
    auto p2 = getParent2();
    bool p1Empty = (!p1) || (p1->getCard()==nullptr);
    bool p2Empty = (!p2) || (p2->getCard()==nullptr);
    return p1Empty && p2Empty;
}

}
