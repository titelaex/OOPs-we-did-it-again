module Core.ConsolePrinter;
import <iostream>;

namespace Core {
    
    void ConsolePrinter::onCardBuilt(const CardEvent& event) {
        std::cout << "[CARD] " << event.playerName << " built " << event.cardName << "\n";
        if (!event.effectsApplied.empty()) {
            for (const auto& effect : event.effectsApplied) {
                std::cout << "       -> " << effect << "\n";
            }
        }
    }
    
    void ConsolePrinter::onCardSold(const CardEvent& event) {
        std::cout << "[CARD] " << event.playerName << " sold " << event.cardName << "\n";
    }
    
    void ConsolePrinter::onCardDiscarded(const CardEvent& event) {
        std::cout << "[CARD] " << event.cardName << " discarded\n";
    }
    
    void ConsolePrinter::onWonderBuilt(const WonderEvent& event) {
        std::cout << "[WONDER] " << event.playerName << " built " << event.wonderName 
                  << " stage " << event.stageIndex << "\n";
    }
    
    void ConsolePrinter::onTreeNodeChanged(const TreeNodeEvent& event) {
        if (!event.isEmpty) {
            std::cout << "[TREE] Node " << event.nodeIndex << " changed - "
                      << (event.isAvailable ? "AVAILABLE" : "UNAVAILABLE") << "\n";
        }
    }
    
    void ConsolePrinter::onTreeNodeEmptied(const TreeNodeEvent& event) {
        std::cout << "[TREE] Node " << event.nodeIndex << " is now empty\n";
    }
    
    void ConsolePrinter::onResourceGained(const ResourceEvent& event) {
        std::cout << "[RESOURCE] " << event.playerName << " +" << event.amount 
                  << " " << event.resourceType << " (total: " << event.newTotal << ")\n";
    }
    
    void ConsolePrinter::onResourceLost(const ResourceEvent& event) {
        std::cout << "[RESOURCE] " << event.playerName << " -" << event.amount 
                  << " " << event.resourceType << " (total: " << event.newTotal << ")\n";
    }
    
    void ConsolePrinter::onCoinsChanged(const CoinEvent& event) {
        std::cout << "[COINS] " << event.playerName << " coins: " << event.newTotal << "\n";
    }
    
    void ConsolePrinter::onTokenAcquired(const TokenEvent& event) {
        std::cout << "[TOKEN] " << event.playerName << " acquired: " << event.tokenName << "\n";
    }
    
    void ConsolePrinter::onPawnMoved(const PawnEvent& event) {
        std::cout << "[PAWN] Moved " << event.steps << " steps: " << event.previousPosition 
                  << " -> " << event.newPosition << " (" << event.reason << ")\n";
    }
    
    void ConsolePrinter::onTurnStarted(const TurnEvent& event) {
        std::cout << "\n>>> TURN: " << event.playerName << " (Phase " << event.phase 
                  << ", Round " << event.round << ")\n";
    }
    
    void ConsolePrinter::onTurnEnded(const TurnEvent& event) {
        std::cout << "<<< END TURN: " << event.playerName << "\n\n";
    }
    
    void ConsolePrinter::onPhaseChanged(const PhaseEvent& event) {
        std::cout << "\n========== PHASE " << event.newPhase << " BEGINS ==========\n";
        std::cout << "(" << event.phaseName << ")\n";
        std::cout << "==============================================\n\n";
    }
    
    void ConsolePrinter::onRoundChanged(int round, int phase) {
        std::cout << "[ROUND] Round " << round << " starts (Phase " << phase << ")\n";
    }
    
    void ConsolePrinter::onVictoryAchieved(const VictoryEvent& event) {
        std::cout << "\n!!!!!! VICTORY !!!!!!\n";
        std::cout << "Winner: " << event.winnerName << "\n";
        std::cout << "Type: " << event.victoryType << "\n";
        std::cout << "Score: " << event.winnerScore << " vs " << event.loserScore << "\n";
        std::cout << "!!!!!!!!!!!!!!!!!!!!!\n\n";
    }
    
    void ConsolePrinter::onGameStarted(int gameMode, Playstyle p1Style, Playstyle p2Style) {
        std::cout << "\n========== GAME STARTED ==========\n";
        std::cout << "==================================\n\n";
    }
    
    void ConsolePrinter::onGameEnded() {
        std::cout << "\n========== GAME ENDED ==========\n";
        std::cout << "================================\n\n";
    }
    
    void ConsolePrinter::onPointsChanged(const PointsEvent& event) {
        std::cout << "[POINTS] " << event.playerName << " = " << event.totalPoints 
                  << " (M:" << event.militaryPoints << " B:" << event.buildingPoints 
                  << " W:" << event.wonderPoints << " P:" << event.progressPoints << ")\n";
    }
    
    void ConsolePrinter::onPlayerStateChanged(int playerID) {
        std::cout << "[STATE] Player " << playerID << " updated\n";
    }
}
