module Models.Bank;
import <chrono>;
import <algorithm>;
import <random>;
import <utility>;
import Models.Bank;
import Models.Token;

using namespace Models;

//Bank::Bank(std::tuple<uint8_t,uint8_t,uint8_t> startingCoins, unsigned int rngSeed)
//	: m_coins(startingCoins)
//{
//	if (rngSeed == 0)
//		rngSeed = static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
//}

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

//void Bank::InitializeTokens(const std::vector<Token>& tokens) { m_tokens = tokens; }
//void Bank::InitializeTokens(std::vector<Token>&& tokens) { m_tokens = std::move(tokens); }
//void Bank::FillWithDefaultTokens() { m_tokens = CreateDefaultTokenSet(); }
//void Bank::ShuffleTokens() { std::shuffle(m_tokens.begin(), m_tokens.end(), m_rng); }
//
//bool Bank::DrawToken(Token& outToken) {
//	if (m_tokens.empty()) return false;
//	outToken = std::move(m_tokens.back());
//	m_tokens.pop_back();
//	return true;
//}
//
//std::vector<Token> Bank::DrawTokens(size_t count) {
//	std::vector<Token> result; result.reserve(count);
//	while (count-- > 0 && !m_tokens.empty()) { result.push_back(std::move(m_tokens.back())); m_tokens.pop_back(); }
//	return result;
//}
//
//void Bank::ReturnToken(const Token& token) { m_tokens.insert(m_tokens.begin(), token); }
//size_t Bank::TokenCount() const noexcept { return m_tokens.size(); }
//void Bank::ClearTokens() noexcept { m_tokens.clear(); }
