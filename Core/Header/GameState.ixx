export module Core.GameState;
import Core.Board;
import Core.Player;
import <string>;
import <memory>;
export namespace Core
{
	export class GameState {
	private:
		GameState();
		GameState(const GameState&) = delete;
		GameState& operator=(const GameState&) = delete;
	public:
		static GameState& getInstance() {
			static GameState instance;
			return instance;
		}
		std::shared_ptr<Player> GetPlayer1() { return m_player1; }
		std::shared_ptr<Player> GetPlayer2() { return m_player2; }
		std::shared_ptr<const Player> GetPlayer1() const { return m_player1; }
		std::shared_ptr<const Player> GetPlayer2() const { return m_player2; }
		void saveGameState(const std::string& filename);
		void loadGameState(const std::string& filename);
	private:
		Core::Board& m_board;
		std::shared_ptr<Core::Player> m_player1;
		std::shared_ptr<Core::Player> m_player2;
	};
}
