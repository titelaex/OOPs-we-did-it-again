

module Core.Player;

#include <compare>
import <iostream>;
import <vector>;
import <tuple>;
import <unordered_map>;
import <memory>;
import <random>;
import <algorithm>;
import <sstream>;

import Models.Wonder;
import Models.Card;
import Models.Player;
import Models.Token;
import Models.LinkingSymbolType;
import GameState;
import Core.Board;

import Models.AgeCard;
import Models.GuildCard;
import Core.CardCsvParser;


namespace Core { void awardMilitaryTokenIfPresent(class Player& receiver); }
extern void movePawn(int steps);
namespace Core {
	namespace {
		thread_local Player* g_current_player = nullptr;
	}


	void setCurrentPlayer(Player* p) { g_current_player = p; }
	Player* getCurrentPlayer() { return g_current_player; }
}

namespace {
    void streamCardByType(std::ostream& out, const Models::Card* card)
    {
        if (!card) return;
        if (const auto* ageCard = dynamic_cast<const Models::AgeCard*>(card)) {
            out << *ageCard;
        }
        else if (const auto* wonder = dynamic_cast<const Models::Wonder*>(card)) {
            out << *wonder;
        }
        else if (const auto* guild = dynamic_cast<const Models::GuildCard*>(card)) {
            out << *guild;
        }
        else {
            out << *card;
        }
    }
}


Core::Player* Core::getOpponentPlayer()
{
	Core::Player* cp = getCurrentPlayer();
	if (!cp) return nullptr;
	auto& gs = Core::GameState::getInstance();
	Player* p1 = gs.GetPlayer1();
	Player* p2 = gs.GetPlayer2();
	if (p1 == cp) return p2;
	if (p2 == cp) return p1;
	return nullptr;
}

void Core::playTurnForCurrentPlayer()
{
	Core::Player* cp = getCurrentPlayer();
	if (!cp) return;
	std::cout << "Playing an extra turn for player\n";
}

void Core::drawTokenForCurrentPlayer()
{
	Core::Player* cp = getCurrentPlayer();
	if (!cp) return;
	std::vector<std::unique_ptr<Models::Token>> combined;
	auto& progressTokens = const_cast<std::vector<std::unique_ptr<Models::Token>>&>(Board::getInstance().getProgressTokens());
	auto& militaryTokens = const_cast<std::vector<std::unique_ptr<Models::Token>>&>(Board::getInstance().getMilitaryTokens());

	for (auto& t : progressTokens) combined.push_back(std::move(t));
	for (auto& t : militaryTokens) combined.push_back(std::move(t));
	if (combined.empty()) return;
	std::random_device rd; std::mt19937 gen(rd());
	std::shuffle(combined.begin(), combined.end(), gen);
	size_t pickCount = std::min<size_t>(3, combined.size());

	std::cout << "Choose a token: \n";
	for (size_t i = 0; i < pickCount; ++i) std::cout << "[" << i << "] " << combined[i]->getName() << "\n";
	size_t choice = 0; std::cin >> choice; if (choice >= pickCount) choice = 0;

	auto chosen = std::move(combined[choice]);
	if (chosen) cp->m_player->addToken(std::move(chosen));

	std::vector<std::unique_ptr<Models::Token>> newProgress;
	std::vector<std::unique_ptr<Models::Token>> newMilitary;
	for (auto& tptr : combined) {
		if (!tptr) continue;
		if (tptr->getType() == Models::TokenType::PROGRESS) newProgress.push_back(std::move(tptr));
		else newMilitary.push_back(std::move(tptr));
	}
	Board::getInstance().setProgressTokens(std::move(newProgress));
	Board::getInstance().setMilitaryTokens(std::move(newMilitary));
}


