module Core.Player;

import <iostream>;
import <vector>;
import <tuple>;
import <unordered_map>;
import <memory>;
import <random>;
import <algorithm>;

import Models.Wonder;
import Models.Card;
import Models.Player;
import Models.Token;
import Models.LinkingSymbolType;
import GameState;
import Core.Board;
import Models.AgeCard;

static thread_local Core::Player* g_current_player = nullptr;

void SetCurrentPlayer(Core::Player* p) { g_current_player = p; }
Core::Player* GetCurrentPlayer() { return g_current_player; }

Core::Player* Core::GetOpponentPlayer()
{
    Core::Player* cp = GetCurrentPlayer();
    if (!cp) return nullptr;
    auto &gs = Core::GameState::getInstance();
    Player* p1 = gs.GetPlayer1();
    Player* p2 = gs.GetPlayer2();
    if (p1 == cp) return p2;
    if (p2 == cp) return p1;
    return nullptr;
}

void Core::playTurnForCurrentPlayer()
{
    Core::Player* cp = GetCurrentPlayer();
    if (!cp) return;
    std::cout << "Playing an extra turn for player\n";
}

void Core::drawTokenForCurrentPlayer()
{
    Core::Player* cp = GetCurrentPlayer();
    if (!cp) return;
    std::vector<Models::Token> combined;
    for (const auto &t : Core::progressTokens) combined.push_back(t);
    for (const auto &t : Core::militaryTokens) combined.push_back(t);
    if (combined.empty()) return;
    std::random_device rd; std::mt19937 gen(rd());
    std::shuffle(combined.begin(), combined.end(), gen);
    size_t pickCount = std::min<size_t>(3, combined.size());
    // show options
    std::cout << "Choose a token: \n";
    for (size_t i = 0; i < pickCount; ++i) std::cout << "[" << i << "] " << combined[i] << "\n";
    size_t choice = 0; std::cin >> choice; if (choice >= pickCount) choice = 0;
    // give token to player and remove it from board pool where found
    const auto &chosen = combined[choice];
    cp->m_player->addToken(chosen);
    // remove first occurrence from progress or military
    for (auto it = Core::progressTokens.begin(); it != Core::progressTokens.end(); ++it) {
        if (it->getName() == chosen.getName()) { Core::progressTokens.erase(it); return; }
    }
    for (auto it = Core::militaryTokens.begin(); it != Core::militaryTokens.end(); ++it) {
        if (it->getName() == chosen.getName()) { Core::militaryTokens.erase(it); return; }
    }
}

// Discard an opponent card of given color (grey or brown). Let current player choose index among opponent owned cards filtered by color.
void Core::discardOpponentCardOfColor(Models::ColorType color)
{
    Core::Player* cp = GetCurrentPlayer();
    if (!cp) return;
    Core::Player* opponent = GetOpponentPlayer();
    if (!opponent) return;
    auto &owned = opponent->m_player->getOwnedCards();
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
    if (moved) Core::discardedCards.push_back(std::move(moved));
}

// Build a chosen card from Core::unusedAgeOneCards (index into that vector)
void Core::buildCardFromAge1Pool(size_t index)
{
    Core::Player* cp = GetCurrentPlayer();
    if (!cp) return;
    if (index >= Core::unusedAgeOneCards.size()) return;
    // move card into buildDiscardedCard pool (as requested)
    auto cardPtr = std::move(Core::buildDiscardedCard[index]);
    if (!cardPtr) return;
    cp->m_player->addCard(cardPtr);
    Core::buildDiscardedCard.erase(Core::buildDiscardedCard.begin() + index);
}

