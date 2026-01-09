export module Core.PlayerDecisionMaker;

import <vector>;
import <cstdint>;

export namespace Core {

    struct IPlayerDecisionMaker {
        virtual ~IPlayerDecisionMaker() = default;
        virtual size_t selectCard(const std::vector<size_t>& available) = 0;
        virtual int selectCardAction() = 0;
        virtual size_t selectWonder(const std::vector<size_t>& candidates) = 0;
        virtual std::uint8_t selectStartingPlayer() = 0; // 0 = Player1, 1 = Player2
    };

    struct HumanDecisionMaker : IPlayerDecisionMaker {
        size_t selectCard(const std::vector<size_t>& available) override;
        int selectCardAction() override;
        size_t selectWonder(const std::vector<size_t>& candidates) override;
        std::uint8_t selectStartingPlayer() override;
    };
}
