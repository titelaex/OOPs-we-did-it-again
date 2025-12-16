module Models.Card;
import Models.ResourceType;
import Models.ScientificSymbolType;
import Models.LinkingSymbolType;
import Models.TradeRuleType;
import Models.Age;
import <optional>;
import <unordered_map>;

namespace Models {
	Card::Card(Card&& other) = default;
	Card& Card::operator=(Card&& other) = default;

	const std::string& Card::getName() const { return m_name; }
	const std::unordered_map<ResourceType, uint8_t>& Card::getResourceCost() const { return m_resourceCost; }
	const std::string& Card::getCaption() const { return m_caption; }
	const ColorType& Card::getColor() const { return m_color; }
	const uint8_t& Card::getVictoryPoints() const { return m_victoryPoints; }
	const std::vector<std::function<void()>>& Card::getOnPlayActions() const { return m_onPlayActions; }
	const std::vector<std::function<void()>>& Card::getOnDiscardActions() const { return m_onDiscardActions; }
	const bool& Card::isVisible() const { return m_isVisible; }
	const bool& Card::isAvailable() const { return m_isAvailable; }

	void Card::onPlay()
	{
		for (const auto& action : m_onPlayActions) {
			if (action) action();
		}
	}


	void Card::onDiscard()
	{
		for (const auto& action : m_onDiscardActions) {
			if (action) action();
		}
	}

	// Default neutral implementations for virtual getters
	const std::unordered_map<ResourceType, uint8_t>& Card::getResourcesProduction() const {
		static const std::unordered_map<ResourceType, uint8_t> empty{};
		return empty;
	}
	const uint8_t& Card::getShieldPoints() const {
		static const uint8_t zero = 0;
		return zero;
	}
	const std::optional<ScientificSymbolType>& Card::getScientificSymbols() const {
		static const std::optional<ScientificSymbolType> none{};
		return none;
	}
	const std::optional<LinkingSymbolType>& Card::getHasLinkingSymbol() const {
		static const std::optional<LinkingSymbolType> none{};
		return none;
	}
	const std::optional<LinkingSymbolType>& Card::getRequiresLinkingSymbol() const {
		static const std::optional<LinkingSymbolType> none{};
		return none;
	}
	const std::unordered_map<TradeRuleType, bool>& Card::getTradeRules() const {
		static const std::unordered_map<TradeRuleType, bool> empty{};
		return empty;
	}
	const Age& Card::getAge() const {
		static const Age kNeutral = Age::AGE_I;
		return kNeutral;
	}
	const ResourceType& Card::getResourceProduction() const {
		static const ResourceType kNone = ResourceType::NO_RESOURCE;
		return kNone;
	}
	bool Card::IsConstructed() const { return false; }

	// Helpers for compact display
	static const char* colorAnsi(ColorType c) {
		switch (c) {
		case ColorType::BROWN: return "\x1b[38;5;94m"; // brown-ish
		case ColorType::GREY: return "\x1b[90m";
		case ColorType::RED: return "\x1b[31m";
		case ColorType::GREEN: return "\x1b[32m";
		case ColorType::YELLOW: return "\x1b[33m";
		case ColorType::BLUE: return "\x1b[34m";
		case ColorType::PURPLE: return "\x1b[35m";
		default: return "\x1b[0m";
		}
	}
	static const char* resetAnsi() { return "\x1b[0m"; }

	static std::string costAbbrev(const std::unordered_map<ResourceType, uint8_t>& rc) {
		// Legend: W=Wood, S=Stone, C=Clay, P=Papyrus, G=Glass
		auto abbrev = [](ResourceType r) -> std::string {
			switch (r) {
			case ResourceType::WOOD: return "W";
			case ResourceType::STONE: return "S";
			case ResourceType::CLAY: return "C";
			case ResourceType::PAPYRUS: return "P";
			case ResourceType::GLASS: return "G";
			default: return "?";
			}
			};
		std::string s;
		bool first = true;
		for (auto& kv : rc) {
			if (!first) s += ','; first = false;
			s += abbrev(kv.first);
			s += ':';
			s += std::to_string(kv.second);
		}
		return s;
	}

