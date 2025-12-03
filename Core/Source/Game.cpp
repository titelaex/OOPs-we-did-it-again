module Core.Game;

import <vector>;
import <string>;
import <random>;
import <iostream>;
import <algorithm>;

import Core.Preparation;
import Core.Board;
import Core.Node;

import Models.AgeCard;
import Models.GuildCard;
import Models.Wonder;
import Models.ResourceType;
import Models.ScientificSymbolType;
import Models.LinkingSymbolType;
import Models.CoinWorthType;
import Models.ColorType;
import Models.Age;
import Models.TradeRuleType;
import Models.Token; // ensures TokenType is available
import Models.Card;
import Core.CardCsvParser;

void m_receiveMoneyAction(class Player& player) {}
void m_opponentLosesMoneyAction(class Player& opponent) {}
void m_playSecondTurnAction(class Player& player) {}

std::vector<Models::Token> randomTokenSelector(std::vector<Models::Token>& discardedTokens)
{
	const uint8_t tokensToSelect = 3;
	if (discardedTokens.size() <= tokensToSelect) return discardedTokens;
	std::vector<Models::Token> selectedTokens;
	std::random_device seed; std::mt19937 generator(seed());
	std::sample(discardedTokens.begin(), discardedTokens.end(), std::back_inserter(selectedTokens), tokensToSelect, generator);
	return selectedTokens;
}

std::pair<std::vector<Models::Token>, std::vector<Models::Token>> startGameTokens(const std::vector<Models::Token>& allTokens)
{
	std::vector<Models::Token> progress;
	std::vector<Models::Token> military;
	for (const auto& t : allTokens) {
		if (t.getType() == Models::TokenType::PROGRESS) progress.push_back(t);
		else if (t.getType() == Models::TokenType::MILITARY) military.push_back(t);
	}

	std::vector<Models::Token> selectedProgress;
	constexpr size_t kSelectCount = 5;
	if (progress.size() <= kSelectCount) {
		selectedProgress = progress;
	}
	else {
		std::random_device rd; std::mt19937 gen(rd());
		std::sample(progress.begin(), progress.end(), std::back_inserter(selectedProgress), kSelectCount, gen);
	}


	return { selectedProgress, military };
}

void m_drawProgressTokenAction(std::vector<Models::Token>& discardedTokens)
{
	auto selectedTokens = randomTokenSelector(discardedTokens);
}

void m_chooseAndConstructBuildingAction(const std::vector<Models::AgeCard>& discardedCards) {}
void m_discardCardFromOpponentAction(class Player& opponent, Models::ColorType color) {}

void movePawn(int steps) {
	Core::pawnTrack.reset();
	Core::pawnPos = std::clamp(Core::pawnPos + steps, 0, 18);
	Core::pawnTrack.set(Core::pawnPos);
}

namespace Core {
	void preparation()
	{
		try {
			Core::PrepareBoardCardPools();

			auto allTokens = ParseTokensFromCSV("Tokens.csv");
			auto [progressSelected, military] = startGameTokens(allTokens);
			Core::progressTokens = std::move(progressSelected);
			Core::militaryTokens = std::move(military);
		}
		catch (const std::exception& ex) {
			std::cerr << "Preparation exception: " << ex.what() << std::endl;
		}
		catch (...) {
			std::cerr << "Unknown exception during preparation" << std::endl;
		}
	}
}