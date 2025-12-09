export module GameState;
import Core.Board;
import Core.Player;
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
		// access to players for global helpers
		Player* GetPlayer1() { return &m_player1; }
		Player* GetPlayer2() { return &m_player2; }
	private:
		Core::Board& m_board;
		Core::Player m_player1, m_player2;
	};
}
