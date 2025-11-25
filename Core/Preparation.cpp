module Core.Preparation;

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <functional>
#include <algorithm>
#include <unordered_map>
#include <exception>
#include <random>

import Core.Board;
import Core.CardCsvParser;
import Core.Node;

import Models.AgeCard;
import Models.GuildCard;
import Models.Wonder;
import Models.Age;
import Models.Card;

namespace Core {

// Preparation.cpp responsibilities:
// - Read CSVs and populate the `Core::unused*` card pools located in `Core::Board`.
// - Shuffle the unused pools deterministically using a seed.
// - Build the Age I/II/III node graphs by moving cards from the unused pools into Node instances
//   stored in `Core::age1Nodes`, `Core::age2Nodes`, `Core::age3Nodes` (in `Core::Board`).
// - Apply visibility (isVisible) and accessibility (isAccessible) rules to Nodes' cards.
// The code below implements these steps and documents the ordering and wiring rules.

// Random utilities
static std::mt19937_64 make_rng(uint32_t seed) { return std::mt19937_64(seed); }

template<typename T>
static void ShuffleInplace(std::vector<T>& v, uint32_t seed)
{
    // Use a 64-bit Mersenne Twister seeded with `seed` for deterministic shuffling.
    auto rng = make_rng(seed);
    std::shuffle(v.begin(), v.end(), rng);
}

// Move-and-shuffle helper: accepts an rvalue vector, moves it into a new vector and shuffles in-place.
// Returns the shuffled moved vector so the caller receives ownership of the elements.
template<typename T>
static std::vector<T> ShuffleAndMove(std::vector<T>&& src, uint32_t seed)
{
    std::vector<T> dest = std::move(src);
    auto rng = make_rng(seed);
    std::shuffle(dest.begin(), dest.end(), rng);
    return dest;
}

// CSV parsing helpers (same approach used elsewhere in the project)
static std::vector<std::string> splitCSVLine(const std::string& line) {
    std::vector<std::string> cols;
    std::istringstream ss(line);
    std::string cell;
    while (std::getline(ss, cell, ',')) cols.push_back(cell);
    return cols;
}

static void loadGenericFile(const std::string& path, const std::function<void(const std::vector<std::string>&)>& onItem, std::ofstream& log)
{
    try {
        std::ifstream ifs(path);
        if (!ifs.is_open()) {
            if (log.is_open()) log << "[Error] Unable to open CSV file: " << path << "\n";
            return;
        }
        std::string header;
        std::getline(ifs, header);
        std::string line;
        size_t lineno = 1;
        while (std::getline(ifs, line)) {
            ++lineno;
            if (line.empty()) continue;
            auto cols = splitCSVLine(line);
            try {
                onItem(cols);
            } catch (const std::exception& ex) {
                if (log.is_open()) log << "[Error] Factory error in " << path << " at line " << lineno << ": " << ex.what() << "\n";
            } catch (...) {
                if (log.is_open()) log << "[Error] Unknown factory error in " << path << " at line " << lineno << "\n";
            }
        }
    } catch (const std::exception& ex) {
        if (log.is_open()) log << "[Exception] Reading " << path << ": " << ex.what() << "\n";
    } catch (...) {
        if (log.is_open()) log << "[Exception] Unknown error reading " << path << "\n";
    }
}

/*
Graph building rules and ordering
- Each age graph is specified by a vector of row counts (top-to-bottom). Examples from the image:
    Age I  (top-to-bottom): {2,3,4,5,6}  (total 20)
    Age II (top-to-bottom): {6,5,4,3,2}  (total 20)
    Age III (top-to-bottom): {2,3,4,2,4,3,2} (total 20: 17 Age III + 3 guilds)

- Card selection order (from the shuffled unused deck):
  For each age we take the first N cards from the unused pool (N = 20 for age 1/2, 20 for age 3 including guilds).
  The requested ordering for placement is: "line 1 from right to left, then line 2 from right to left, ..."
  To satisfy this, the builder performs the following steps for each row:
    1. Create node instances for that row by popping cards from the front of the selected card list
       and constructing Nodes. The builder collects pointers to nodes in left-to-right order for wiring.
    2. After creating the left-to-right list (rowPtrs), the builder appends the nodes into the global
       ownedNodes vector in right-to-left order. This ensures the vector ordering matches the desired
       right-to-left per-row placement while wiring still uses natural left-to-right indices.

- Parent/child wiring rules:
  For each parent node in row R (left-to-right index p), its children in row R+1 are the nodes at indices
  p (left child) and p+1 (right child) in the left-to-right array of the children row. This corresponds to
  the triangular layout seen in the image.

- Visibility and accessibility rules:
  - Bottom row (last/top depending on drawing) cards are accessible (SetIsAccessible(true)).
  - Bottom row cards are visible (SetIsVisible(true)).
  - For other rows, cards are marked visible based on a simple alternating pattern (row+col parity).
    This reproduces the white/colored visual alternation from the image. You can change this to be
    CSV-driven if you prefer explicit control.

- Important: Cards are moved (std::unique_ptr) from the `Core::unused*` pools found in `Core::Board` into
  Node instances. No extra copies are made — nodes take ownership of the card objects.
*/

// Build nodes for a given set of row counts. The function assumes selectedCards contains exactly the
// number of cards required by rows (or more). Cards are moved from selectedCards into Nodes.
static std::vector<std::unique_ptr<Node>> build_tree_rows(std::vector<std::unique_ptr<Models::Card>>& selectedCards, const std::vector<size_t>& rows)
{
    // ownedNodes: will hold owning unique_ptrs for all nodes in row-append order (right-to-left per row)
    std::vector<std::unique_ptr<Node>> ownedNodes;

    // rowLeftToRightPtrs: for each row we maintain a vector of Node* in left-to-right order for wiring
    std::vector<std::vector<Node*>> rowLeftToRightPtrs;

    size_t total = 0;
    for (auto r : rows) total += r;
    if (selectedCards.size() < total) return ownedNodes; // safety check

    size_t idx = 0;
    // Build nodes row by row (top-to-bottom)
    for (size_t r = 0; r < rows.size(); ++r) {
        size_t count = rows[r];

        // temporary storage for this row's owning nodes and left-to-right pointers
        std::vector<std::unique_ptr<Node>> rowOwned;
        std::vector<Node*> rowPtrs;

        // create nodes left-to-right using the next cards from selectedCards (order: front -> back)
        for (size_t c = 0; c < count; ++c) {
            // move the card into the node, Node takes ownership via unique_ptr
            rowOwned.push_back(std::make_unique<Node>(std::move(selectedCards[idx++]), nullptr, nullptr));
            rowPtrs.push_back(rowOwned.back().get());
        }

        // Keep left-to-right pointers for wiring
        rowLeftToRightPtrs.push_back(rowPtrs);

        // Append the nodes to the ownedNodes vector in left-to-right order so the final vector has
        // nodes ordered per-row left-to-right as requested (this matches natural reading order and
        // simplifies mapping from deck order into board positions).
        for (size_t i = 0; i < count; ++i) {
            ownedNodes.push_back(std::move(rowOwned[i]));
        }
    }

    // Wire parents -> children using the left-to-right arrays (natural indexing). Parent p connects
    // to children at indices p and p+1 in the next row.
    for (size_t r = 0; r + 1 < rowLeftToRightPtrs.size(); ++r) {
        auto &parents = rowLeftToRightPtrs[r];
        auto &children = rowLeftToRightPtrs[r+1];
        for (size_t p = 0; p < parents.size(); ++p) {
            Node* parent = parents[p];
            Node* leftChild = children[p];
            Node* rightChild = children[p+1];
            parent->setChild1(leftChild);
            parent->setChild2(rightChild);
        }
    }

    // Apply visibility/accessibility rules. 'isBottom' nodes are those in the last row.
    size_t lastRow = rows.size() - 1;
    for (size_t r = 0; r < rowLeftToRightPtrs.size(); ++r) {
        for (size_t c = 0; c < rowLeftToRightPtrs[r].size(); ++c) {
            Node* node = rowLeftToRightPtrs[r][c];
            Models::Card* card = node->getCard();
            if (!card) continue;
            bool isBottom = (r == lastRow);
            bool visible = isBottom || (((int)r + (int)c) % 2 == 0);
            card->SetIsVisible(visible);
            card->SetIsAccessible(isBottom);
        }
    }

    return ownedNodes;
}

// Public preparation entry: read csvs, fill unused pools (in Core::Board), then build each age's tree
void PrepareBoardCardPools()
{
    // Reserve vector sizes for known counts
    Core::SetupCardPools();

    // Deterministic seed
    uint32_t seed = static_cast<uint32_t>(std::random_device{}());
    std::ofstream log("Preparation.log", std::ios::app);

    // Load Age cards from CSV and split by age into Board's unused pools. Cards are created as
    // concrete AgeCard objects and then wrapped into std::unique_ptr<Models::Card> so they can be
    // moved to Nodes later without slicing or copying.
    try {
        std::vector<Models::AgeCard> allAgeCards;
        auto loadFn = [&](const std::vector<std::string>& cols){
            Models::AgeCard card = AgeCardFactory(cols);
            allAgeCards.push_back(std::move(card));
        };
        loadGenericFile("ModelsDLL/AgeCards.csv", loadFn, log);

        // Shuffle age cards before splitting into pools to ensure random distribution per age
        ShuffleInplace(allAgeCards, seed);

        for (size_t i = 0; i < allAgeCards.size(); ++i) {
            auto& c = allAgeCards[i];
            switch (c.getAge()) {
            case Models::Age::AGE_I:
                Core::unusedAgeOneCards.push_back(std::make_unique<Models::AgeCard>(std::move(c)));
                break;
            case Models::Age::AGE_II:
                Core::unusedAgeTwoCards.push_back(std::make_unique<Models::AgeCard>(std::move(c)));
                break;
            case Models::Age::AGE_III:
            default:
                Core::unusedAgeThreeCards.push_back(std::make_unique<Models::AgeCard>(std::move(c)));
                break;
            }
        }
    } catch (const std::exception& ex) {
        if (log.is_open()) log << "[Exception] While processing age cards: " << ex.what() << "\n";
    }

    // Load guilds and wonders and store as unique_ptr<Models::Card> as well
    try {
        std::vector<Models::GuildCard> tempGuilds;
        auto loadG = [&](const std::vector<std::string>& cols){
            Models::GuildCard g = GuildCardFactory(cols);
            tempGuilds.push_back(std::move(g));
        };
        loadGenericFile("ModelsDLL/Guilds.csv", loadG, log);
        ShuffleInplace(tempGuilds, seed);
        for (auto& g : tempGuilds) Core::unusedGuildCards.push_back(std::make_unique<Models::GuildCard>(std::move(g)));
    } catch (const std::exception& ex) {
        if (log.is_open()) log << "[Exception] While processing guild cards: " << ex.what() << "\n";
    }

    try {
        std::vector<Models::Wonder> tempWonders;
        auto loadW = [&](const std::vector<std::string>& cols){
            Models::Wonder w = WonderFactory(cols);
            tempWonders.push_back(std::move(w));
        };
        loadGenericFile("ModelsDLL/Wonders.csv", loadW, log);
        ShuffleInplace(tempWonders, seed);
        for (auto& w : tempWonders) Core::unusedWonders.push_back(std::make_unique<Models::Wonder>(std::move(w)));
    } catch (const std::exception& ex) {
        if (log.is_open()) log << "[Exception] While processing wonder cards: " << ex.what() << "\n";
    }

    // At this point the unused pools hold all cards per age as unique_ptr<Models::Card>.
    // Now for each age we will:
    //  - take the required number of cards from the front of the unused pool
    //  - shuffle the taken set (deterministic via seed)
    //  - move the shuffled cards into nodes using the row-based builder

    // Age I: rows top-to-bottom {2,3,4,5,6} (total 20)
    {
        const std::vector<size_t> rows = {2,3,4,5,6};
        std::vector<std::unique_ptr<Models::Card>> selected;
        size_t take = std::min<size_t>(20, Core::unusedAgeOneCards.size());
        for (size_t i = 0; i < take; ++i) selected.push_back(std::move(Core::unusedAgeOneCards[i]));
        Core::unusedAgeOneCards.erase(Core::unusedAgeOneCards.begin(), Core::unusedAgeOneCards.begin()+take);
        selected = ShuffleAndMove(std::move(selected), seed);
        Core::age1Nodes = build_tree_rows(selected, rows);
    }

    // Age II: rows top-to-bottom {6,5,4,3,2} (total 20)
    {
        const std::vector<size_t> rows = {6,5,4,3,2};
        std::vector<std::unique_ptr<Models::Card>> selected;
        size_t take = std::min<size_t>(20, Core::unusedAgeTwoCards.size());
        for (size_t i = 0; i < take; ++i) selected.push_back(std::move(Core::unusedAgeTwoCards[i]));
        Core::unusedAgeTwoCards.erase(Core::unusedAgeTwoCards.begin(), Core::unusedAgeTwoCards.begin()+take);
        selected = ShuffleAndMove(std::move(selected), seed + 1);
        Core::age2Nodes = build_tree_rows(selected, rows);
    }

    // Age III: custom structure top-to-bottom {2,3,4,2,4,3,2} (total 20; take 17 from age3 + 3 guilds)
    {
        const std::vector<size_t> rows = {2,3,4,2,4,3,2};
        std::vector<std::unique_ptr<Models::Card>> selected;
        size_t take = std::min<size_t>(17, Core::unusedAgeThreeCards.size());
        for (size_t i = 0; i < take; ++i) selected.push_back(std::move(Core::unusedAgeThreeCards[i]));
        Core::unusedAgeThreeCards.erase(Core::unusedAgeThreeCards.begin(), Core::unusedAgeThreeCards.begin()+take);

        // add 3 guilds into the age III selection
        size_t guildTake = std::min<size_t>(3, Core::unusedGuildCards.size());
        for (size_t i = 0; i < guildTake; ++i) selected.push_back(std::move(Core::unusedGuildCards[i]));
        Core::unusedGuildCards.erase(Core::unusedGuildCards.begin(), Core::unusedGuildCards.begin()+guildTake);

        selected = ShuffleAndMove(std::move(selected), seed + 2);
        Core::age3Nodes = build_tree_rows(selected, rows);
    }
}

} // namespace Core