void Core::Player::chooseWonder(std::vector<std::unique_ptr<Models::Wonder>>& availableWonders, uint8_t chosenIndex)
{
    if (availableWonders.empty())
    {
        std::cout << "No available wonders to choose from->\n";
        return;
    }

    if(availableWonders.size() == 1)
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

void Core::Player::sellCard()
{
    uint8_t yellowCards = countYellowCards();
    uint8_t coinsEarned = 2 + yellowCards;
	//addCoins(coinsEarned);

	/*
        auto it = std::find_if(m_player.getOwnedCards().begin(), m_player.getOwnedCards().end(), [&](const std::unique_ptr<Models::Card>& p) { return p && p.get() == &card; });
	if (it != m_player.getOwnedCards().end()) {
		(*it)->SetIsVisible(false);
		std::cout << "Card \"" << (*it)->GetName() << "\" discarded. Player \"" << m_player.getPlayerUsername() << "\" gains " << static_cast<int>(coinsEarned) << " coins.\n";
	//	m_player.getOwnedCards().erase(it);
	} else {
		card.SetIsVisible(false);
		std::cout << "Card \"" << card.GetName() << "\" discarded. Player \"" << m_player.getPlayerUsername() << "\" gains " << static_cast<int>(coinsEarned) << " coins.\n";
	}
    */
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
    // check if WondersBuilt < 7
    if (Models::Wonder::wondersBuilt < 7)
    {
        if (!wonder->IsConstructed())
        {
            if(canAffordWonder(wonder, opponent))
            {
                payForWonder(wonder, opponent);

                Models::Wonder* rawWonderPtr = wonder.release();
                std::unique_ptr<Models::Card> tempCard(rawWonderPtr);
                applyCardEffects(tempCard);
                rawWonderPtr = static_cast<Models::Wonder*>(tempCard.release());
                wonder.reset(rawWonderPtr);

                wonder->setConstructed(true);
                Models::Wonder::wondersBuilt++;
                if(Models::Wonder::wondersBuilt == 7)
                {
                    std::cout << "All wonders have been built in the game->\n";
                    discardRemainingWonder();
                }

				ageCard.reset(); // remove reference to the age card used to build the wonder
                /*
                discardedCards.push_back(std::move(ageCard));
                
                De ce e greșit: Tu arunci cartea în pachetul de discard (discardedCards).
                Acest lucru afectează minunea The Mausoleum, care permite jucătorului să ia o carte din "discard pile". 
                Dacă pui cărțile folosite la minuni acolo, vei permite Mausoleului să recupereze cărți care ar trebui să fie scoase definitiv din joc 
                (ascunse sub minuni).
                Corecție: Cartea trebuie stocată într-un vector intern al minunii sau pur și simplu "ștearsă" logic, dar NU adăugată la discardedCards.     
                */
            }
            else
            {
                std::cout << "Cannot afford to construct wonder \"" << wonder->getName() << "\"->\n";
                return;
		    }
        }
        else 
            std::cout << "Wonder \"" << wonder->getName() << "\" is already constructed->\n";
            return;
    }
    else
    {
        std::cout << "All wonders have been built in the game->\n";
        return;
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
        return true; // No resources needed, so it's affordable.
    }

    // 2. Check for Architecture token discount

    //apply the discount givem from the Architecture token if the player has it
    /*
    Architecture
        Any future Wonders built by you will cost 2 fewer resources.
        At each construction, you are free to choose which resources this rebate affects.
    */

    bool hasArchitectureToken = false;
    for (const auto& token : m_player->getOwnedTokens()) {
        if (token.getName() == "Architecture") {
            hasArchitectureToken = true;
            break;
        }
    }

    // Helper to check for commercial discounts using m_tradeRules
    auto getTradeDiscount = [&](Models::ResourceType resource) -> int {
        const auto& tradeRules = m_player->getTradeRules();
        Models::TradeRuleType ruleType;
        switch (resource) {
        case Models::ResourceType::WOOD:    ruleType = Models::TradeRuleType::WOOD; break;
        case Models::ResourceType::STONE:   ruleType = Models::TradeRuleType::STONE; break;
        case Models::ResourceType::CLAY:    ruleType = Models::TradeRuleType::CLAY; break;
        case Models::ResourceType::PAPYRUS: ruleType = Models::TradeRuleType::PAPYRUS; break;
        case Models::ResourceType::GLASS:   ruleType = Models::TradeRuleType::GLASS; break;
        default: return -1; // Not a resource that can have a trade rule
        }

        auto it = tradeRules.find(ruleType);
        if (it != tradeRules.end() && it->second) {
            return 1; // Cost is 1 coin
        }

        return -1; // No discount
        };

    // 1. Correctly calculate opponent's production from only brown and grey cards
    std::unordered_map<Models::ResourceType, uint8_t> opponentBrownGreyProduction;
    for (const auto& card : opponent->getOwnedCards())
    {
        if (card && (card->getColor() == Models::ColorType::BROWN || card->getColor() == Models::ColorType::GREY))
        {
            if (const auto* ageCard = dynamic_cast<const Models::AgeCard*>(card.get()))
            {
                for (const auto& resourcePair : ageCard->getResourceProduction())
                {
                    // Only count opponent's production if we don't have a trade discount for it
                    if (getTradeDiscount(resourcePair.first) == -1)
                    {
                        opponentBrownGreyProduction[resourcePair.first] += resourcePair.second;
                    }
                }
            }
        }
    }


    uint8_t totalCost = 0;

   // apply the discount from Architecture token in the most beneficial way -> eliminate the cost of the
   // resources the player doen't produce at all first and are the most expensive to buy from the bank
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
        if (token.getName() == "Architecture") {
            hasArchitectureToken = true;
            break;
        }
    }


    if (hasArchitectureToken && totalMissingUnits <= 2) {
        std::cout << "Player constructed the wonder for free (covered fully by Architecture token).\n";
        return;
    }


	// if we get there, that means we have to pay some coins for missing resources

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
        if (it != tradeRules.end() && it->second) return 1; // Cost fix 1
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
                    for (const auto& resourcePair : ageCard->getResourceProduction()) {
                        if (missingResources.find(resourcePair.first) != missingResources.end()) {
                            opponentBrownGreyProduction[resourcePair.first] += resourcePair.second;
                        }
                    }
                }
            }
        }
    }

	/// final calculation of total cost
    std::vector<uint8_t> purchaseCosts;

    for (const auto& [resource, amount] : missingResources) {
        int discountedCost = getTradeDiscount(resource);
        uint8_t costPerUnit = 0;

        if (discountedCost != -1) {
			// from yellow cards or trade rules -> 1 coin each
            costPerUnit = 1;
        }
        else {
			// from opponent production 
            uint8_t opponentAmount = opponentBrownGreyProduction.count(resource) ? opponentBrownGreyProduction.at(resource) : 0;
            costPerUnit = 2 + opponentAmount;
        }

        for (int i = 0; i < amount; ++i) {
            purchaseCosts.push_back(costPerUnit);
        }
    }

	/// apply Architecture token discount if available
	/// we already verified above that there are more than 2 missing units
    if (hasArchitectureToken) {
        std::sort(purchaseCosts.rbegin(), purchaseCosts.rend());
		/// eliminate the two most expensive resources
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
    else {
		/// for safety, should not happen
        std::cout << "Player constructed the wonder for free.\n";
    }
}