	void Card::displayCardInfo() {
		// [Name colored] | VP=n | Cost=W:1,S:2 | Vis=Y/N | Avl=Y/N
		const char* cAnsi = colorAnsi(m_color);
		const char* rAnsi = resetAnsi();
		std::cout << cAnsi << m_name << rAnsi
			<< " | VP=" << static_cast<int>(m_victoryPoints)
			<< " | Cost=" << (!m_resourceCost.empty() ? costAbbrev(m_resourceCost) : "FREE")
			<< " | Vis=" << (m_isVisible ? 'Y' : 'N')
			<< " | Avl=" << (m_isAvailable ? 'Y' : 'N')
			<< "\n";
	}

	void Card::setName(const std::string& name) { m_name = name; }
	void Card::setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost) { m_resourceCost = resourceCost; }
	void Card::setVictoryPoints(const uint8_t& victoryPoints) { m_victoryPoints = victoryPoints; }
	void Card::setCaption(const std::string& caption) { m_caption = caption; }
	void Card::setColor(const ColorType& color) { m_color = color; }
	void Card::addOnPlayAction(const std::function<void()>& action) { m_onPlayActions.push_back(action); }
	void Card::addOnDiscardAction(const std::function<void()>& action) { m_onDiscardActions.push_back(action); }
	void Card::setIsVisible(const bool& isVisible) { m_isVisible = isVisible; }
	void Card::setIsAvailable(const bool& isAvailable) { m_isAvailable = isAvailable; }

	CardBuilder& CardBuilder::setVictoryPoints(const uint8_t& victoryPoints) {
		m_card.setVictoryPoints(victoryPoints);
		return *this;
	}


	CardBuilder& CardBuilder::setCaption(const std::string& caption) {
		m_card.setCaption(caption);
		return *this;
	}

	CardBuilder& CardBuilder::setName(const std::string& name) {
		m_card.setName(name);
		return *this;
	}

	CardBuilder& CardBuilder::setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost) {
		m_card.setResourceCost(resourceCost);
		return *this;
	}

	CardBuilder& CardBuilder::setColor(const ColorType& color) {
		m_card.setColor(color);
		return *this;
	}

	CardBuilder& CardBuilder::addOnPlayAction(const std::function<void()>& action) {
		m_card.addOnPlayAction(action);
		return *this;
	}

	CardBuilder& CardBuilder::addOnDiscardAction(const std::function<void()>& action) {
		m_card.addOnDiscardAction(action);
		return *this;
	}

	Card CardBuilder::build() {
		return std::move(m_card);
	}

	std::ostream& operator<<(std::ostream& out, const Card& card)
	{
		auto csvEscape = [](const std::string& s) -> std::string {
			if (s.find_first_of(",\"\n\r") != std::string::npos) {
				std::string out; out.reserve(s.size() +2);
				out.push_back('"');
				for (char ch : s) {
					if (ch == '"') out += "\"\""; else out.push_back(ch);
				}
				out.push_back('"');
				return out;
			}
			return s;
		};

		std::string costStr;
		const auto& cost = card.getResourceCost();
		bool first = true;
		for (const auto& kv : cost) {
			if (!first) costStr.push_back(';'); first = false;
			costStr += Models::ResourceTypeToString(kv.first);
			costStr.push_back(':');
			costStr += std::to_string(static_cast<int>(kv.second));
		}

		out << csvEscape(card.getName()) << ','
			<< static_cast<int>(card.getColor()) << ','
			<< static_cast<int>(card.getVictoryPoints()) << ','
			<< csvEscape(card.getCaption()) << ','
			<< csvEscape(costStr);
		return out;
	}
}