module Models.GuildCard;
import <iostream>;

using namespace Models;

GuildCard::GuildCard()
{
}

GuildCard::GuildCard(GuildCard&& other) noexcept
	: Card(std::move(other)) {}
GuildCard& GuildCard::operator=(GuildCard&& other) noexcept { if (this != &other) { Card::operator=(std::move(other));  } return *this; }

void GuildCard::displayCardInfo() {
	Card::displayCardInfo();
	std::cout << "Guild Name: " << Card::GetName() << "\n";
	auto type = Card::GetCoinWorth();
	switch (type)
	{
		case CoinWorthType::WONDER:
			std::cout << " 2 victory points for each Wonder constructed in the city which has the most wonders.";
			break;
		case CoinWorthType::BLUE:
			std::cout << "  1 coin/blue card in the city which has the most blue cards at that time + 1v_point/blue card ath the end of the game";
			break;
		case CoinWorthType::GREYBROWN:
			std::cout << "1 coin/(gray+brown in the city which has the most at that time)+ 1v_point/(same thing at the end)";
			break;
		case CoinWorthType::RED:
			std::cout << "1 coin/(red in the city which has the most at that time)+ 1v_point/(same thing at the end)";
			break;
		case CoinWorthType::YELLOW:
			std::cout << "1 coin/(yellow in the city which has the most at that time)+ 1v_point/(same thing at the end)";
			break;
		case CoinWorthType::GREEN:
			std::cout << "1 coin/(green in the city which has the most at that time)+ 1v_point/(same thing at the end)";
			break;
		default:
			std::cout << "Tip neidentificat";
	}
}

void GuildCard::onDiscard()
{
    Card::onDiscard();
}

GuildCardBuilder& GuildCardBuilder::setName(const std::string& name) { m_card.setName(name); return *this; }
GuildCardBuilder& GuildCardBuilder::setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost) { m_card.setResourceCost(resourceCost); return *this; }
GuildCardBuilder& GuildCardBuilder::setVictoryPoints(const uint8_t& victoryPoints) { m_card.setVictoryPoints(victoryPoints); return *this; }
GuildCardBuilder& GuildCardBuilder::setCoinWorth(const CoinWorthType& coinWorth) { m_card.setCoinWorth(coinWorth); return *this; }
GuildCardBuilder& GuildCardBuilder::setCoinReward(const uint8_t& coinReward) { m_card.setCoinReward(coinReward); return *this; }
GuildCardBuilder& GuildCardBuilder::setCaption(const std::string& caption) { m_card.setCaption(caption); return *this; }
GuildCardBuilder& GuildCardBuilder::setColor(const ColorType& color) { m_card.setColor(color); return *this; }
GuildCardBuilder& GuildCardBuilder::addOnPlayAction(const std::function<void()>& action) { m_card.addOnPlayAction(action); return *this; }
GuildCard GuildCardBuilder::build() { return std::move(m_card); }