void Core::discardOpponentCardOfColor(Models::ColorType color)
{
	Core::Player* cp = getCurrentPlayer();
	if (!cp) return;
	Core::Player* opponent = getOpponentPlayer();
	if (!opponent) return;
	auto& owned = opponent->m_player->getOwnedCards();
	std::vector<size_t> candidates;
	for (size_t i = 0; i < owned.size(); ++i) {
		if (!owned[i]) continue;
		if (owned[i]->getColor() == color) candidates.push_back(i);
	}
	if (candidates.empty()) return;
	std::cout << "Choose opponent card to discard:\n";
	for (size_t idx = 0; idx < candidates.size(); ++idx) {
		size_t i = candidates[idx];
		std::cout << "[" << idx << "] " << owned[i]->getName() << "\n";
	}
	size_t choice = 0; std::cin >> choice; if (choice >= candidates.size()) choice = 0;
	size_t removeIdx = candidates[choice];
	// move card into board::discardedCards
	auto moved = opponent->m_player->removeOwnedCardAt(removeIdx);
	if (moved)
	{
		auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(Core::Board::getInstance().getDiscardedCards());
		discarded.push_back(std::move(moved));
	}
}

void Core::Player::chooseWonder(std::vector<std::unique_ptr<Models::Wonder>>& availableWonders, uint8_t chosenIndex)
{
	if (availableWonders.empty())
	{
		std::cout << "No available wonders to choose from->\n";
		return;
	}

	if (availableWonders.size() == 1)
	{
		m_player->addWonder(std::move(availableWonders[0]));
		availableWonders.erase(availableWonders.begin());
		std::cout << "Only one wonder available. Automatically chosen: " << m_player->getOwnedWonders().back()->getName() << "->\n";
		return;
	}

	if (chosenIndex >= availableWonders.size())
	{
		std::cout << "Invalid wonder index->\n";
		return;
	}
	auto& chosenWonder = availableWonders[chosenIndex];
	m_player->addWonder(std::move(chosenWonder));
	availableWonders.erase(availableWonders.begin() + chosenIndex);
	std::cout << "Chosen wonder: " << m_player->getOwnedWonders().back()->getName() << "->\n";

}

void Core::Player::sellCard(std::unique_ptr<Models::Card>& ageCard, std::vector<std::unique_ptr<Models::Card>>& discardedCards)
{
	// 1. Calculate Base Value
	uint8_t coinsToGain = 2;

	// 2. Calculate Yellow Card Bonus

	uint8_t yellowCardCount = countYellowCards();
	coinsToGain += yellowCardCount;

	// 3. Add coins to Treasury
	addCoins(coinsToGain);

	std::cout << "Player sold \"" << ageCard->getName() << "\" for "
		<< static_cast<int>(coinsToGain) << " coins.\n";

	// 4. Move card to Discard Pile
	discardedCards.push_back(std::move(ageCard));
}

uint8_t Core::Player::countYellowCards()
{
	uint8_t count = 0;
	for (const auto& p : m_player->getOwnedCards()) {
		if (!p) continue;
		if (p->getColor() == Models::ColorType::YELLOW) ++count;
	}
	return count;
}


// WONDER GAMEPLAY METHODS

void Core::Player::playCardWonder(std::unique_ptr<Models::Wonder>& wonder, std::unique_ptr<Models::Card>& ageCard, std::unique_ptr<Models::Player>& opponent,
	std::vector<Models::Token>& discardedTokens, std::vector<std::unique_ptr<Models::Card>>& discardedCards)
{
	if (Models::Wonder::getWondersBuilt() >= Models::Wonder::MaxWonders)
	{
		std::cout << "All wonders have been built in the game->\n";
		return;
	}

	if (wonder->IsConstructed())
	{
		std::cout << "Wonder \"" << wonder->getName() << "\" is already constructed->\n";
		return;
	}

	if (!canAffordWonder(wonder, opponent))
	{
		std::cout << "Cannot afford to construct wonder \"" << wonder->getName() << "\"->\n";
		return;
	}

	payForWonder(wonder, opponent);

	Models::Wonder* rawWonderPtr = wonder.release();
	std::unique_ptr<Models::Card> tempCard(rawWonderPtr);
	applyCardEffects(tempCard);
	rawWonderPtr = static_cast<Models::Wonder*>(tempCard.release());
	wonder.reset(rawWonderPtr);

	wonder->setConstructed(true);
	const auto builtCount = Models::Wonder::incrementWondersBuilt();
	if (builtCount == Models::Wonder::MaxWonders)
	{
		std::cout << "All wonders have been built in the game->\n";
		discardRemainingWonder(opponent);
	}
	//ageCard.reset()
	wonder->attachUnderCard(std::move(ageCard));

	if (wonder->getShieldPoints() > 0)
	{
		movePawn(static_cast<int>(wonder->getShieldPoints()));

		Core::awardMilitaryTokenIfPresent(*this);
	}

	std::cout << "Wonder \"" << wonder->getName() << "\" constructed successfully->\n";
}