void Core::Player::discardRemainingWonder()
{
    uint8_t builtCount = 0;
    // placeholder - actual list of all wonders should be provided by caller or game state
    // this implementation keeps method for API compatibility
}



void Core::Player::playCardBuilding(std::unique_ptr<Models::Card>& card, std::unique_ptr<Models::Player>& opponent)
{
    if (!card->getIsVisible())
    {
        std::cout << "Card \"" << card->getName() << "\" is not accessible->\n";
        return;
    }

    // Chain construction logic requires specific card APIs; commented out for now
    /*
    if (card->GetRequiresLinkingSymbol() != Models::LinkingSymbolType::NO_SYMBOL)
    {
        for (const auto& ownedCard : m_player->getOwnedCards())
        {
            if (ownedCard->GetHasLinkingSymbol() == card->GetRequiresLinkingSymbol())
            {
                m_player->addCard(card);
                card->SetIsVisible(false);
                std::cout << "Card \"" << card->GetName() << "\" constructed for free via chain->\n";
                applyCardEffects(card);
                return;
            }
        }
    }
    */

    // Check if card is free (resource cost only checked; coin-cost logic omitted)
    //if (card.GetResourceCost()->empty() /* && coin-cost logic omitted */)
    //{
    //    m_player->addCard(card);
    //    card->SetIsVisible(false);
    //    std::cout << "Card \"" << card->GetName() << "\" constructed for free->\n";
    //    applyCardEffects(card);
    //    return;
    //}

    // Check affordability
    if (!canAffordCard(card, opponent))
    {
        std::cout << "Cannot afford to construct \"" << card->getName() << "\"->\n";
        return;
    }

    // Pay cost
    payForCard(card, opponent);

    // Construct card
    /*m_player->addCard(card);
    card->SetIsVisible(false);
    std::cout << "Card \"" << card->GetName() << "\" constructed->\n";
    applyCardEffects(card);*/
}

bool Core::Player::canAffordCard(std::unique_ptr<Models::Card>& card, std::unique_ptr<Models::Player>& opponent)
{
    // 1-> Free card (coin-cost omitted)
    //if (card->GetResourceCost()->empty() /* && coin-cost omitted */)
    //    return true;

    // 2-> Chain construction (commented out; requires card linking APIs)
    /*
    if (card->GetRequiresLinkingSymbol() != Models::LinkingSymbolType::NO_SYMBOL)
    {
        for (const auto& ownedCard : m_player->getOwnedCards())
        {
            if (ownedCard->GetHasLinkingSymbol() == card->GetRequiresLinkingSymbol())
                return true;
        }
    }
    */

    // 3-> Resource cost
    const auto& cost = card->getResourceCost();
    /*const auto& ownPermanent = m_player->getOwnedPermanentResources();
    const auto& ownTrading = m_player->getOwnedTradingResources();
    const auto& opponentProduction = opponent->getOwnedPermanentResources();

    uint8_t availableCoins = m_player->totalCoins(m_player->getRemainingCoins());

    for (const auto& kv : cost)
    {
        auto resource = kv->first;
        auto requiredAmount = kv->second;

        uint8_t produced = 0;
        if (ownPermanent->find(resource) != ownPermanent->end()) produced += ownPermanent->at(resource);
        if (ownTrading->find(resource) != ownTrading->end()) produced += ownTrading->at(resource);

        if (produced >= requiredAmount)
            continue;

        uint8_t missing = requiredAmount - produced;
        uint8_t opponentAmount = (opponentProduction->find(resource) != opponentProduction->end()) ? opponentProduction->at(resource) : 0;
        uint8_t costPerUnit = 2 + opponentAmount;

        uint8_t totalCost = costPerUnit * missing;
        if (availableCoins < totalCost)
            return false;
    }*/
    return true;
}

