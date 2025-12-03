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
	private:
		std::tuple<uint8_t,uint8_t,uint8_t> m_coins = {0,0,0};
	public:
		//explicit Bank(std::tuple<uint8_t,uint8_t,uint8_t> startingCoins = {12,6,7}, unsigned int rngSeed =0);
		Bank();
		std::tuple<uint8_t, uint8_t, uint8_t> getCoins() const noexcept;
		uint32_t getRemainingCoins() const noexcept;
		bool tryWithdraw(uint32_t amount) noexcept;
		void deposit(uint32_t amount) noexcept;
		/*void InitializeTokens(const std::vector<Token>& tokens);
		void InitializeTokens(std::vector<Token>&& tokens);
		void FillWithDefaultTokens();
		void ShuffleTokens();
		bool DrawToken(Token& outToken);
		std::vector<Token> DrawTokens(size_t count);
		void ReturnToken(const Token& token);
		size_t TokenCount() const noexcept;
		void ClearTokens() noexcept;*/

		
	};
}