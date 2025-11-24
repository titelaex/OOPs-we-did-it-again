module;
#include <utility>
module Core.Node;

import Core.Node;
import Models.Card;
import <memory>;

namespace Core {

Node::Node(std::unique_ptr<Models::Card> card, std::unique_ptr<Node> child1, std::unique_ptr<Node> child2)
    : m_card(std::move(card)), m_child1(std::move(child1)), m_child2(std::move(child2)) {}

Node::Node(Node&&) noexcept = default;
Node& Node::operator=(Node&&) noexcept = default;

Node::~Node() = default;

}