// Need to include payForCard, applyCardEffects, etc
void Core::Player::payForCard(std::unique_ptr<Models::Card>& card, std::unique_ptr<Models::Player>& opponent)
{
    const auto& cost = card->getResourceCost();
    /*const auto& ownPermanent = m_player->getOwnedPermanentResources();
    const auto& ownTrading = m_player->getOwnedTradingResources();
    const auto& opponentPermanent = opponent->getOwnedPermanentResources();

    uint8_t totalCoinsToPay = 0;

    for (const auto& kv : cost)
    {
        auto resource = kv->first;
        auto requiredAmount = kv->second;

        uint8_t produced = 0;
        if (ownPermanent->find(resource) != ownPermanent->end()) produced += ownPermanent->at(resource);
        if (ownTrading->find(resource) != ownTrading->end()) produced += ownTrading->at(resource);

        if (produced >= requiredAmount)
            continue;

        uint8_t missing = requiredAmount - produced;
        uint8_t opponentProduction = (opponentPermanent->find(resource) != opponentPermanent->end()) ? opponentPermanent->at(resource) : 0;
        uint8_t costPerUnit = 2 + opponentProduction;

        totalCoinsToPay += costPerUnit * missing;
    }

    m_player->subtractCoins(totalCoinsToPay);*/
}

void Core::Player::applyCardEffects(std::unique_ptr<Models::Card>& card)
{
    std::cout << "Applying effects of card \"" << card->getName() << "\"->\n";
}

void Core::Player::takeCard(std::unique_ptr<Models::Card> card)
{
    // Rebuild the card to bind its onPlay actions to this Core::Player
    const auto& oldActions = card->getOnPlayActions();
    Models::CardBuilder builder;
    builder.setName(card->getName())
        .setResourceCost(card->getResourceCost())
        .setVictoryPoints(card->getVictoryPoints())
        .setCoinWorth(card->getCoinWorth())
        .setCoinReward(card->getCoinReward())
        .setCaption(card->getCaption())
        .setColor(card->getColor());

    for (const auto& act : oldActions) {
        // wrap original action so it always runs with this player as current context
        builder.addOnPlayAction([this, act]() {
            Core::scopedPlayerContext ctx(this);
            if (act) act();
        });
    }

    auto newCard = std::make_unique<Models::Card>(builder.build());

    std::cout << "Player takes card: " << newCard->getName() << "\n";
    // play the card immediately in the context of this player
    {
        Core::scopedPlayerContext ctx(this);
        newCard->onPlay();
    }
    m_player->addCard(std::move(newCard));
}

void Core::Player::addCoins(uint8_t amt)
{
    auto coins = m_player->getkPlayerIdgetRemainingCoins();
    uint32_t total = m_player->totalCoins(coins) + amt;
    uint8_t sixes = static_cast<uint8_t>(total / 6u);
    uint8_t rem = static_cast<uint8_t>(total % 6u);
    uint8_t threes = static_cast<uint8_t>(rem / 3u);
    uint8_t ones = static_cast<uint8_t>(rem % 3u);
    m_player->setRemainingCoins({ones, threes, sixes});
}

void Core::Player::subtractCoins(uint8_t amt)
{
    auto coins = m_player->getRemainingCoins();
    uint32_t total = m_player->totalCoins(coins);
    if (amt >= total) { m_player->setRemainingCoins({0,0,0}); return; }
    uint32_t newTotal = total - amt;
    uint8_t sixes = static_cast<uint8_t>(newTotal / 6u);
    uint8_t rem = static_cast<uint8_t>(newTotal % 6u);
    uint8_t threes = static_cast<uint8_t>(rem / 3u);
    uint8_t ones = static_cast<uint8_t>(rem % 3u);
    m_player->setRemainingCoins({ones, threes, sixes});
}