bool Core::Player::canAffordWonder(std::unique_ptr<Models::Wonder>& wonder, const std::unique_ptr<Models::Player>& opponent)
{

	const auto& cost = wonder->getResourceCost();
	const auto& ownPermanent = m_player->getOwnedPermanentResources();
	const auto& ownTrading = m_player->getOwnedTradingResources();
	uint8_t availableCoins = m_player->totalCoins(m_player->getRemainingCoins());

	std::unordered_map<Models::ResourceType, uint8_t> missingResources;
	for (const auto& [resource, requiredAmount] : cost) {
		uint8_t produced = 0;
		if (ownPermanent.count(resource)) produced += ownPermanent.at(resource);
		if (ownTrading.count(resource)) produced += ownTrading.at(resource);

		if (produced < requiredAmount) {
			missingResources[resource] = requiredAmount - produced;
		}
	}

	if (missingResources.empty()) {
		return true;
	}

	bool hasArchitectureToken = false;
	for (const auto& token : m_player->getOwnedTokens()) {
		if (token->getName() == "Architecture") {
			hasArchitectureToken = true;
			break;
		}
	}
	auto getTradeDiscount = [&](Models::ResourceType resource) -> int {
		const auto& tradeRules = m_player->getTradeRules();
		Models::TradeRuleType ruleType;
		switch (resource) {
		case Models::ResourceType::WOOD:    ruleType = Models::TradeRuleType::WOOD; break;
		case Models::ResourceType::STONE:   ruleType = Models::TradeRuleType::STONE; break;
		case Models::ResourceType::CLAY:    ruleType = Models::TradeRuleType::CLAY; break;
		case Models::ResourceType::PAPYRUS: ruleType = Models::TradeRuleType::PAPYRUS; break;
		case Models::ResourceType::GLASS:   ruleType = Models::TradeRuleType::GLASS; break;
		default: return -1;
		}

		auto it = tradeRules.find(ruleType);
		if (it != tradeRules.end() && it->second) {
			return 1;
		}

		return -1;
		};

	std::unordered_map<Models::ResourceType, uint8_t> opponentBrownGreyProduction;
	for (const auto& card : opponent->getOwnedCards())
	{
		if (card && (card->getColor() == Models::ColorType::BROWN || card->getColor() == Models::ColorType::GREY))
		{
			if (const auto* ageCard = dynamic_cast<const Models::AgeCard*>(card.get()))
			{
				for (const auto& resourcePair : ageCard->getResourcesProduction())
				{
					if (getTradeDiscount(resourcePair.first) == -1)
					{
						opponentBrownGreyProduction[resourcePair.first] += resourcePair.second;
					}
				}
			}
		}
	}


	uint8_t totalCost = 0;

	if (hasArchitectureToken) {
		std::vector<std::pair<uint8_t, Models::ResourceType>> purchaseCosts;
		for (const auto& [resource, amount] : missingResources) {
			int discountedCost = getTradeDiscount(resource);
			uint8_t opponentAmount = opponentBrownGreyProduction.count(resource) ? opponentBrownGreyProduction.at(resource) : 0;
			uint8_t costPerUnit = (discountedCost != -1) ? discountedCost : (2 + opponentAmount);
			purchaseCosts.push_back({ costPerUnit, resource });
		}
		std::sort(purchaseCosts.rbegin(), purchaseCosts.rend());

		int discount = 2;
		for (auto& costPair : purchaseCosts) {
			auto resource = costPair.second;
			uint8_t needed = missingResources[resource];
			uint8_t discountedAmount = std::min((int)needed, discount);
			missingResources[resource] -= discountedAmount;
			discount -= discountedAmount;
			if (discount <= 0) break;
		}
	}

	for (const auto& [resource, amount] : missingResources) {
		if (amount == 0) continue;
		int discountedCost = getTradeDiscount(resource);
		uint8_t opponentAmount = opponentBrownGreyProduction.count(resource) ? opponentBrownGreyProduction.at(resource) : 0;
		uint8_t costPerUnit = (discountedCost != -1) ? discountedCost : (2 + opponentAmount);
		totalCost += costPerUnit * amount;
	}

	return availableCoins >= totalCost;
}

