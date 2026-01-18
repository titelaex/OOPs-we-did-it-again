#include <compare>
module Core.Player;
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
import Core.GameState;
import Core.Board;
import Core.Game;
import Models.AgeCard;
import Models.GuildCard;
import Core.CardCsvParser;
import Core.IGameListener;
import Core.PlayerDecisionMaker;

namespace Core {
namespace {
thread_local std::shared_ptr<Player> g_current_player = nullptr;
}

void setCurrentPlayer(std::shared_ptr<Player> p) { g_current_player = p; }
std::shared_ptr<Player> getCurrentPlayer() { return g_current_player; }

std::shared_ptr<Player> getOpponentPlayer()
{
	std::shared_ptr<Player> cp = getCurrentPlayer();
    if (!cp) return nullptr;
	auto& gs = GameState::getInstance();
	std::shared_ptr<Player> p1 = gs.GetPlayer1();
	std::shared_ptr<Player> p2 = gs.GetPlayer2();
    if (p1 == cp) return p2;
    if (p2 == cp) return p1;
    return nullptr;
}
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

void Core::playTurnForCurrentPlayer()
{
    auto cp = getCurrentPlayer();
    if (!cp) return;
    std::cout << "Playing an extra turn for player\n";
}
void Core::drawTokenForCurrentPlayer(std::optional<std::reference_wrapper<IPlayerDecisionMaker>> decisionMaker)
{
    auto cp = getCurrentPlayer();
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
	auto& notifier = GameState::getInstance().getEventNotifier();
	DisplayRequestEvent event;
	event.displayType = DisplayRequestEvent::Type::MESSAGE;
	event.context = "Choose a token:";
	notifier.notifyDisplayRequested(event);
	
	std::vector<size_t> tokenIndices;
	for (size_t i = 0; i < pickCount; ++i) {
		tokenIndices.push_back(i);
		event.context = "[" + std::to_string(i) + "] " + combined[i]->getName();
		notifier.notifyDisplayRequested(event);
	}
	
	size_t choice = 0;
	if (decisionMaker.has_value()) {
		choice = decisionMaker->get().selectProgressToken(tokenIndices);
	}
	
	if (choice >= pickCount) choice = 0;
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
void Core::discardOpponentCardOfColor(Models::ColorType color, std::optional<std::reference_wrapper<IPlayerDecisionMaker>> decisionMaker)
{
	auto opponent = Core::getOpponentPlayer();
	if (!opponent || !decisionMaker.has_value()) return;

    auto cp = getCurrentPlayer();
    if (!cp) return;
	
	Game::handleOpponentCardDiscard(*opponent, *cp, color, decisionMaker->get());
    }
void Core::Player::chooseWonder(std::vector<std::unique_ptr<Models::Wonder>>& availableWonders, uint8_t chosenIndex)
{
    if (availableWonders.empty())
    {
		std::cout << "No available wonders to choose from.\n";
        return;
    }
	if (availableWonders.size() == 1)
    {
		std::string wonderName = availableWonders[0]->getName();
        m_player->addWonder(std::move(availableWonders[0]));
        availableWonders.erase(availableWonders.begin());
		std::cout << "Automatically chosen (last wonder): " << wonderName << "\n\n";
        return;
	}
    if (chosenIndex >= availableWonders.size())
    {
		std::cout << "ERROR: Invalid wonder index " << static_cast<int>(chosenIndex) 
		          << " (available: 0-" << (availableWonders.size() - 1) << ")\n";
        return;
    }
	std::string wonderName = availableWonders[chosenIndex]->getName();
	m_player->addWonder(std::move(availableWonders[chosenIndex]));
    availableWonders.erase(availableWonders.begin() + chosenIndex);
	std::cout << "Chosen wonder: " << wonderName << "\n\n";
}

void Core::Player::sellCard(std::unique_ptr<Models::Card>& ageCard, std::vector<std::unique_ptr<Models::Card>>& discardedCards)
{
    uint8_t coinsToGain = 2;
	uint8_t yellowCardCount = countYellowCards();
	coinsToGain += yellowCardCount;
    addCoins(coinsToGain);

    std::cout << "Player sold \"" << ageCard->getName() << "\" for "
        << static_cast<int>(coinsToGain) << " coins.\n";

	Core::Game::getNotifier().notifyCardSold({
		m_player->getkPlayerId(),                       
		m_player->getPlayerUsername(),                  
		ageCard->getName(),                             
		-1,                                             
		Models::ColorTypeToString(ageCard->getColor()), 
		});

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
void Core::Player::playCardWonder(std::unique_ptr<Models::Wonder>& wonder, std::unique_ptr<Models::Card>& ageCard, std::unique_ptr<Models::Player>& opponent,
    std::vector<Models::Token>& discardedTokens, std::vector<std::unique_ptr<Models::Card>>& discardedCards)
{
    if (Models::Wonder::getWondersBuilt() >= Models::Wonder::MaxWonders)
    {
		std::cout << "All wonders have been built in the game\n";
        return;
    }

    if (wonder->IsConstructed())
    {
		std::cout << "Wonder \"" << wonder->getName() << "\" is already constructed\n";
        return;
    }

    if (!canAffordWonder(wonder, opponent))
    {
		std::cout << "Cannot afford to construct wonder \"" << wonder->getName() << "\n";
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
		std::cout << "All wonders have been built in the game\n";
        discardRemainingWonder(opponent);
    }
	if (wonder->getResourceProduction() != Models::ResourceType::NO_RESOURCE) {
		m_player->addPermanentResource(wonder->getResourceProduction(), 1);
		std::cout << "  Added 1x " << Models::ResourceTypeToString(wonder->getResourceProduction()) << " to permanent resources\n";
	}
	wonder->attachUnderCard(std::move(ageCard));
	if (wonder->getShieldPoints() > 0)
	{
		Game::movePawn(static_cast<int>(wonder->getShieldPoints()));
		Core::Player oppWrapper;
		oppWrapper.m_player = std::move(opponent);
		Game::awardMilitaryTokenIfPresent(*this, oppWrapper);
		opponent = std::move(oppWrapper.m_player);
	}
	std::cout << "Wonder \"" << wonder->getName() << "\" constructed successfully\n";
	bool hasTheologyToken = m_player->hasToken(Models::TokenIndex::THEOLOGY);
	if (hasTheologyToken) {
		auto& notifier = GameState::getInstance().getEventNotifier();
		DisplayRequestEvent event;
		event.displayType = DisplayRequestEvent::Type::MESSAGE;
		event.context = "Theology Token: Wonder grants an extra Play Again effect!";
		notifier.notifyDisplayRequested(event);
	}
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
	bool hasArchitectureToken = m_player->hasToken(Models::TokenIndex::ARCHITECTURE);
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
	bool hasArchitectureToken = m_player->hasToken(Models::TokenIndex::ARCHITECTURE);
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

	//verify if we need to check opponent production at all
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
		std::cout << "  Total coins to pay for trading: " << static_cast<int>(totalTradingCost) << " coins\n";
        subtractCoins(totalTradingCost);
		std::cout << "  Coins after payment: " << static_cast<int>(m_player->totalCoins(m_player->getRemainingCoins())) << " coins\n";
    }
    else {
		std::cout << "  No trading needed - player has all required resources (or covered by Architecture token)\n";
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
	if (!card)
    {
		std::cout << "Card is null\n";
        return;
    }

	if (!opponent)
	{
		std::cout << "ERROR: Opponent is null in playCardBuilding\n";
		return;
	}
	
	if (!card->isAvailable())
	{
		std::cout << "Card \"" << card->getName() << "\" is not available for building\n";
		return;
	}
    
    if (card->getRequiresLinkingSymbol() != Models::LinkingSymbolType::NO_SYMBOL)
    {
        for (const auto& ownedCard : m_player->getOwnedCards())
        {
            if (ownedCard->getHasLinkingSymbol() == card->getRequiresLinkingSymbol())
            {
                card->setIsVisible(false);
				auto& notifier = GameState::getInstance().getEventNotifier();
				DisplayRequestEvent event;
				event.displayType = DisplayRequestEvent::Type::MESSAGE;
				event.context = "Card \"" + std::string(card->getName()) + "\" constructed for free via chain->";
				notifier.notifyDisplayRequested(event);
                applyCardEffects(card);
				if (const auto* ageCard = dynamic_cast<const Models::AgeCard*>(card.get())) {
					const auto& resourceProduction = ageCard->getResourcesProduction();
					for (const auto& [resource, amount] : resourceProduction) {
						m_player->addPermanentResource(resource, amount);
						event.context = "  Added " + std::to_string(static_cast<int>(amount)) + "x " + Models::ResourceTypeToString(resource) + " to permanent resources";
						notifier.notifyDisplayRequested(event);
					}
				}
				bool hasUrbanismToken = m_player->hasToken(Models::TokenIndex::URBANISM);
				if (hasUrbanismToken) {
					addCoins(4);
				}
				m_player->addCard(std::move(card));
                return;
            }
        }
    }
    

    if (card->getResourceCost().empty())
    {
        card->setIsVisible(false);
        std::cout << "Card \"" << card->getName() << "\" constructed for free->\n";
        applyCardEffects(card);
		if (const auto* ageCard = dynamic_cast<const Models::AgeCard*>(card.get())) {
			const auto& resourceProduction = ageCard->getResourcesProduction();
			for (const auto& [resource, amount] : resourceProduction) {
				m_player->addPermanentResource(resource, amount);
				std::cout << "  Added " << static_cast<int>(amount) << "x " << Models::ResourceTypeToString(resource) << " to permanent resources\n";
			}
		}
		m_player->addCard(std::move(card));
        return;
    }
	if (!canAffordCard(*card, opponent))
    {
        std::cout << "Cannot afford to construct \"" << card->getName() << "\"->\n";
        return;
    }
    payForCard(card, opponent);
    card->setIsVisible(false);
    std::cout << "Card \"" << card->getName() << "\" constructed->\n";
    applyCardEffects(card);
	if (const auto* ageCard = dynamic_cast<const Models::AgeCard*>(card.get())) {
		const auto& resourceProduction = ageCard->getResourcesProduction();
		for (const auto& [resource, amount] : resourceProduction) {
			m_player->addPermanentResource(resource, amount);
			std::cout << "  Added " << static_cast<int>(amount) << "x " << Models::ResourceTypeToString(resource) << " to permanent resources\n";
		}
	}
	m_player->addCard(std::move(card));
}

bool Core::Player::canAffordCard(const Models::Card& card, std::unique_ptr<Models::Player>& opponent)
{
    if (!opponent)
        return false;
    
    if (card.getResourceCost().empty())
        return true;
    if (card.getRequiresLinkingSymbol() != Models::LinkingSymbolType::NO_SYMBOL)
    {
        for (const auto& ownedCard : m_player->getOwnedCards())
        {
            if (ownedCard->getHasLinkingSymbol() == card.getRequiresLinkingSymbol())
                return true;
        }
    }
    const auto& cost = card.getResourceCost();
    const auto& ownPermanent = m_player->getOwnedPermanentResources();
    const auto& ownTrading = m_player->getOwnedTradingResources();
    const auto& opponentProduction = opponent->getOwnedPermanentResources();
    uint8_t availableCoins = m_player->totalCoins(m_player->getRemainingCoins());
    
    bool hasMasonryToken = m_player->hasToken(Models::TokenIndex::MASONRY);
    if (hasMasonryToken && card.getColor() == Models::ColorType::BLUE) {
        availableCoins += 2;
    }

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
	if (!card || !opponent) {
		std::cout << "ERROR: Card or opponent is null in payForCard\n";
		return;
	}

    const auto& cost = card->getResourceCost();
	const auto& ownPermanent = m_player->getOwnedPermanentResources();
    const auto& ownTrading = m_player->getOwnedTradingResources();
    const auto& opponentPermanent = opponent->getOwnedPermanentResources();
    uint8_t totalCoinsToPay = 0;
	
	if (const auto* ageCard = dynamic_cast<const Models::AgeCard*>(card.get())) {
		uint8_t cardCoinCost = ageCard->getCoinCost();
		if (cardCoinCost > 0) {
			totalCoinsToPay += cardCoinCost;
			std::cout << "  Card maintenance cost: " << static_cast<int>(cardCoinCost) << " coins\n";
		}
	}

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
		uint8_t opponentAmount = (opponentPermanent.find(resource) != opponentPermanent.end()) ? opponentPermanent.at(resource) : 0;
		uint8_t costPerUnit = 2 + opponentAmount;
        totalCoinsToPay += costPerUnit * missing;
		std::cout << "  Buying " << static_cast<int>(missing) << "x " << Models::ResourceTypeToString(resource) 
		          << " for " << static_cast<int>(costPerUnit) << " coins each (total: " << static_cast<int>(costPerUnit * missing) << " coins)\n";
    }

	bool hasMasonryToken = m_player->hasToken(Models::TokenIndex::MASONRY);
	if (hasMasonryToken && card->getColor() == Models::ColorType::BLUE) {
		if (totalCoinsToPay >= 2) {
			totalCoinsToPay -= 2;
		}
	}
	
	if (totalCoinsToPay > 0) {
		std::cout << "  Total coins to pay for trading: " << static_cast<int>(totalCoinsToPay) << " coins\n";
		subtractCoins(totalCoinsToPay);
		std::cout << "  Coins after payment: " << static_cast<int>(m_player->totalCoins(m_player->getRemainingCoins())) << " coins\n";
	} else {
		std::cout << "  No trading needed - player has all required resources\n";
	}
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
            auto cp = getCurrentPlayer();
			(void)cp;
			if (act.first) act.first();
			}, act.second);
    }

    auto newCard = std::make_unique<Models::Card>(builder.build());

    std::cout << "Player takes card: " << newCard->getName() << "\n";
    {
        auto cp = getCurrentPlayer();
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
	void chooseTokenByIndex(std::vector<std::unique_ptr<Models::Token>>& tokens, std::optional<std::reference_wrapper<IPlayerDecisionMaker>> decisionMaker)
    {
        auto cp = getCurrentPlayer();
        if (!cp) return;

        if (tokens.empty()) return;
		auto& notifier = GameState::getInstance().getEventNotifier();
		DisplayRequestEvent event;
		event.displayType = DisplayRequestEvent::Type::MESSAGE;
		event.context = "Choose a progress token by index:";
		notifier.notifyDisplayRequested(event);

		std::vector<size_t> tokenIndices;
        for (size_t i = 0; i < tokens.size(); ++i) {
			if (tokens[i]) {
				tokenIndices.push_back(i);
				event.context = "[" + std::to_string(i) + "] " + tokens[i]->getName();
				notifier.notifyDisplayRequested(event);
			}
        }

        size_t idx = 0;
		if (decisionMaker.has_value()) {
			idx = decisionMaker->get().selectProgressToken(tokenIndices);
        }

		if (idx >= tokens.size()) idx = 0;
        std::unique_ptr<Models::Token> taken = std::move(tokens[idx]);
        tokens.erase(tokens.begin() + idx);
        if (taken) cp->m_player->addToken(std::move(taken));
    }
	void chooseTokenByName(std::vector<std::unique_ptr<Models::Token>>& tokens, std::optional<std::reference_wrapper<IPlayerDecisionMaker>> decisionMaker)
	{
		auto cp = getCurrentPlayer();
		if (!cp) return;
		if (tokens.empty()) return;
		auto& notifier = GameState::getInstance().getEventNotifier();
		DisplayRequestEvent event;
		event.displayType = DisplayRequestEvent::Type::MESSAGE;
		event.context = "Choose a progress token by name:";
		notifier.notifyDisplayRequested(event);
		
		std::vector<size_t> tokenIndices;
		for (size_t i = 0; i < tokens.size(); ++i) {
			if (tokens[i]) {
				tokenIndices.push_back(i);
				event.context = "[" + std::to_string(i) + "] " + tokens[i]->getName();
				notifier.notifyDisplayRequested(event);
			}
		}
		
		size_t idx = 0;
		if (decisionMaker.has_value()) {
			idx = decisionMaker->get().selectProgressToken(tokenIndices);
		}
		
		if (idx >= tokenIndices.size()) idx = 0;
		size_t selectedIdx = tokenIndices[idx];
		
		std::unique_ptr<Models::Token> taken = std::move(tokens[selectedIdx]);
		tokens.erase(tokens.begin() + selectedIdx);
		if (taken) cp->m_player->addToken(std::move(taken));
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
					if (ch == '"') out += "`\""; else out.push_back(ch);
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
			player.m_player = std::make_unique<Models::Player>();
		}
		std::string line;
		while (std::getline(in, line)) {
			if (line.empty()) continue;
			auto columns = splitCsvLine(line);
			if (columns.size() < 2 || columns[0] != "Player") {
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
				player.m_player->addWonder(std::move(wonder));
			}
			else if (type == "Token" && columns.size() > 2) {
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
void Core::Player::setHasAnotherTurn(bool hasAnotherTurn)
{
	if (!hasAnotherTurn) return;
	auto cp = getCurrentPlayer();
	if (!cp) return;
	std::cout << "Player gets an extra turn!\n";
	Core::playTurnForCurrentPlayer();
}
void Core::Player::discardCard(Models::ColorType color, std::optional<std::reference_wrapper<IPlayerDecisionMaker>> decisionMaker)
{
	auto opponent = Core::getOpponentPlayer();
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
		auto& notifier = GameState::getInstance().getEventNotifier();
		DisplayRequestEvent event;
		event.displayType = DisplayRequestEvent::Type::MESSAGE;
		event.context = "No opponent cards of the specified color to discard.";
		notifier.notifyDisplayRequested(event);
		return;
	}
	auto& notifier = GameState::getInstance().getEventNotifier();
	DisplayRequestEvent event;
	event.displayType = DisplayRequestEvent::Type::MESSAGE;
	event.context = "Choose opponent card to discard:";
	notifier.notifyDisplayRequested(event);
	
	for (size_t idx = 0; idx < candidates.size(); ++idx) {
		size_t i = candidates[idx];
		event.context = "[" + std::to_string(idx) + "] " + owned[i]->getName();
		notifier.notifyDisplayRequested(event);
	}
	
	size_t choice = 0;
	if (decisionMaker.has_value()) {
		choice = decisionMaker->get().selectCardToDiscard(candidates);
	}
	
	if (choice >= candidates.size()) choice = 0;
	size_t removeIdx = candidates[choice];
	auto moved = opponent->m_player->removeOwnedCardAt(removeIdx);
	if (!moved) return;
	auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(Core::Board::getInstance().getDiscardedCards());
	discarded.push_back(std::move(moved));
}
void Core::Player::drawToken(std::optional<std::reference_wrapper<IPlayerDecisionMaker>> decisionMaker)
{
	if (decisionMaker.has_value()) {
		Core::drawTokenForCurrentPlayer(decisionMaker);
	} else {
		Core::drawTokenForCurrentPlayer(std::nullopt);
	}
}
void Core::Player::chooseProgressTokenFromBoard(std::optional<std::reference_wrapper<IPlayerDecisionMaker>> decisionMaker)
{
	auto& board = Core::Board::getInstance();
	auto& availableTokens = const_cast<std::vector<std::unique_ptr<Models::Token>>&>(board.getProgressTokens());
	auto& notifier = GameState::getInstance().getEventNotifier();

	if (availableTokens.empty()) {
		DisplayRequestEvent event;
		event.displayType = DisplayRequestEvent::Type::MESSAGE;
		event.context = "No progress tokens available on the board!";
		notifier.notifyDisplayRequested(event);
		return;
	}

	DisplayRequestEvent pairEvent;
	pairEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
	pairEvent.context = "\nCONGRATULATIONS! You formed a pair of scientific symbols!";
	notifier.notifyDisplayRequested(pairEvent);
	
	pairEvent.context = "Choose a progress token from the board:";
	notifier.notifyDisplayRequested(pairEvent);

	std::vector<size_t> tokenIndices;
	for (size_t i = 0; i < availableTokens.size(); ++i) {
		if (availableTokens[i]) {
			tokenIndices.push_back(i);
			DisplayRequestEvent tokenEvent;
			tokenEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
			tokenEvent.context = "[" + std::to_string(i) + "] " + availableTokens[i]->getName();
			notifier.notifyDisplayRequested(tokenEvent);
		}
	}

	size_t choice = 0;
	if (decisionMaker.has_value()) {
		choice = decisionMaker->get().selectProgressToken(tokenIndices);
	} else {
		bool valid = false;
		while (!valid) {
			std::cout << "Your choice (0-" << availableTokens.size() - 1 << "): ";
			if (std::cin >> choice && choice < availableTokens.size()) {
				valid = true;
			}
			else {
				std::cin.clear();
				std::string dummy;
				std::getline(std::cin, dummy);
				std::cout << "Invalid input.\n";
			}
		}
	}

	auto chosenToken = std::move(availableTokens[choice]);
	availableTokens.erase(availableTokens.begin() + choice);
	if (chosenToken) {
		std::string tokenName = chosenToken->getName();
		std::string tokenDesc = chosenToken->getDescription();
		
		DisplayRequestEvent choiceEvent;
		choiceEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
		choiceEvent.context = "You chose the token: " + tokenName + "\n";
		notifier.notifyDisplayRequested(choiceEvent);
		
		m_player->addToken(std::move(chosenToken));
		
		Core::TokenEvent tokenEvent;
		tokenEvent.playerID = static_cast<int>(m_player->getkPlayerId());
		tokenEvent.playerName = m_player->getPlayerUsername();
		tokenEvent.tokenName = tokenName;
		tokenEvent.tokenType = "PROGRESS";
		tokenEvent.tokenDescription = tokenDesc;
		Core::Game::getNotifier().notifyTokenAcquired(tokenEvent);
	}
}
void Core::Player::takeNewCard(std::optional<std::reference_wrapper<IPlayerDecisionMaker>> decisionMaker)
{
	auto cp = getCurrentPlayer();
	if (!cp) return;
	auto& board = Core::Board::getInstance();
	auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
	auto& notifier = GameState::getInstance().getEventNotifier();
	
	if (discarded.empty()) {
		DisplayRequestEvent event;
		event.displayType = DisplayRequestEvent::Type::MESSAGE;
		event.context = "No cards in the discard pile.";
		notifier.notifyDisplayRequested(event);
		return;
	}

	DisplayRequestEvent event;
	event.displayType = DisplayRequestEvent::Type::MESSAGE;
	event.context = "Choose a card from the discard pile:";
	notifier.notifyDisplayRequested(event);

	std::vector<size_t> cardIndices;
	for (size_t i = 0; i < discarded.size(); ++i) {
		if (discarded[i]) {
			cardIndices.push_back(i);
			event.context = "[" + std::to_string(i) + "] " + discarded[i]->getName();
			notifier.notifyDisplayRequested(event);
		}
	}

	size_t choice = 0;
	if (decisionMaker.has_value()) {
		// If an MCTS-based AI is playing, pick from discard using weight-driven scoring
		// (so WeightOptimizer affects this decision too).
		if (auto* mctsDM = dynamic_cast<MCTSDecisionMaker*>(&decisionMaker->get())) {
			AIConfig cfg(mctsDM->getPlaystyle());
			const auto w = cfg.getWeights();

			auto scoreCard = [&](const Models::Card* card) -> double {
				if (!card) return -1e9;
				double score = 0.0;
				score += static_cast<double>(card->getVictoryPoints()) * w.victoryPointValue;

				// Colors roughly map to existing weights.
				switch (card->getColor()) {
				case Models::ColorType::BROWN:
				case Models::ColorType::GREY:
					score += 1.0 * w.resourceValue;
					break;
				case Models::ColorType::BLUE:
					score += 1.0 * w.victoryPointValue;
					break;
				case Models::ColorType::GREEN:
					score += 1.0 * w.sciencePriority;
					break;
				case Models::ColorType::RED:
					score += 1.0 * w.militaryPriority;
					break;
				case Models::ColorType::YELLOW:
					score += 1.0 * w.economyPriority;
					break;
				default:
					break;
				}

				if (auto* ageCard = dynamic_cast<const Models::AgeCard*>(card)) {
					score += static_cast<double>(ageCard->getShieldPoints()) * w.militaryPriority;
					if (ageCard->getScientificSymbols().has_value()) score += 1.5 * w.sciencePriority;
					for (const auto& [res, amt] : ageCard->getResourcesProduction()) {
						(void)res;
						score += static_cast<double>(amt) * w.resourceValue;
					}
				}

				return score;
			};

			double bestScore = -1e18;
			for (size_t pos = 0; pos < cardIndices.size(); ++pos) {
				size_t idx = cardIndices[pos];
				if (idx >= discarded.size() || !discarded[idx]) continue;
				double s = scoreCard(discarded[idx].get());
				if (s > bestScore) {
					bestScore = s;
					choice = pos;
				}
			}
		}
		else {
			choice = decisionMaker->get().selectCard(cardIndices);
		}
	}
	
	if (choice >= cardIndices.size()) choice = 0;
	size_t selectedIdx = cardIndices[choice];

	if (!discarded[selectedIdx]) {
		event.context = "Invalid card selection.";
		notifier.notifyDisplayRequested(event);
		return;
	}

	auto card = std::move(discarded[selectedIdx]);
	discarded.erase(discarded.begin() + selectedIdx);
	if (!card) return;

	cp->m_player->addCard(std::move(card));
	cp->m_player->getOwnedCards().back()->onPlay();
	
	event.context = "Card \"" + std::string(cp->m_player->getOwnedCards().back()->getName()) + "\" constructed for free.";
	notifier.notifyDisplayRequested(event);
}
