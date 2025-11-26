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
#include <filesystem>
#include <memory>

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
/// - Apply visibility (isVisible) and accessibility (isAccessible) rules to Nodes' cards.
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
    std::vector<std::string> columns;
    std::stringstream ss(line);
    std::string cell;
    bool in_quotes = false;
    char c;

    while (ss.get(c)) {
        if (c == '"') {
            in_quotes = !in_quotes;
        }
        else if (c == ',' && !in_quotes) {
            columns.push_back(cell);
            cell.clear();
        }
        else {
            cell += c;
        }
    }
    columns.push_back(cell); // Add the last cell
    return columns;
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

    // Wire parents -> children. Handle cases where child row may have more, fewer or equal nodes
    // compared to the parent row. Use linear interpolation from parent index to child index
    // to compute left/right child indices robustly and avoid out-of-range access.
    for (size_t r = 0; r + 1 < rowLeftToRightPtrs.size(); ++r) {
        auto &parents = rowLeftToRightPtrs[r];
        auto &children = rowLeftToRightPtrs[r+1];
        size_t P = parents.size();
        size_t C = children.size();
        if (P == 0 || C == 0) continue;
        for (size_t p = 0; p < P; ++p) {
            Node* parent = parents[p];
            Node* leftChild = nullptr;
            Node* rightChild = nullptr;

            if (P == 1) {
                // Single parent maps to center child (or first)
                size_t idx = (C > 0) ? (C - 1) / 2 : 0;
                leftChild = rightChild = children[idx];
            } else {
                double t = 0.0;
                if (P > 1) t = static_cast<double>(p) * static_cast<double>(C - 1) / static_cast<double>(P - 1);
                size_t li = static_cast<size_t>(std::floor(t));
                size_t ri = static_cast<size_t>(std::ceil(t));
                if (li >= C) li = C - 1;
                if (ri >= C) ri = C - 1;
                leftChild = children[li];
                rightChild = children[ri];
            }

            if (parent) {
                parent->setChild1(leftChild);
                parent->setChild2(rightChild);
            }
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

    // Log current working directory to help diagnose relative path issues
    try {
        auto cwd = std::filesystem::current_path();
        if (log.is_open()) log << "Current working directory: " << cwd.string() << "\n";
        std::cout << "Current working directory: " << cwd.string() << "\n";
    } catch (...) {
        if (log.is_open()) log << "[Warning] Unable to determine current working directory\n";
    }

    // Helper: try several candidate relative paths and return the first that exists
    auto findExistingPath = [&](const std::vector<std::string>& candidates) -> std::string {
        for (const auto& p : candidates) {
            try { if (std::filesystem::exists(p)) return p; } catch (...) {}
        }
        return std::string{};
    };

    // Load Age cards: find CSV, parse into temporary vector, then split into per-age pools and shuffle
    try {
        std::vector<Models::AgeCard> allAgeCards;
        auto loadFn = [&](const std::vector<std::string>& cols){
            Models::AgeCard card = AgeCardFactory(cols);
            allAgeCards.push_back(std::move(card));
        };

        const std::vector<std::string> ageCandidates = {
            "AgeCards.csv",
            "Resources/AgeCards.csv",
            "Core/Resources/AgeCards.csv",
            "../Core/Resources/AgeCards.csv",
            "../ModelsDLL/AgeCards.csv",
            "ModelsDLL/AgeCards.csv",
            "../Models/AgeCards.csv"
        };
        std::string agePath = findExistingPath(ageCandidates);
        if (agePath.empty()) {
            if (log.is_open()) log << "[Error] Age file not found in candidates\n";
            std::cout << "[Error] Age file not found in candidates\n";
        } else {
            if (log.is_open()) log << "Using Age file: " << agePath << "\n";
            std::cout << "Using Age file: " << agePath << "\n";
            loadGenericFile(agePath, loadFn, log);
        }

        // Split into per-age pools first, preserving the card objects, then shuffle each pool independently.
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
        // Shuffle each age pool independently to ensure per-age randomization
        ShuffleInplace(Core::unusedAgeOneCards, seed);
        ShuffleInplace(Core::unusedAgeTwoCards, seed + 1);
        ShuffleInplace(Core::unusedAgeThreeCards, seed + 2);
        if (log.is_open()) log << "Loaded ages: I=" << Core::unusedAgeOneCards.size()
            << " II=" << Core::unusedAgeTwoCards.size() << " III=" << Core::unusedAgeThreeCards.size() << "\n";
        std::cout << "Loaded ages: I=" << Core::unusedAgeOneCards.size()
            << " II=" << Core::unusedAgeTwoCards.size() << " III=" << Core::unusedAgeThreeCards.size() << "\n";
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
        const std::vector<std::string> guildCandidates = {
            "Guilds.csv",
            "Resources/Guilds.csv",
            "Core/Resources/Guilds.csv",
            "../Core/Resources/Guilds.csv",
            "../ModelsDLL/Guilds.csv",
            "ModelsDLL/Guilds.csv",
            "../Models/Guilds.csv"
        };
        std::string guildPath = findExistingPath(guildCandidates);
        if (guildPath.empty()) {
            if (log.is_open()) log << "[Error] Guild file not found in candidates\n";
            std::cout << "[Error] Guild file not found in candidates\n";
        } else {
            if (log.is_open()) log << "Using Guild file: " << guildPath << "\n";
            std::cout << "Using Guild file: " << guildPath << "\n";
            loadGenericFile(guildPath, loadG, log);
        }
        ShuffleInplace(tempGuilds, seed);
        for (auto& g : tempGuilds) Core::unusedGuildCards.push_back(std::make_unique<Models::GuildCard>(std::move(g)));
        if (log.is_open()) log << "Loaded guilds: " << Core::unusedGuildCards.size() << "\n";
        std::cout << "Loaded guilds: " << Core::unusedGuildCards.size() << "\n";
    } catch (const std::exception& ex) {
        if (log.is_open()) log << "[Exception] While processing guild cards: " << ex.what() << "\n";
    }

    try {
        std::vector<Models::Wonder> tempWonders;
        auto loadW = [&](const std::vector<std::string>& cols){
            Models::Wonder w = WonderFactory(cols);
            tempWonders.push_back(std::move(w));
        };
        const std::vector<std::string> wonderCandidates = {
            "Wonders.csv",
            "Resources/Wonders.csv",
            "Core/Resources/Wonders.csv",
            "../Core/Resources/Wonders.csv",
            "../ModelsDLL/Wonders.csv",
            "ModelsDLL/Wonders.csv",
            "../Models/Wonders.csv"
        };
        std::string wonderPath = findExistingPath(wonderCandidates);
        if (wonderPath.empty()) {
            if (log.is_open()) log << "[Error] Wonder file not found in candidates\n";
            std::cout << "[Error] Wonder file not found in candidates\n";
        } else {
            if (log.is_open()) log << "Using Wonder file: " << wonderPath << "\n";
            std::cout << "Using Wonder file: " << wonderPath << "\n";
            loadGenericFile(wonderPath, loadW, log);
        }
        ShuffleInplace(tempWonders, seed);
        for (auto& w : tempWonders) Core::unusedWonders.push_back(std::make_unique<Models::Wonder>(std::move(w)));
        if (log.is_open()) log << "Loaded wonders: " << Core::unusedWonders.size() << "\n";
        std::cout << "Loaded wonders: " << Core::unusedWonders.size() << "\n";
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
