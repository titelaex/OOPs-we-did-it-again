module Core.Game;

import <vector>;
import <string>;
import <random>;
import <iostream>;
import <algorithm>;
import <utility>;


import Core.GamePhases;
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
import Models.Token; 
import Models.Card;
import Core.CardCsvParser;
import Core.Player;

std::unique_ptr<std::vector<std::unique_ptr<Models::Token>>> setupUnusedProgressTokens;

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

std::pair<std::vector<std::unique_ptr<Models::Token>>, std::vector<std::unique_ptr<Models::Token>>> startGameTokens(std::vector<std::unique_ptr<Models::Token>> allTokens)
{
  
    if (!setupUnusedProgressTokens) setupUnusedProgressTokens = std::make_unique<std::vector<std::unique_ptr<Models::Token>>>();
    setupUnusedProgressTokens->clear();

    std::vector<std::unique_ptr<Models::Token>> progress;
    std::vector<std::unique_ptr<Models::Token>> military;
    progress.reserve(allTokens.size()); military.reserve(allTokens.size());
    for (auto &t : allTokens) {
        if (!t) continue;
        if (t->getType() == Models::TokenType::PROGRESS) progress.push_back(std::move(t));
        else if (t->getType() == Models::TokenType::MILITARY) military.push_back(std::move(t));
    }

    std::vector<std::unique_ptr<Models::Token>> selectedProgress;
    constexpr size_t kSelectCount = 5;
    if (progress.size() <= kSelectCount) {
        // move all
        selectedProgress = std::move(progress);
    }
    else {
        std::random_device rd; std::mt19937 gen(rd());
        std::vector<size_t> idx(progress.size());
        for (size_t i = 0; i < idx.size(); ++i) idx[i] = i;
        std::shuffle(idx.begin(), idx.end(), gen);
        for (size_t i = 0; i < idx.size(); ++i) {
            if (i < kSelectCount) selectedProgress.push_back(std::move(progress[idx[i]]));
            else setupUnusedProgressTokens->push_back(std::move(progress[idx[i]]));
        }
    }

	return { std::move(selectedProgress), std::move(military) };
}

void m_drawProgressTokenAction(std::vector<Models::Token>& discardedTokens)
{
	auto selectedTokens = randomTokenSelector(discardedTokens);
}
void m_chooseAndConstructBuildingAction(const std::vector<Models::AgeCard>& discardedCards) {}
void m_discardCardFromOpponentAction(class Player& opponent, Models::ColorType color) {}

void movePawn(int steps) {
	auto track= Core::Board::getInstance().getPawnTrack();
	track.reset();
	auto position = Core::Board::getInstance().getPawnPos();
	position = std::clamp(position + steps, 0, 18);
	Core::Board::getInstance().setPawnPos(position);
	track.set(position);
    Core::Board::getInstance().setPawnTrack(track);
}

namespace Core {
	void preparation()
	{
		try {
			Core::PrepareBoardCardPools();

			auto allTokens = parseTokensFromCSV("Tokens.csv");
			auto [progressSelected, military] = startGameTokens(std::move(allTokens));
			Core::Board::getInstance().setProgressTokens(std::move(progressSelected));
			Core::Board::getInstance().setMilitaryTokens(std::move(military));
			Core::Board::getInstance().setUnusedProgressTokens(std::move(*setupUnusedProgressTokens));
		}
		catch (const std::exception& ex) {
			std::cerr << "Preparation exception: " << ex.what() << std::endl;
		}
		catch (...) {
			std::cerr << "Unknown exception during preparation" << std::endl;
		}
	}

    ///O stergem?
    /*void greatLibraryDrawFromSetup()
    {
        Core::Player* cp = Core::getCurrentPlayer();
        if (!cp) return;
        const auto& pool = (setupDiscardedProgressTokens && !setupDiscardedProgressTokens->empty()) ? *setupDiscardedProgressTokens : Core::Board::getInstance().getProgressTokens();
        if (pool.empty()) return;

        std::vector<size_t> indices(pool.size());
        for (size_t i = 0; i < indices.size(); ++i) indices[i] = i;
        std::random_device rd; std::mt19937 gen(rd());
        std::shuffle(indices.begin(), indices.end(), gen);
        size_t pickCount = std::min<size_t>(3, indices.size());
        indices.resize(pickCount);

        std::cout << "The Great Library: choose one of the drawn progress tokens:\n";
        for (size_t i = 0; i < pickCount; ++i) {
            const auto& tptr = pool[indices[i]];
            if (tptr) std::cout << "[" << i << "] " << tptr->getName() << "\n";
            else std::cout << "[" << i << "] <empty>\n";
        }

        size_t choice = 0;
        if (!(std::cin >> choice) || choice >= pickCount) {
            choice = 0;
            if (!std::cin) { std::cin.clear(); std::string discard; std::getline(std::cin, discard); }
        }

        const auto& chosenPtr = pool[indices[choice]];
        if (!chosenPtr) return;

        if (setupDiscardedProgressTokens && !setupDiscardedProgressTokens->empty()) {
            size_t chosenIndex = indices[choice];
            if (chosenIndex < setupDiscardedProgressTokens->size()) {
                std::unique_ptr<Models::Token> movedToken = std::move((*setupDiscardedProgressTokens)[chosenIndex]);
                setupDiscardedProgressTokens->erase(setupDiscardedProgressTokens->begin() + chosenIndex);
                if (movedToken) cp->m_player->addToken(std::move(movedToken));
            }
        }
        else {
            auto moved = removeProgressTokenByName(chosenPtr->getName());
            if (moved) cp->m_player->addToken(std::move(moved));
        }
    }
    */

}

