export module Core.PlayerDecisionMaker;
import <vector>;
import <cstdint>;
import <memory>;
import <string>;
import Core.AIConfig;
namespace Core {
    class MCTS;
    struct MCTSGameState;
    struct MCTSAction;
}
export namespace Core {
    struct IPlayerDecisionMaker {
        virtual ~IPlayerDecisionMaker() = default;
        virtual size_t selectCard(const std::vector<size_t>& available) = 0;
        virtual int selectCardAction() = 0;
        virtual size_t selectWonder(const std::vector<size_t>& candidates) = 0;
        virtual std::uint8_t selectStartingPlayer() = 0; 
    };
    struct HumanDecisionMaker : IPlayerDecisionMaker {
        size_t selectCard(const std::vector<size_t>& available) override;
        int selectCardAction() override;
        size_t selectWonder(const std::vector<size_t>& candidates) override;
        std::uint8_t selectStartingPlayer() override;
    };
    struct MCTSDecisionMaker : IPlayerDecisionMaker {
        MCTSDecisionMaker(Playstyle playstyle = Playstyle::BRITNEY, 
                          int iterations = 1000, 
                          double explorationConstant = 1.414,
                          int maxDepth = 20);
        ~MCTSDecisionMaker();
        size_t selectCard(const std::vector<size_t>& available) override;
        int selectCardAction() override;
        size_t selectWonder(const std::vector<size_t>& candidates) override;
        std::uint8_t selectStartingPlayer() override;
        void setPlaystyle(Playstyle style);
        void setIterations(int count);
        void setExplorationConstant(double constant);
        void setMaxDepth(int depth);
        Playstyle getPlaystyle() const;
    private:
        std::unique_ptr<MCTS> m_mcts;
        Playstyle m_playstyle;
        int m_iterations;
        double m_explorationConstant;
        int m_maxDepth;
    };
    struct HumanAssistedDecisionMaker : IPlayerDecisionMaker {
        HumanAssistedDecisionMaker(Playstyle suggestionStyle = Playstyle::BRITNEY,
                                    int iterations = 500);
        ~HumanAssistedDecisionMaker();
        size_t selectCard(const std::vector<size_t>& available) override;
        int selectCardAction() override;
        size_t selectWonder(const std::vector<size_t>& candidates) override;
        std::uint8_t selectStartingPlayer() override;
        void setSuggestionStyle(Playstyle style);
        Playstyle getSuggestionStyle() const;
    private:
        std::unique_ptr<MCTS> m_mcts;
        Playstyle m_suggestionStyle;
        int m_iterations;
        void showSuggestions(const std::vector<size_t>& options, const std::string& context);
    };
    struct AIDecisionMaker : IPlayerDecisionMaker {
        AIDecisionMaker();
        ~AIDecisionMaker();
        size_t selectCard(const std::vector<size_t>& available) override;
        int selectCardAction() override;
        size_t selectWonder(const std::vector<size_t>& candidates) override;
        std::uint8_t selectStartingPlayer() override;
        void setSimulationCount(unsigned int count);
        void setExplorationConstant(double constant);
        void setMaxDepth(unsigned int depth);
    private:
        std::unique_ptr<MCTS> m_mcts;
        unsigned int m_simulationCount = 1000;
        double m_explorationConstant = 1.414;
        unsigned int m_maxDepth = 50;
    };
}
