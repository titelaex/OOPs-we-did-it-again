export module Models.Bank;

import Models.Token;
import <vector>;
import <tuple>;
import <random>;
import <algorithm>;
import <chrono>;
import <cstdint>;
import <utility>;

export namespace Models
{
	export class __declspec(dllexport) Bank
	{
	public:
		explicit Bank(std::tuple<uint8_t,uint8_t,uint8_t> startingCoins = {12,6,7}, unsigned int rngSeed =0);

		std::tuple<uint8_t, uint8_t, uint8_t> GetCoins() const noexcept;
		uint32_t GetRemainingCoins() const noexcept;
		bool TryWithdraw(uint32_t amount) noexcept;
		void Deposit(uint32_t amount) noexcept;
		/*void InitializeTokens(const std::vector<Token>& tokens);
		void InitializeTokens(std::vector<Token>&& tokens);
		void FillWithDefaultTokens();
		void ShuffleTokens();
		bool DrawToken(Token& outToken);
		std::vector<Token> DrawTokens(size_t count);
		void ReturnToken(const Token& token);
		size_t TokenCount() const noexcept;
		void ClearTokens() noexcept;*/

	private:
		std::tuple<uint8_t,uint8_t,uint8_t> m_coins = {0,0,0};
		
	};
}