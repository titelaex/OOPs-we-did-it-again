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
		Bank();
		std::tuple<uint8_t, uint8_t, uint8_t> getCoins() const noexcept;
		uint32_t getRemainingCoins() const noexcept;
		bool tryWithdraw(uint32_t amount) noexcept;
		void deposit(uint32_t amount) noexcept;

		
	};
}