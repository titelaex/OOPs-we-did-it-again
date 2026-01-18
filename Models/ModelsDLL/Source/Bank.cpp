module Models.Bank;
import <chrono>;
import <algorithm>;
import <random>;
import <utility>;
import Models.Bank;
import Models.Token;

using namespace Models;

Bank::Bank():m_coins({12,6,7}){}


std::tuple<uint8_t, uint8_t, uint8_t> Bank::getCoins() const noexcept {
	return m_coins;
}

uint32_t Bank::getRemainingCoins() const noexcept {
	return std::get<0>(m_coins) * 1 + std::get<1>(m_coins) * 3 + std::get<2>(m_coins) * 6;
}

bool Bank::tryWithdraw(uint32_t amount) noexcept {
	if (amount == 0) return true;
	const uint32_t total = getRemainingCoins();
	if (amount > total) return false;
	uint8_t availableOnes = std::get<0>(m_coins);
	uint8_t availableThrees = std::get<1>(m_coins);
	uint8_t availableSixes = std::get<2>(m_coins);
	uint8_t maxSixes = static_cast<uint8_t>(std::min<uint32_t>(availableSixes, amount / 6u));
	for (int s = static_cast<int>(maxSixes); s >= 0; --s) {
		uint32_t remAfterSix = amount - static_cast<uint32_t>(s) * 6u;
		uint8_t maxThrees = static_cast<uint8_t>(std::min<uint32_t>(availableThrees, remAfterSix / 3u));
		for (int t = static_cast<int>(maxThrees); t >= 0; --t) {
			uint32_t rem = remAfterSix - static_cast<uint32_t>(t) * 3u;
			if (rem <= availableOnes) {
				std::get<2>(m_coins) = static_cast<uint8_t>(availableSixes - s);
				std::get<1>(m_coins) = static_cast<uint8_t>(availableThrees - t);
				std::get<0>(m_coins) = static_cast<uint8_t>(availableOnes - rem);
				return true;
			}
		}
	}
	return false;
}

void Bank::deposit(uint32_t amount) noexcept {
	if (amount == 0) return;
	uint8_t& ones = std::get<0>(m_coins);
	uint8_t& threes = std::get<1>(m_coins);
	uint8_t& sixes = std::get<2>(m_coins);
	uint32_t addSixes = amount / 6u;
	if (addSixes > 0) { sixes = static_cast<uint8_t>(sixes + static_cast<uint8_t>(addSixes)); amount -= addSixes * 6u; }
	uint32_t addThrees = amount / 3u;
	if (addThrees > 0) { threes = static_cast<uint8_t>(threes + static_cast<uint8_t>(addThrees)); amount -= addThrees * 3u; }
	if (amount > 0) { ones = static_cast<uint8_t>(ones + static_cast<uint8_t>(amount)); }
}

