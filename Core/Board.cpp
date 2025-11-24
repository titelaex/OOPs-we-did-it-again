module Core.Board;

import Core.Node;
import Models.AgeCard;
import Models.GuildCard;
import Models.Wonder;

#include <random>
#include <algorithm>

namespace Core {
    std::vector<std::unique_ptr<Models::Card>> unusedAgeOneCards;
    std::vector<std::unique_ptr<Models::Card>> unusedAgeTwoCards;
    std::vector<std::unique_ptr<Models::Card>> unusedAgeThreeCards; // excluding guilds
    std::vector<std::unique_ptr<Models::Card>> unusedGuildCards;
    std::vector<std::unique_ptr<Models::Card>> unusedWonders;

    std::vector<std::unique_ptr<Node>> age1Nodes;
    std::vector<std::unique_ptr<Node>> age2Nodes;
    std::vector<std::unique_ptr<Node>> age3Nodes;

    void SetupCardPools()
    {
        unusedAgeOneCards.reserve(23);
        unusedAgeTwoCards.reserve(23);
        unusedAgeThreeCards.reserve(20);
        unusedGuildCards.reserve(7);
        unusedWonders.reserve(12);
    }

    template<typename T>
    static std::vector<T> shuffle_and_move_impl(std::vector<T>&& src, uint32_t seed)
    {
        auto rng = std::mt19937_64(seed);
        std::vector<T> dest = std::move(src);
        std::shuffle(dest.begin(), dest.end(), rng);
        return dest;
    }

    static std::vector<std::unique_ptr<Node>> build_tree_from_cards_internal(std::vector<std::unique_ptr<Models::Card>>& cards, const std::vector<size_t>& rows)
    {
        std::vector<std::unique_ptr<Node>> ownedNodes;
        std::vector<std::vector<Node*>> rowPtrs;

        size_t total = 0;
        for (auto r : rows) total += r;
        if (cards.size() < total) return ownedNodes;

        size_t idx = 0;
        for (size_t r = 0; r < rows.size(); ++r) {
            size_t count = rows[r];
            rowPtrs.emplace_back();
            for (size_t c = 0; c < count; ++c) {
                ownedNodes.push_back(std::make_unique<Node>(std::move(cards[idx++]), nullptr, nullptr));
                rowPtrs[r].push_back(ownedNodes.back().get());
            }
        }

        for (size_t r = 0; r + 1 < rowPtrs.size(); ++r) {
            auto& parents = rowPtrs[r];
            auto& children = rowPtrs[r+1];
            for (size_t p = 0; p < parents.size(); ++p) {
                Node* parent = parents[p];
                Node* leftChild = children[p];
                Node* rightChild = children[p+1];
                parent->setChild1(leftChild);
                parent->setChild2(rightChild);
            }
        }

        size_t lastRow = rowPtrs.size() - 1;
        for (size_t r = 0; r < rowPtrs.size(); ++r) {
            for (size_t c = 0; c < rowPtrs[r].size(); ++c) {
                Node* node = rowPtrs[r][c];
                Models::Card* card = node->getCard();
                if (card) {
                    bool isBottom = (r == lastRow);
                    bool visible = isBottom || (((int)r + (int)c) % 2 == 0);
                    card->SetIsVisible(visible);
                    card->SetIsAccessible(isBottom);
                }
            }
        }

        return ownedNodes;
    }

    void BuildAge1Tree(uint32_t seed)
    {
        std::vector<size_t> rows = {5,4,3,2,1};
        std::vector<std::unique_ptr<Models::Card>> working;
        size_t take = std::min<size_t>(20, unusedAgeOneCards.size());
        for (size_t i = 0; i < take; ++i) working.push_back(std::move(unusedAgeOneCards[i]));
        unusedAgeOneCards.erase(unusedAgeOneCards.begin(), unusedAgeOneCards.begin()+take);
        working = shuffle_and_move_impl(std::move(working), seed);
        age1Nodes = build_tree_from_cards_internal(working, rows);
    }

    void BuildAge2Tree(uint32_t seed)
    {
        std::vector<size_t> rows = {5,4,3,2,1};
        std::vector<std::unique_ptr<Models::Card>> working;
        size_t take = std::min<size_t>(20, unusedAgeTwoCards.size());
        for (size_t i = 0; i < take; ++i) working.push_back(std::move(unusedAgeTwoCards[i]));
        unusedAgeTwoCards.erase(unusedAgeTwoCards.begin(), unusedAgeTwoCards.begin()+take);
        working = shuffle_and_move_impl(std::move(working), seed+1);
        age2Nodes = build_tree_from_cards_internal(working, rows);
    }

    void BuildAge3Tree(uint32_t seed)
    {
        std::vector<size_t> rows = {5,4,4,3,2};
        std::vector<std::unique_ptr<Models::Card>> working;
        size_t take = std::min<size_t>(17, unusedAgeThreeCards.size());
        for (size_t i = 0; i < take; ++i) working.push_back(std::move(unusedAgeThreeCards[i]));
        unusedAgeThreeCards.erase(unusedAgeThreeCards.begin(), unusedAgeThreeCards.begin()+take);
        size_t guildTake = std::min<size_t>(3, unusedGuildCards.size());
        for (size_t i = 0; i < guildTake; ++i) working.push_back(std::move(unusedGuildCards[i]));
        unusedGuildCards.erase(unusedGuildCards.begin(), unusedGuildCards.begin()+guildTake);
        working = shuffle_and_move_impl(std::move(working), seed+2);
        age3Nodes = build_tree_from_cards_internal(working, rows);
    }
}