void Core::Player::payForWonder(std::unique_ptr<Models::Wonder>& wonder, const std::unique_ptr<Models::Player>& opponent)
{

	const auto& cost = wonder->getResourceCost();
	const auto& ownPermanent = m_player->getOwnedPermanentResources();
	const auto& ownTrading = m_player->getOwnedTradingResources();

	std::unordered_map<Models::ResourceType, uint8_t> missingResources;
	int totalMissingUnits = 0;

	for (const auto& [resource, requiredAmount] : cost) {

		uint8_t produced = 0;
		if (ownPermanent.count(resource)) produced += ownPermanent.at(resource);
		if (ownTrading.count(resource)) produced += ownTrading.at(resource);

		if (produced < requiredAmount) {
			uint8_t missing = requiredAmount - produced;
			missingResources[resource] = missing;
			totalMissingUnits += missing;
		}
	}

	if (missingResources.empty()) {
		std::cout << "Player constructed the wonder for free (sufficient resources owned).\n";
		return;
	}

	bool hasArchitectureToken = false;
	for (const auto& token : m_player->getOwnedTokens()) {
		if (token->getName() == "Architecture") {
			hasArchitectureToken = true;
			break;
		}
	}


	if (hasArchitectureToken && totalMissingUnits <= 2) {
		std::cout << "Player constructed the wonder for free (covered fully by Architecture token).\n";
		return;
	}

	auto getTradeDiscount = [&](Models::ResourceType resource) -> int {
		const auto& tradeRules = m_player->getTradeRules();
		Models::TradeRuleType ruleType;
		switch (resource) {
		case Models::ResourceType::WOOD:    ruleType = Models::TradeRuleType::WOOD; break;
		case Models::ResourceType::STONE:   ruleType = Models::TradeRuleType::STONE; break;
		case Models::ResourceType::CLAY:    ruleType = Models::TradeRuleType::CLAY; break;
		case Models::ResourceType::PAPYRUS: ruleType = Models::TradeRuleType::PAPYRUS; break;
		case Models::ResourceType::GLASS:   ruleType = Models::TradeRuleType::GLASS; break;
		default: return -1;
		};
		auto it = tradeRules.find(ruleType);
		if (it != tradeRules.end() && it->second) return 1;
		return -1;
		};

	std::unordered_map<Models::ResourceType, uint8_t> opponentBrownGreyProduction;
	bool opponentCheckNeeded = false;

	for (const auto& [resource, amount] : missingResources) {
		if (getTradeDiscount(resource) == -1) {
			opponentCheckNeeded = true;
			break;
		}
	}

	if (opponentCheckNeeded) {
		for (const auto& card : opponent->getOwnedCards()) {
			if (card && (card->getColor() == Models::ColorType::BROWN || card->getColor() == Models::ColorType::GREY)) {
				if (const auto* ageCard = dynamic_cast<const Models::AgeCard*>(card.get())) {
					for (const auto& resourcePair : ageCard->getResourcesProduction()) {
						if (missingResources.find(resourcePair.first) != missingResources.end()) {
							opponentBrownGreyProduction[resourcePair.first] += resourcePair.second;
						}
					}
				}
			}
		}
	}

	std::vector<uint8_t> purchaseCosts;

	for (const auto& [resource, amount] : missingResources) {
		int discountedCost = getTradeDiscount(resource);
		uint8_t costPerUnit = 0;

		if (discountedCost != -1) {
			costPerUnit = 1;
		}
		else {
			uint8_t opponentAmount = opponentBrownGreyProduction.count(resource) ? opponentBrownGreyProduction.at(resource) : 0;
			costPerUnit = 2 + opponentAmount;
		}

		for (int i = 0; i < amount; ++i) {
			purchaseCosts.push_back(costPerUnit);
		}
	}

	if (hasArchitectureToken) {
		std::sort(purchaseCosts.rbegin(), purchaseCosts.rend());
		if (purchaseCosts.size() >= 2) {
			purchaseCosts.erase(purchaseCosts.begin(), purchaseCosts.begin() + 2);
		}
	}

	uint8_t totalTradingCost = 0;
	for (uint8_t price : purchaseCosts) {
		totalTradingCost += price;
	}

	if (totalTradingCost > 0) {
		subtractCoins(totalTradingCost);
		std::cout << "Player paid " << static_cast<int>(totalTradingCost) << " coins for trading resources.\n";
	}
	else {//n ar trebui sa se ajunga aici
		std::cout << "Player constructed the wonder for free.\n";
	}
}

