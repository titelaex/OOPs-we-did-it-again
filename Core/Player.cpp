module Core.Player;

import <iostream>;
import <vector>;
import <tuple>;

import Models.Wonder;
import Models.Card;
import Models.Player;
import Models.Token;
import Models.LinkingSymbolType;


//de refacut functia asta
void Core::Player::playCardWonder(std::unique_ptr<Models::Wonder>& wonder, std::unique_ptr<Models::Card>& ageCard, std::unique_ptr<Models::Player>& opponent,
    std::vector<Models::Token>& discardedTokens, std::vector<std::unique_ptr<Models::Card>>& discardedCards,
    uint8_t& totalWondersBuilt)
{
    // check if WondersBuilt < 7

    // 1-> Check if Wonder is already built
    if (wonder->GetIsVisible())
    {
        std::cout << "Wonder \"" << wonder->GetName() << "\" is already constructed->\n";
        return;
    }

    // 2-> Check affordability 
    if (!canAffordWonder(wonder, opponent))
    {
        std::cout << "Cannot afford Wonder \"" << wonder->GetName() << "\"->\n";
        return;
    }

    // 3-> Pay cost (resources + coins)
    payForWonder(wonder, opponent);

    // 4-> Use Age card to build Wonder
    ageCard->toggleVisibility(); // mark as discarded

    // 5-> Trigger Wonder effects
    if (wonder->GetCoinReward() > 0)
    {
        // placeholder: give coins to player
    }

    if (wonder->getOpponentLosesMoney() > 0)
    {
        // placeholder: opponent loses money
    }

    if (wonder->getPlaySecondTurn())
    {
        // placeholder: schedule second turn
    }

    if (wonder->getDrawProgressTokens())
    {
        // placeholder: draw progress tokens
    }

    if (wonder->getChooseAndConstructBuilding())
    {
        // placeholder: allow choose and construct building
    }

    if (wonder->getDiscardCardFromOpponent())
    {
        // placeholder: discard an opponent's card
    }

    // 6-> Add Wonder to player's built list
    //m_player->addWonder(wonder);
    wonder->SetIsVisible(true); // mark as constructed

    // 7-> Check if 7 Wonders have been built
    ++totalWondersBuilt;
    if (totalWondersBuilt == 7)
    {
        discardRemainingWonder();
    }

    std::cout << "Wonder \"" << wonder->GetName() << "\" constructed successfully->\n";
}

void Core::Player::playCardBuilding(std::unique_ptr<Models::Card>& card, std::unique_ptr<Models::Player>& opponent)
{
    if (!card->GetIsVisible())
    {
        std::cout << "Card \"" << card->GetName() << "\" is not accessible->\n";
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
    //if (card.GetResourceCost()->empty() /* && coin-cost omitted */)
    //{
    //    m_player->addCard(card);
    //    card->SetIsVisible(false);
    //    std::cout << "Card \"" << card->GetName() << "\" constructed for free->\n";
    //    applyCardEffects(card);
    //    return;
    //}


   /* void Core::Player::sellCard()
    {
       
    }*/

  /*  uint8_t countYellowCards() const
{
	uint8_t count = 0;
	for (const auto& p : m_player->getOwnedCards()) {
		if (!p) continue;
		if (p->GetColor() == ColorType::YELLOW) ++count;
	}
	return count;
}*/

    // Check affordability
    if (!canAffordCard(card, opponent))
    {
        std::cout << "Cannot afford to construct \"" << card->GetName() << "\"->\n";
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

bool Core::Player::canAffordWonder(std::unique_ptr<Models::Wonder>& wonder, std::unique_ptr<Models::Player>& opponent)
{
    const auto& cost = wonder->GetResourceCost();
    /*const auto& ownPermanent = m_player->getOwnedPermanentResources();
    const auto& ownTrading = m_player->getOwnedTradingResources();
    const auto& opponentPermanent = opponent->getOwnedPermanentResources();

    uint8_t totalAvailableCoins = m_player->totalCoins(m_player->getRemainingCoins());

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
        uint8_t totalCost = costPerUnit * missing;

        if (totalAvailableCoins < totalCost)
            return false;

        totalAvailableCoins -= totalCost;
    }*/
    return true;
}

void Core::Player::payForWonder(std::unique_ptr<Models::Wonder>& wonder, std::unique_ptr<Models::Player>& opponent)
{
    const auto& cost = wonder->GetResourceCost();
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

void Core::Player::discardRemainingWonder()
{
    uint8_t builtCount = 0;
    // placeholder - actual list of all wonders should be provided by caller or game state
    // this implementation keeps method for API compatibility
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
    const auto& cost = card->GetResourceCost();
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
    const auto& cost = card->GetResourceCost();
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
    std::cout << "Applying effects of card \"" << card->GetName() << "\"->\n";
}


