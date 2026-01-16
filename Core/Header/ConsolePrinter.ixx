export module Core.ConsolePrinter;
import Core.IGameListener;
import <memory>;

export namespace Core {
    
    export class ConsolePrinter : public IGameListener {
    public:
        ConsolePrinter() = default;
        ~ConsolePrinter() override = default;
        
        void onCardBuilt(const CardEvent& event) override;
        void onCardSold(const CardEvent& event) override;
        void onCardDiscarded(const CardEvent& event) override;
        void onWonderBuilt(const WonderEvent& event) override;
        
        void onTreeNodeChanged(const TreeNodeEvent& event) override;
        void onTreeNodeEmptied(const TreeNodeEvent& event) override;
        
        void onResourceGained(const ResourceEvent& event) override;
        void onResourceLost(const ResourceEvent& event) override;
        void onCoinsChanged(const CoinEvent& event) override;
        
        void onTokenAcquired(const TokenEvent& event) override;
        void onPawnMoved(const PawnEvent& event) override;
        
        void onTurnStarted(const TurnEvent& event) override;
        void onTurnEnded(const TurnEvent& event) override;
        void onPhaseChanged(const PhaseEvent& event) override;
        void onRoundChanged(int round, int phase) override;
        
        void onVictoryAchieved(const VictoryEvent& event) override;
        void onGameStarted(int gameMode, Playstyle p1Style, Playstyle p2Style) override;
        void onGameEnded() override;
        
        void onPointsChanged(const PointsEvent& event) override;
        void onPlayerStateChanged(int playerID) override;
    };
}