void Core::Player::discardRemainingWonder(const std::unique_ptr<Models::Player>& opponent)
{
	auto discardFromList = [](std::vector<std::unique_ptr<Models::Wonder>>& wonderList) -> bool {

		for (auto it = wonderList.begin(); it != wonderList.end(); ++it) {
			if (*it && !(*it)->IsConstructed()) {

				std::cout << "7 Wonders Rule: Removing unbuilt wonder \""
					<< (*it)->getName() << "\" from the game.\n";

				wonderList.erase(it);
				return true;
			}
		}
		return false;
		};

	if (discardFromList(m_player->getOwnedWonders())) {
		return;
	}

	if (discardFromList(opponent->getOwnedWonders())) {
		return;
	}

	std::cout << "Warning: Could not find the 8th unbuilt wonder to discard.\n";
}



void Core::Player::playCardBuilding(std::unique_ptr<Models::Card>& card, std::unique_ptr<Models::Player>& opponent)
{
	if (!card->isVisible())
	{
		std::cout << "Card \"" << card->getName() << "\" is not accessible\n";
		return;
	}



	if (card->getRequiresLinkingSymbol() != Models::LinkingSymbolType::NO_SYMBOL)
	{
		for (const auto& ownedCard : m_player->getOwnedCards())
		{
			if (ownedCard->getHasLinkingSymbol() == card->getRequiresLinkingSymbol())
			{
				m_player->addCard(card);
				card->setIsVisible(false);
				std::cout << "Card \"" << card->getName() << "\" constructed for free via chain->\n";
				applyCardEffects(card);
				return;
			}
		}
	}


	if (card->getResourceCost().empty())
	{
		m_player->addCard(card);
		card->setIsVisible(false);
		std::cout << "Card \"" << card->getName() << "\" constructed for free->\n";
		applyCardEffects(card);
		return;
	}

	if (!canAffordCard(card, opponent))
	{
		std::cout << "Cannot afford to construct \"" << card->getName() << "\"->\n";
		return;
	}

	payForCard(card, opponent);

	m_player->addCard(card);
	card->setIsVisible(false);
	std::cout << "Card \"" << card->getName() << "\" constructed->\n";
	applyCardEffects(card);
}

bool Core::Player::canAffordCard(std::unique_ptr<Models::Card>& card, std::unique_ptr<Models::Player>& opponent)
{
	if (card->getResourceCost().empty())
		return true;


	if (card->getRequiresLinkingSymbol() != Models::LinkingSymbolType::NO_SYMBOL)
	{
		for (const auto& ownedCard : m_player->getOwnedCards())
		{
			if (ownedCard->getHasLinkingSymbol() == card->getRequiresLinkingSymbol())
				return true;
		}
	}

	const auto& cost = card->getResourceCost();
	const auto& ownPermanent = m_player->getOwnedPermanentResources();
	const auto& ownTrading = m_player->getOwnedTradingResources();
	const auto& opponentProduction = opponent->getOwnedPermanentResources();

	uint8_t availableCoins = m_player->totalCoins(m_player->getRemainingCoins());

	for (const auto& kv : cost)
	{
		auto resource = kv.first;
		auto requiredAmount = kv.second;

		uint8_t produced = 0;
		if (ownPermanent.find(resource) != ownPermanent.end())
			produced += ownPermanent.at(resource);
		if (ownTrading.find(resource) != ownTrading.end())
			produced += ownTrading.at(resource);

		if (produced >= requiredAmount)
			continue;

		uint8_t missing = requiredAmount - produced;
		uint8_t opponentAmount = (opponentProduction.find(resource) != opponentProduction.end()) ? opponentProduction.at(resource) : 0;
		uint8_t costPerUnit = 2 + opponentAmount;

		uint8_t totalCost = costPerUnit * missing;
		if (availableCoins < totalCost)
			return false;
	}
	return true;
}

