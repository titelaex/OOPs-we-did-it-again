export module Core.Board;
import <bitset>;
import <vector>;
import <memory>;
import <deque>;
import Core.Node;
import Models.Card;
import Models.AgeCard;
import Models.GuildCard;
import Models.Wonder;
import Models.Token;
import Models.Card;
import Models.Bank;
export namespace Core {
    export class Board{
        private:
        Models::Bank bank{};
        std::vector<std::unique_ptr<Models::Card>> unusedAgeOneCards;
        std::vector<std::unique_ptr<Models::Card>> unusedAgeTwoCards;
        std::vector<std::unique_ptr<Models::Card>> unusedAgeThreeCards;
        std::vector<std::unique_ptr<Models::Card>> unusedGuildCards;
        std::vector<std::unique_ptr<Models::Card>> unusedWonders;
		std::vector<std::unique_ptr<Models::Card>> discardedCards;
        std::bitset<19> pawnTrack;
        uint8_t pawnPos;
        std::vector<std::unique_ptr<Models::Token>> progressTokens;
        std::vector<std::unique_ptr<Models::Token>> militaryTokens;
        std::vector<std::unique_ptr<Models::Token>> unusedProgressTokens;
        std::vector<std::shared_ptr<Node>> age1Nodes;
        std::vector<std::shared_ptr<Node>> age2Nodes;
        std::vector<std::shared_ptr<Node>> age3Nodes;
    public:
        Models::Bank& getBank();
        void setupCardPools();
        void displayBoard();
	    void displayEntireBoard();
        const std::vector<std::unique_ptr<Models::Card>>& getUnusedAgeOneCards() const;
        void setUnusedAgeOneCards(std::vector<std::unique_ptr<Models::Card>> v);
		const std::vector<std::unique_ptr<Models::Card>>& getUnusedAgeTwoCards() const;
		void setUnusedAgeTwoCards(std::vector<std::unique_ptr<Models::Card>> v);
		const std::vector<std::unique_ptr<Models::Card>>& getUnusedAgeThreeCards() const;
		void setUnusedAgeThreeCards(std::vector<std::unique_ptr<Models::Card>> v);
		const std::vector<std::unique_ptr<Models::Card>>& getUnusedGuildCards() const;
		void setUnusedGuildCards(std::vector<std::unique_ptr<Models::Card>> v);
		const std::vector<std::unique_ptr<Models::Card>>& getUnusedWonders() const;
		void setUnusedWonders(std::vector<std::unique_ptr<Models::Card>> v);
		const std::vector<std::unique_ptr<Models::Card>>& getDiscardedCards() const;
		void setDiscardedCards(std::vector<std::unique_ptr<Models::Card>> v);
		const std::bitset<19>& getPawnTrack() const;
		void setPawnTrack(const std::bitset<19>& track);
		uint8_t getPawnPos() const;
		void setPawnPos(uint8_t pos);
		const std::vector<std::unique_ptr<Models::Token>>& getProgressTokens() const;
		void setProgressTokens(std::vector<std::unique_ptr<Models::Token>> v);
		const std::vector<std::unique_ptr<Models::Token>>& getUnusedProgressTokens() const;
		void setUnusedProgressTokens(std::vector<std::unique_ptr<Models::Token>> v);
		const std::vector<std::unique_ptr<Models::Token>>& getMilitaryTokens() const;
		void setMilitaryTokens(std::vector<std::unique_ptr<Models::Token>> v);
		const std::vector<std::shared_ptr<Node>>& getAge1Nodes() const;
		void setAge1Nodes(std::vector<std::shared_ptr<Node>> v);
		const std::vector<std::shared_ptr<Node>>& getAge2Nodes() const;
		void setAge2Nodes(std::vector<std::shared_ptr<Node>> v);
		const std::vector<std::shared_ptr<Node>>& getAge3Nodes() const;
		void setAge3Nodes(std::vector<std::shared_ptr<Node>> v);
		std::deque<Models::Card*> getAvailableCardsByAge(int age) const;
    private:
        Board();
        Board(const Board&) = delete;
        Board& operator=(const Board&) = delete;
    public:
        static Board& getInstance() {
            static Board instance;
            return instance;
        }
    };
	export __declspec(dllexport) std::ostream& operator<<(std::ostream& out, const Board& board);
	export __declspec(dllexport) std::istream& operator>>(std::istream& in, Board& board);
}
