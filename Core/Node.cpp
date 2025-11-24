module Core.Node;

#include <utility>

import Core.Node;
import Models.Card;
import <memory>;

namespace Core {

Node::Node(std::unique_ptr<Models::Card> card, Node* child1, Node* child2)
    : m_card(std::move(card)), m_child1(child1), m_child2(child2)
{
    if (m_child1) { if (m_child1->getParent1() == nullptr) m_child1->setParent1(this); else m_child1->setParent2(this); }
    if (m_child2) { if (m_child2->getParent1() == nullptr) m_child2->setParent1(this); else m_child2->setParent2(this); }
}

}