void Core::Player::payForCard(std::unique_ptr<Models::Card>& card, std::unique_ptr<Models::Player>& opponent)
{
	const auto& cost = card->getResourceCost();
	const auto& ownPermanent = m_player->getOwnedPermanentResources();
	const auto& ownTrading = m_player->getOwnedTradingResources();
	const auto& opponentPermanent = opponent->getOwnedPermanentResources();

	uint8_t totalCoinsToPay = 0;

	for (const auto& kv : cost)
	{
		auto resource = kv.first;
		auto requiredAmount = kv.second;

		uint8_t produced = 0;
		if (ownPermanent.find(resource) != ownPermanent.end()) produced += ownPermanent.at(resource);
		if (ownTrading.find(resource) != ownTrading.end()) produced += ownTrading.at(resource);

		if (produced >= requiredAmount)
			continue;

		uint8_t missing = requiredAmount - produced;
		uint8_t opponentProduction = (opponentPermanent.find(resource) != opponentPermanent.end()) ? opponentPermanent.at(resource) : 0;
		uint8_t costPerUnit = 2 + opponentProduction;

		totalCoinsToPay += costPerUnit * missing;
	}

	subtractCoins(totalCoinsToPay);
}

void Core::Player::applyCardEffects(std::unique_ptr<Models::Card>& card)
{
	std::cout << "Applying effects of card \"" << card->getName() << "\"->\n";
	card->onPlay();
}

void Core::Player::takeCard(std::unique_ptr<Models::Card> card)
{
	const auto& oldActions = card->getOnPlayActions();
	Models::CardBuilder builder;
	builder.setName(card->getName())
		.setResourceCost(card->getResourceCost())
		.setVictoryPoints(card->getVictoryPoints())
		.setCaption(card->getCaption())
		.setColor(card->getColor());

	for (const auto& act : oldActions) {
		builder.addOnPlayAction([act]() {
			Core::Player* cp = getCurrentPlayer();
			if (act) act();
			});
	}

	auto newCard = std::make_unique<Models::Card>(builder.build());

	std::cout << "Player takes card: " << newCard->getName() << "\n";
	{
		Core::Player* cp = getCurrentPlayer();
		newCard->onPlay();
	}
	m_player->addCard(std::move(newCard));
}

void Core::Player::addCoins(uint8_t amt)
{
	auto coins = m_player->getRemainingCoins();
	uint32_t total = m_player->totalCoins(coins) + amt;
	uint8_t sixes = static_cast<uint8_t>(total / 6u);
	uint8_t rem = static_cast<uint8_t>(total % 6u);
	uint8_t threes = static_cast<uint8_t>(rem / 3u);
	uint8_t ones = static_cast<uint8_t>(rem % 3u);
	m_player->setRemainingCoins({ ones, threes, sixes });
}

void Core::Player::subtractCoins(uint8_t amt)
{
	auto coins = m_player->getRemainingCoins();
	uint32_t total = m_player->totalCoins(coins);
	if (amt >= total) { m_player->setRemainingCoins({ 0,0,0 }); return; }
	uint32_t newTotal = total - amt;
	uint8_t sixes = static_cast<uint8_t>(newTotal / 6u);
	uint8_t rem = static_cast<uint8_t>(newTotal % 6u);
	uint8_t threes = static_cast<uint8_t>(rem / 3u);
	uint8_t ones = static_cast<uint8_t>(rem % 3u);
	m_player->setRemainingCoins({ ones, threes, sixes });
}

namespace Core
{
	void chooseTokenByIndex(std::vector<std::unique_ptr<Models::Token>>& tokens)
	{
		Core::Player* cp = getCurrentPlayer();
		if (!cp) return;

		if (tokens.empty()) return;

		std::cout << "Choose a progress token by index:\n";
		for (size_t i = 0; i < tokens.size(); ++i) {
			if (tokens[i]) std::cout << "[" << i << "] " << tokens[i]->getName() << "\n";
		}

		size_t idx = 0;
		if (!(std::cin >> idx) || idx >= tokens.size()) {
			if (!std::cin) { std::cin.clear(); std::string discard; std::getline(std::cin, discard); }
			idx = 0;
		}

		std::unique_ptr<Models::Token> taken = std::move(tokens[idx]);
		tokens.erase(tokens.begin() + idx);
		if (taken) cp->m_player->addToken(std::move(taken));
	}

