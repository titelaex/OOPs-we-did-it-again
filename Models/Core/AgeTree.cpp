module Core.AgeTree;

import <vector>;
import <memory>;
import <algorithm>;
import <cmath>;

import Core.Node;
import Models.Card;

namespace Core {

// Helper: create nodes linearized row by row and return vector of Node* rows for wiring
static std::vector<std::vector<Node*>> makeRows(std::vector<std::unique_ptr<Node>>& ownedNodes, const std::vector<size_t>& rows) {
    std::vector<std::vector<Node*>> ptrs;
    size_t idx = 0;
    for (size_t r = 0; r < rows.size(); ++r) {
        size_t count = rows[r];
        std::vector<Node*> row;
        for (size_t c = 0; c < count; ++c) {
            row.push_back(ownedNodes[idx++].get());
        }
        ptrs.push_back(row);
    }
    return ptrs;
}

// Wire parents to children with simple triangular mapping: parent p -> children[p] and children[p+1]
static void wireTriangular(const std::vector<std::vector<Node*>>& rows) {
    for (size_t r = 0; r + 1 < rows.size(); ++r) {
        auto &parents = rows[r];
        auto &children = rows[r+1];
        for (size_t p = 0; p < parents.size(); ++p) {
            Node* parent = parents[p];
            Node* left = nullptr;
            Node* right = nullptr;
            if (p < children.size()) left = children[p];
            if (p + 1 < children.size()) right = children[p+1];
            if (left) parent->setChild1(left);
            if (right && right != left) parent->setChild2(right);
        }
    }
}

// Wire triangular mapping for a specific range of rows [start, endExclusive)
static void wireTriangularRange(const std::vector<std::vector<Node*>>& rows, size_t start, size_t endExclusive) {
    if (start >= endExclusive) return;
    for (size_t r = start; r + 1 < endExclusive && r + 1 < rows.size(); ++r) {
        auto &parents = rows[r];
        auto &children = rows[r+1];
        for (size_t p = 0; p < parents.size(); ++p) {
            Node* parent = parents[p];
            Node* left = nullptr;
            Node* right = nullptr;
            if (p < children.size()) left = children[p];
            if (p + 1 < children.size()) right = children[p + 1];
            if (left) parent->setChild1(left);
            if (right && right != left) parent->setChild2(right);
        }
    }
}

// Wire parents to children for reversed-pyramid shapes where parent row is larger than child row.
static void wireReversed(const std::vector<std::vector<Node*>>& rows) {
    for (size_t r = 0; r + 1 < rows.size(); ++r) {
        auto &parents = rows[r];
        auto &children = rows[r+1];
        size_t P = parents.size();
        size_t C = children.size();
        for (size_t p = 0; p < P; ++p) {
            Node* parent = parents[p];
            if (!parent) continue;
            Node* left = nullptr;
            Node* right = nullptr;
            // Desired mapping: child j should have parents j and j+1 when P == C+1.
            // Therefore parent p should attach to children p-1 and p (if exist).
            if (p > 0 && (p - 1) < C) left = children[p - 1];
            if (p < C) right = children[p];
            if (left) parent->setChild1(left);
            if (right && right != left) parent->setChild2(right);
        }
    }
}

// Wire reversed mapping for a specific range of rows [start, endExclusive)
static void wireReversedRange(const std::vector<std::vector<Node*>>& rows, size_t start, size_t endExclusive) {
    if (start >= endExclusive) return;
    for (size_t r = start; r + 1 < endExclusive && r + 1 < rows.size(); ++r) {
        auto &parents = rows[r];
        auto &children = rows[r+1];
        size_t P = parents.size();
        size_t C = children.size();
        for (size_t p = 0; p < P; ++p) {
            Node* parent = parents[p];
            if (!parent) continue;
            Node* left = nullptr;
            Node* right = nullptr;
            if (p > 0 && (p - 1) < C) left = children[p - 1];
            if (p < C) right = children[p];
            if (left) parent->setChild1(left);
            if (right && right != left) parent->setChild2(right);
        }
    }
}

// If a node ended up with only one parent, set its second parent to the same pointer
// so both parent1 and parent2 are non-null and equal. This matches requested behavior
// for Age1/Age3 where single-parent children should have both parent slots filled.
static void fixSingleParentDuplicates(const std::vector<std::vector<Node*>>& rows) {
    for (size_t r = 0; r < rows.size(); ++r) {
        for (auto node : rows[r]) {
            if (!node) continue;
            Node* p1 = node->getParent1();
            Node* p2 = node->getParent2();
            if (p1 && !p2) node->setParent2(p1);
            else if (!p1 && p2) node->setParent1(p2);
        }
    }
}

// Age 1: pyramid rows {2,3,4,5,6}
Age1Tree::Age1Tree(std::vector<std::unique_ptr<Models::Card>>&& cards) {
    const std::vector<size_t> rows = {2,3,4,5,6};
    size_t total = 0; for (auto v : rows) total += v;
    m_nodes.reserve(total);
    for (size_t i = 0; i < total && i < cards.size(); ++i) {
        m_nodes.push_back(std::make_unique<Node>(std::move(cards[i]), nullptr, nullptr));
    }
    auto rowPtrs = makeRows(m_nodes, rows);
    wireTriangular(rowPtrs);
    fixSingleParentDuplicates(rowPtrs);
}

std::vector<std::unique_ptr<Node>> Age1Tree::releaseNodes() {
    return std::move(m_nodes);
}

// Age 2: reversed pyramid rows {6,5,4,3,2}
Age2Tree::Age2Tree(std::vector<std::unique_ptr<Models::Card>>&& cards) {
    const std::vector<size_t> rows = {6,5,4,3,2};
    size_t total = 0; for (auto v : rows) total += v;
    m_nodes.reserve(total);
    for (size_t i = 0; i < total && i < cards.size(); ++i) {
        m_nodes.push_back(std::make_unique<Node>(std::move(cards[i]), nullptr, nullptr));
    }
    auto rowPtrs = makeRows(m_nodes, rows);
    // wire reversed: parents are larger row to smaller row. Use triangular mapping as well.
    wireReversed(rowPtrs);
    fixSingleParentDuplicates(rowPtrs);
}

std::vector<std::unique_ptr<Node>> Age2Tree::releaseNodes() {
    return std::move(m_nodes);
}

// Age 3: custom rows {2,3,4,2,4,3,2}
Age3Tree::Age3Tree(std::vector<std::unique_ptr<Models::Card>>&& cards) {
    const std::vector<size_t> rows = {2,3,4,2,4,3,2};
    size_t total = 0; for (auto v : rows) total += v;
    m_nodes.reserve(total);
    for (size_t i = 0; i < total && i < cards.size(); ++i) {
        m_nodes.push_back(std::make_unique<Node>(std::move(cards[i]), nullptr, nullptr));
    }
    auto rowPtrs = makeRows(m_nodes, rows);
    // wire rows 0->1 and 1->2 triangularly
    wireTriangularRange(rowPtrs, 0, 3);

    // handle the special 4->2->4 pattern at rows 2,3,4
    if (rowPtrs.size() >= 5 && rowPtrs[2].size() == 4 && rowPtrs[3].size() == 2 && rowPtrs[4].size() >= 4) {
        auto &A = rowPtrs[2];
        auto &B = rowPtrs[3];
        auto &C = rowPtrs[4];

        // A(4) -> B(2): B0 parents A0,A1 ; B1 parents A2,A3
        if (A.size() == 4 && B.size() == 2) {
            // assign children of A to B (A->B connections)
            A[0]->setChild1(B[0]);
            A[1]->setChild1(B[0]);
            A[2]->setChild1(B[1]);
            A[3]->setChild1(B[1]);
        }

        // B(2) -> C(4): B0 -> C0,C1 ; B1 -> C2,C3
        if (C.size() >= 4) {
            B[0]->setChild1(C[0]); B[0]->setChild2(C[1]);
            B[1]->setChild1(C[2]); B[1]->setChild2(C[3]);
        }

        // wire subsequent rows reversed/pyramid as appropriate
        wireReversedRange(rowPtrs, 4, rowPtrs.size());
    } else {
        // fallback wiring for the rest
        wireTriangularRange(rowPtrs, 2, rowPtrs.size());
    }

    // Ensure single-parent children have both parent slots filled for Age1/Age3 behaviour
    fixSingleParentDuplicates(rowPtrs);
}

std::vector<std::unique_ptr<Node>> Age3Tree::releaseNodes() {
    return std::move(m_nodes);
}

} // namespace Core
