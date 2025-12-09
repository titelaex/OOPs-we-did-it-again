module Models.GuildCard;

import <iostream>;


using namespace Models;

GuildCard::GuildCard(GuildCard&& other) noexcept
	: Card(std::move(other)) {}
GuildCard& GuildCard::operator=(GuildCard&& other) noexcept { if (this != &other) { Card::operator=(std::move(other));  } return *this; }

void GuildCard::displayCardInfo() {
	Card::displayCardInfo();
	std::cout << "Guild Name: " << getName() << "\n";
}

void GuildCard::onDiscard()
{
    Card::onDiscard();
}

GuildCardBuilder& GuildCardBuilder::setName(const std::string& name) { m_card.setName(name); return *this; }
GuildCardBuilder& GuildCardBuilder::setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost) { m_card.setResourceCost(resourceCost); return *this; }
GuildCardBuilder& GuildCardBuilder::setVictoryPoints(const uint8_t& victoryPoints) { m_card.setVictoryPoints(victoryPoints); return *this; }
GuildCardBuilder& GuildCardBuilder::setCaption(const std::string& caption) { m_card.setCaption(caption); return *this; }
GuildCardBuilder& GuildCardBuilder::setColor(const ColorType& color) { m_card.setColor(color); return *this; }
GuildCardBuilder& GuildCardBuilder::addOnPlayAction(const std::function<void()>& action) { m_card.addOnPlayAction(action); return *this; }
GuildCardBuilder& GuildCardBuilder::addOnDiscardAction(const std::function<void()>& action) { m_card.addOnDiscardAction(action); return *this; }
GuildCard GuildCardBuilder::build() { return std::move(m_card); }