	void chooseTokenByName(std::vector<std::unique_ptr<Models::Token>>& tokens)
	{
		Core::Player* cp = getCurrentPlayer();
		if (!cp) return;

		if (tokens.empty()) return;

		std::cout << "Choose a progress token by name:\n";
		for (size_t i = 0; i < tokens.size(); ++i) {
			if (tokens[i]) std::cout << "[" << i << "] " << tokens[i]->getName() << "\n";
		}

		std::string name;
		if (!std::getline(std::cin >> std::ws, name)) return;

		if (name.empty()) {
			if (!tokens.empty() && tokens[0]) {
				cp->m_player->addToken(std::move(tokens[0]));
				tokens.erase(tokens.begin());
			}
			return;

		}

		for (auto it = tokens.begin(); it != tokens.end(); ++it) {
			if (*it && (*it)->getName() == name) {
				std::unique_ptr<Models::Token> taken = std::move(*it);
				tokens.erase(it);
				if (taken) cp->m_player->addToken(std::move(taken));
				return;
			}
		}

		std::cout << "No progress token named '" << name << "' found." << std::endl;
	}
}

namespace Core {
	std::ostream& operator<<(std::ostream& out, const Player& player)
	{
		if (!player.m_player) {
			return out;
		}

		const auto& p = player.m_player;
		auto csvEscape = [](const std::string& s) -> std::string {
			if (s.find_first_of(",\"\n\r") != std::string::npos) {
				std::string out; out.reserve(s.size() + 2);
				out.push_back('"');
				for (char ch : s) {
					if (ch == '"') out += "\"\""; else out.push_back(ch);
				}
				out.push_back('"');
				return out;
			}
			return s;
			};

		out << "Player,Username," << csvEscape(p->getPlayerUsername()) << "\n";
		out << "Player,Coins," << static_cast<int>(p->totalCoins(p->getRemainingCoins())) << "\n";

		for (const auto& card : p->getOwnedCards()) {
			if (card) {
				out << "Player,Card,";
                streamCardByType(out, card.get());
                out << "\n";
			}
		}

		for (const auto& wonder : p->getOwnedWonders()) {
			if (wonder) {
				out << "Player,Wonder,";
                streamCardByType(out, wonder.get());
                out << "\n";
			}
		}

		for (const auto& token : p->getOwnedTokens()) {
			if (token) {
				out << "Player,Token," << csvEscape(token->getName()) << "\n";
			}
		}

		for (const auto& resource : p->getOwnedPermanentResources()) {
			out << "Player,PermanentResource," << static_cast<int>(resource.first) << ':' << static_cast<int>(resource.second) << "\n";
		}

		for (const auto& resource : p->getOwnedTradingResources()) {
			out << "Player,TradingResource," << static_cast<int>(resource.first) << ':' << static_cast<int>(resource.second) << "\n";
		}

		for (const auto& symbol : p->getOwnedScientificSymbols()) {
			out << "Player,ScientificSymbol," << static_cast<int>(symbol.first) << ':' << static_cast<int>(symbol.second) << "\n";
		}

		return out;
	}

	namespace {
		std::vector<std::string> splitCsvLine(const std::string& line) {
			std::vector<std::string> columns;
			std::stringstream ss(line);
			std::string cell;
			bool in_quotes = false;
			char c;

			while (ss.get(c)) {
				if (c == '"') {
					in_quotes = !in_quotes;
				}
				else if (c == ',' && !in_quotes) {
					columns.push_back(cell);
					cell.clear();
				}
				else {
					cell += c;
				}
			}
			columns.push_back(cell);
			return columns;
		}
	}

