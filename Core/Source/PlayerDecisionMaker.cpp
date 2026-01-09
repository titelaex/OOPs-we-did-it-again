module Core.PlayerDecisionMaker;

import <vector>;
import <cstdint>;
import <iostream>;
import <string>;

namespace Core {
    size_t HumanDecisionMaker::selectCard(const std::vector<size_t>& available) {
        if (available.empty()) return 0;
        size_t choice = 0;
        if (!(std::cin >> choice) || choice >= available.size()) {
            if (!std::cin) { std::cin.clear(); std::string discard; std::getline(std::cin, discard); }
            choice = 0;
        }
        return choice;
    }

    int HumanDecisionMaker::selectCardAction() {
        int action = 0;
        if (!(std::cin >> action) || action < 0 || action > 2) {
            if (!std::cin) { std::cin.clear(); std::string discard; std::getline(std::cin, discard); }
            action = 0;
        }
        return action;
    }

    size_t HumanDecisionMaker::selectWonder(const std::vector<size_t>& candidates) {
        if (candidates.empty()) return 0;
        size_t choice = 0;
        if (!(std::cin >> choice) || choice >= candidates.size()) {
            if (!std::cin) { std::cin.clear(); std::string discard; std::getline(std::cin, discard); }
            choice = 0;
        }
        return choice;
    }

    std::uint8_t HumanDecisionMaker::selectStartingPlayer() {
        unsigned int v = 0;
        if (!(std::cin >> v) || (v != 0 && v != 1)) {
            if (!std::cin) { std::cin.clear(); std::string discard; std::getline(std::cin, discard); }
            v = 0;
        }
        return static_cast<std::uint8_t>(v);
    }
}