	std::istream& operator>>(std::istream& in, Player& player)
	{
		if (!player.m_player) {
			player.m_player = std::make_unique<Models::Player>();
		}
		else {
			// Clear existing player data
			player.m_player = std::make_unique<Models::Player>();
		}

		std::string line;
		while (std::getline(in, line)) {
			if (line.empty()) continue;

			auto columns = splitCsvLine(line);
			if (columns.size() < 2 || columns[0] != "Player") {
				// Not a player line, so put it back and stop reading for this player
				in.seekg(-static_cast<std::streamoff>(line.length()) - 1, std::ios_base::cur);
				break;
			}

			const std::string& type = columns[1];
			if (type == "Username" && columns.size() > 2) {
				player.m_player->setPlayerUsername(columns[2]);
			}
			else if (type == "Coins" && columns.size() > 2) {
				player.addCoins(static_cast<uint8_t>(std::stoi(columns[2])));
			}
			else if (type == "Card" && columns.size() > 3) {
				std::vector<std::string> card_cols(columns.begin() + 2, columns.end());
				auto card = std::make_unique<Models::AgeCard>(ageCardFactory(card_cols));
				player.m_player->addCard(std::move(card));
			}
			else if (type == "Wonder" && columns.size() > 3) {
				std::vector<std::string> wonder_cols(columns.begin() + 2, columns.end());
				auto wonder = std::make_unique<Models::Wonder>(wonderFactory(wonder_cols));
				player.m_player->addWonder(wonder);
			}
			else if (type == "Token" && columns.size() > 2) {
				// This requires finding the token from a global list, which is not available here.
				// For now, we will skip loading tokens.
			}
			else if (type == "PermanentResource" && columns.size() > 2) {
				std::string res_str = columns[2];
				auto pos = res_str.find(':');
				if (pos != std::string::npos) {
					auto res_type = static_cast<Models::ResourceType>(std::stoi(res_str.substr(0, pos)));
					auto amount = static_cast<uint8_t>(std::stoi(res_str.substr(pos + 1)));
					player.m_player->addPermanentResource(res_type, amount);
				}
			}
			else if (type == "TradingResource" && columns.size() > 2) {
				std::string res_str = columns[2];
				auto pos = res_str.find(':');
				if (pos != std::string::npos) {
					auto res_type = static_cast<Models::ResourceType>(std::stoi(res_str.substr(0, pos)));
					auto amount = static_cast<uint8_t>(std::stoi(res_str.substr(pos + 1)));
					player.m_player->addTradingResource(res_type, amount);
				}
			}
			else if (type == "ScientificSymbol" && columns.size() > 2) {
				std::string sym_str = columns[2];
				auto pos = sym_str.find(':');
				if (pos != std::string::npos) {
					auto sym_type = static_cast<Models::ScientificSymbolType>(std::stoi(sym_str.substr(0, pos)));
					auto amount = static_cast<uint8_t>(std::stoi(sym_str.substr(pos + 1)));
					player.m_player->addScientificSymbol(sym_type, amount);
				}
			}
		}
		return in;
	}
}

// Minimal no-op implementations for methods referenced by CSV-defined lambdas
void Core::Player::setHasAnotherTurn(bool) { /* no-op for now */ }
void Core::Player::discardCard(Models::ColorType color)
{
	Core::Player* opponent = Core::getOpponentPlayer();
	if (!opponent) return;

	auto& owned = opponent->m_player->getOwnedCards();
	std::vector<size_t> candidates;
	for (size_t i = 0; i < owned.size(); ++i) {
		if (!owned[i]) continue;
		if (color == Models::ColorType::NO_COLOR || owned[i]->getColor() == color) {
			candidates.push_back(i);
		}
	}
	if (candidates.empty()) {
		std::cout << "No opponent cards of the specified color to discard.\n";
		return;
	}

	std::cout << "Choose opponent card to discard:\n";
	for (size_t idx = 0; idx < candidates.size(); ++idx) {
		size_t i = candidates[idx];
		std::cout << "[" << idx << "] " << owned[i]->getName() << "\n";
	}

	size_t choice = 0;
	if (!(std::cin >> choice) || choice >= candidates.size()) {
		if (!std::cin) { std::cin.clear(); std::string discard; std::getline(std::cin, discard); }
		choice = 0;
	}

	size_t removeIdx = candidates[choice];
	auto moved = opponent->m_player->removeOwnedCardAt(removeIdx);
	if (!moved) return;

	auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(Core::Board::getInstance().getDiscardedCards());
	discarded.push_back(std::move(moved));
}

void Core::Player::drawToken()
{
}

void Core::Player::takeNewCard()
{
}


