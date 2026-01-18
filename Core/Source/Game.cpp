#include <exception>
#include <filesystem>
module Core.Game;
import <vector>;
import <string>;
import <random>;
import <iostream>;
import <algorithm>;
import <utility>;
import <fstream>;
import <sstream>;
import <functional>;
import <memory>;
import <optional>;
import Core.PlayerDecisionMaker;
import Core.TrainingLogger;
import Core.MCTS;
import Core.GameStateSerializer;
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
import Models.Player;
import Models.Token; 
import Models.Card;
import Core.CardCsvParser;
import Core.Player;
import Core.AgeTree; 
import Core.Node;
import Core.GameState;
import Core.PlayerNameValidator;
import Core.ConsoleListener;
import <unordered_map>;
namespace Core {
	namespace {
		std::unique_ptr<std::vector<std::unique_ptr<Models::Token>>> setupUnusedProgressTokens;
		const int kNrOfRounds = 20;
		const std::vector<int> kMilitaryTokenPositions = { 2,5,8,11,14,17 };
		bool g_last_active_was_player_one = true;
		std::mt19937_64 make_rng(uint32_t seed) {
			return std::mt19937_64(seed);
		}
		template<typename T>
		void ShuffleInplace(std::vector<T>& v, uint32_t seed) {
			auto rng = make_rng(seed);
			std::shuffle(v.begin(), v.end(), rng);
		}
		template<typename T>
		std::vector<T> ShuffleAndMove(std::vector<T>&& src, uint32_t seed) {
			std::vector<T> dest = std::move(src);
			auto rng = make_rng(seed);
			std::shuffle(dest.begin(), dest.end(), rng);
			return dest;
		}
		std::vector<std::string> splitCSVLine(const std::string& line) {
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
		void loadGenericFile(const std::string& path, const std::function<void(const std::vector<std::string>&)>& onItem, std::ofstream& log) {
			try {
				std::ifstream ifs(path);
				if (!ifs.is_open()) {
					if (log.is_open()) log << "[Error] Unable to open CSV file: " << path << "\n";
					return;
				}
				std::string header;
				std::getline(ifs, header);
				std::string line;
				size_t lineno = 1;
				while (std::getline(ifs, line)) {
					++lineno;
					if (line.empty()) continue;
					auto cols = splitCSVLine(line);
					try {
						onItem(cols);
					}
					catch (const std::exception& ex) {
						if (log.is_open()) log << "[Error] Factory error in " << path << " at line " << lineno << ": " << ex.what() << "\n";
					}
					catch (...) {
						if (log.is_open()) log << "[Error] Unknown factory error in " << path << " at line " << lineno << "\n";
					}
				}
			}
			catch (const std::exception& ex) {
				if (log.is_open()) log << "[Exception] Reading " << path << ": " << ex.what() << "\n";
			}
			catch (...) {
				if (log.is_open()) log << "[Exception] Unknown error reading " << path << "\n";
			}
		}
		std::pair<std::vector<std::unique_ptr<Models::Token>>, std::vector<std::unique_ptr<Models::Token>>> startGameTokens(std::vector<std::unique_ptr<Models::Token>> allTokens) {
			if (!setupUnusedProgressTokens) setupUnusedProgressTokens = std::make_unique<std::vector<std::unique_ptr<Models::Token>>>();
			setupUnusedProgressTokens->clear();
			std::vector<std::unique_ptr<Models::Token>> progress;
			std::vector<std::unique_ptr<Models::Token>> military;
			progress.reserve(allTokens.size());
			military.reserve(allTokens.size());
			for (auto& t : allTokens) {
				if (!t) continue;
				if (t->getType() == Models::TokenType::PROGRESS) progress.push_back(std::move(t));
				else if (t->getType() == Models::TokenType::MILITARY) military.push_back(std::move(t));
			}
			std::vector<std::unique_ptr<Models::Token>> selectedProgress;
			constexpr size_t kSelectCount = 5;
			if (progress.size() <= kSelectCount) {
				selectedProgress = std::move(progress);
			}
			else {
				std::random_device rd;
				std::mt19937 gen(rd());
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
		bool determineChooserFromBoardAndLastActive(bool lastActiveWasPlayerOne) {
			auto& board = Board::getInstance();
			int pos = board.getPawnPos();
			if (pos < 9) return true;
			else if (pos > 9) return false;
			else return lastActiveWasPlayerOne;
		}
		int checkImmediateMilitaryVictory() {
			auto& board = Board::getInstance();
			int pos = board.getPawnPos();
			if (pos <= 0) return 0;
			if (pos >= 18) return 1;
			return -1;
		}
		int checkImmediateScientificVictory(Player& p1, Player& p2) {
			auto countDistinct = [](const std::unique_ptr<Models::Player>& mp)->size_t {
				if (!mp) return 0;
				const auto& syms = mp->getOwnedScientificSymbols();
				size_t cnt = 0;
				for (const auto& kv : syms) if (kv.second > 0) ++cnt;
				return cnt;
				};
			size_t s1 = countDistinct(p1.m_player);
			size_t s2 = countDistinct(p2.m_player);
			if (s1 >= 6 && s2 >= 6) return -1;
			if (s1 >= 6) return 0;
			if (s2 >= 6) return 1;
			return -1;
		}
		int determineCivilianWinner(Player& p1, Player& p2) {
			auto& board = Board::getInstance();
			int pawnPos = board.getPawnPos();

			auto getMilitaryVP = [](int pawnPos) -> uint32_t {
				if (pawnPos == 9) return 0;

				if (pawnPos > 9) {
					int distance = pawnPos - 9;
					if (distance >= 2 && distance <= 3) return 2;
					if (distance >= 4 && distance <= 6) return 5;
					if (distance >= 7 && distance <= 9) return 10;
				}
				else {
					int distance = 9 - pawnPos;
					if (distance >= 1 && distance <= 2) return 2;
					if (distance >= 3 && distance <= 5) return 5;
					if (distance >= 6 && distance <= 8) return 10;
				}
				return 0;
				};

			auto calculateScore = [&](std::optional<std::reference_wrapper<const Models::Player>> mp)->uint32_t {
				if (!mp.has_value()) return 0;
				const auto& player = mp->get();
				const auto& ptsRef = player.getPoints();
				uint32_t pts = getMilitaryVP(pawnPos)
					+ static_cast<uint32_t>(ptsRef.m_buildingVictoryPoints)
					+ static_cast<uint32_t>(ptsRef.m_wonderVictoryPoints)
					+ static_cast<uint32_t>(ptsRef.m_progressVictoryPoints);
				// totalCoins is not const, so we need to call it on a non-const reference
				// Extract coins tuple first, then call totalCoins on the original non-const player
				auto coins = player.getRemainingCoins();
				uint8_t totalCoins = const_cast<Models::Player&>(player).totalCoins(coins);
				pts += static_cast<uint32_t>(totalCoins / 3);
				return pts;
				};

			std::optional<std::reference_wrapper<const Models::Player>> m1;
			if (p1.m_player) m1 = std::cref(*p1.m_player);
			std::optional<std::reference_wrapper<const Models::Player>> m2;
			if (p2.m_player) m2 = std::cref(*p2.m_player);
			uint32_t total1 = calculateScore(m1);
			uint32_t total2 = calculateScore(m2);
			if (total1 > total2) return 0;
			if (total2 > total1) return 1;

			uint32_t b1 = m1.has_value() ? m1->get().getBlueBuildingVictoryPoints() : 0;
			uint32_t b2 = m2.has_value() ? m2->get().getBlueBuildingVictoryPoints() : 0;
			if (b1 > b2) return 0;
			if (b2 > b1) return 1;
			return -1;
		}
		void performCardAction(int action, Player& cur, Player& opp, std::unique_ptr<Models::Card>& cardPtr, Board& board, std::optional<std::reference_wrapper<IPlayerDecisionMaker>> decisionMaker = std::nullopt) {
			if (!cardPtr) return;

			switch (action) {
			case 0: {
				if (!cur.canAffordCard(*cardPtr, opp.m_player)) {
					auto& notifier = GameState::getInstance().getEventNotifier();
					DisplayRequestEvent event;
					event.displayType = DisplayRequestEvent::Type::ERROR;
					event.context = "ERROR: You cannot afford to build this card! The card will be returned to the tree.";
					notifier.notifyDisplayRequested(event);
					return;
				}
				try {
					cur.playCardBuilding(cardPtr, opp.m_player);
				}
				catch (const std::exception& ex) {
					auto& notifier = GameState::getInstance().getEventNotifier();
					DisplayRequestEvent event;
					event.displayType = DisplayRequestEvent::Type::ERROR;
					event.context = std::string("Exception in playCardBuilding: ") + ex.what();
					notifier.notifyDisplayRequested(event);
				}
				break;
			}
			case 1: {
				auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
				discarded.push_back(std::move(cardPtr));
				break;
			}
			case 2: {
				if (Models::Wonder::getWondersBuilt() >= Models::Wonder::MaxWonders) {
					auto& notifier = GameState::getInstance().getEventNotifier();
					DisplayRequestEvent event;
					event.displayType = DisplayRequestEvent::Type::ERROR;
					event.context = "ERROR: Cannot build wonder - maximum of 7 wonders have already been built in the game! The card will be returned to the tree.";
					notifier.notifyDisplayRequested(event);
					return;
				}
				auto& owned = cur.m_player->getOwnedWonders();
				std::vector<size_t> candidates;
				for (size_t i = 0; i < owned.size(); ++i) {
					if (owned[i] && !owned[i]->IsConstructed())
						candidates.push_back(i);
				}
				if (candidates.empty()) {
					auto& notifier = GameState::getInstance().getEventNotifier();
					DisplayRequestEvent event;
					event.displayType = DisplayRequestEvent::Type::MESSAGE;
					event.context = "No available unbuilt wonders. Moving card to discard.";
					notifier.notifyDisplayRequested(event);
					auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());

					Core::Game::getNotifier().notifyCardDiscarded({
					static_cast<int>(cur.m_player->getkPlayerId()),
					cur.m_player->getPlayerUsername(),
					cardPtr->getName(),
					-1,
					Models::ColorTypeToString(cardPtr->getColor()),
					{"Discarded (No wonders available)"}
						});

					discarded.push_back(std::move(cardPtr));
					break;
				}
				auto& notifier = GameState::getInstance().getEventNotifier();
				DisplayRequestEvent headerEvent;
				headerEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
				headerEvent.context = "Choose wonder to construct:";
				notifier.notifyDisplayRequested(headerEvent);

				for (size_t i = 0; i < candidates.size(); ++i) {
					DisplayRequestEvent cardEvent;
					cardEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
					cardEvent.context = "[" + std::to_string(i) + "] " + owned[candidates[i]]->getName();
					notifier.notifyDisplayRequested(cardEvent);
				}

				size_t wchoice = decisionMaker.has_value() ? decisionMaker->get().selectWonder(candidates) : 0;
				std::unique_ptr<Models::Wonder>& chosenWonderPtr = owned[candidates[wchoice]];
				if (!cur.canAffordWonder(chosenWonderPtr, opp.m_player)) {
					auto& notifier = GameState::getInstance().getEventNotifier();
					DisplayRequestEvent event;
					event.displayType = DisplayRequestEvent::Type::ERROR;
					event.context = "ERROR: You cannot afford to build this wonder! The card will be returned to the tree.";
					notifier.notifyDisplayRequested(event);
					return;
				}
				std::vector<Models::Token> discardedTokens;
				auto& discardedCards = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
				cur.playCardWonder(chosenWonderPtr, cardPtr, opp.m_player, discardedTokens, discardedCards);
				break;
			}
			default:
			{
				auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
				discarded.push_back(std::move(cardPtr));
			}
			break;
			}
		}
		uint8_t getShieldPointsFromCard(const Models::Card& card, std::optional<std::reference_wrapper<const Player>> player = std::nullopt) {
			if (auto ac = dynamic_cast<const Models::AgeCard*>(&card)) {
				uint8_t shields = static_cast<uint8_t>(ac->getShieldPoints());
				if (player.has_value() && player->get().m_player && player->get().m_player->hasToken(Models::TokenIndex::STRATEGY)) {
					if (card.getColor() == Models::ColorType::RED) {
						shields += 1;
					}
				}
				return shields;
			}
			return 0;
		}
		void displayCardDetails(const Models::Card& card) {
			auto& notifier = GameState::getInstance().getEventNotifier();

			DisplayRequestEvent event;
			event.displayType = DisplayRequestEvent::Type::MESSAGE;
			event.context = "\n=== CARD DETAILS ===";
			notifier.notifyDisplayRequested(event);

			event.context = "Name: " + std::string(card.getName());
			notifier.notifyDisplayRequested(event);

			event.context = "Color: " + Models::ColorTypeToString(card.getColor());
			notifier.notifyDisplayRequested(event);

			const auto& resCost = card.getResourceCost();
			event.context = "Resource Cost: ";
			if (resCost.empty()) {
				event.context += "FREE";
			}
			else {
				bool first = true;
				for (const auto& [res, amt] : resCost) {
					if (!first) event.context += ", ";
					first = false;
					event.context += Models::ResourceTypeToString(res) + " x" + std::to_string(amt);
				}
			}
			notifier.notifyDisplayRequested(event);

			if (auto* ageCard = dynamic_cast<const Models::AgeCard*>(&card)) {
				uint8_t coinCost = ageCard->getCoinCost();
				if (coinCost > 0) {
					event.context = "Coin Cost: " + std::to_string(coinCost);
					notifier.notifyDisplayRequested(event);
				}
			}

			if (card.getVictoryPoints() > 0) {
				event.context = "Victory Points: " + std::to_string(card.getVictoryPoints());
				notifier.notifyDisplayRequested(event);
			}
			if (auto* ageCard = dynamic_cast<const Models::AgeCard*>(&card)) {
				const auto& resProd = ageCard->getResourcesProduction();
				if (!resProd.empty()) {
					event.context = "Produces: ";
					bool first = true;
					for (const auto& [res, amt] : resProd) {
						if (!first) event.context += ", ";
						first = false;
						event.context += Models::ResourceTypeToString(res) + " x" + std::to_string(amt);
					}
					notifier.notifyDisplayRequested(event);
				}
				if (ageCard->getShieldPoints() > 0) {
					event.context = "Shield Points: " + std::to_string(ageCard->getShieldPoints());
					notifier.notifyDisplayRequested(event);
				}
				if (ageCard->getScientificSymbols().has_value()) {
					event.context = "Scientific Symbol: " + Models::ScientificSymbolTypeToString(ageCard->getScientificSymbols().value());
					notifier.notifyDisplayRequested(event);
				}
				const auto& tradeRules = ageCard->getTradeRules();
				if (!tradeRules.empty()) {
					event.context = "Trade Discount: ";
					bool first = true;
					for (const auto& [rule, enabled] : tradeRules) {
						if (enabled) {
							if (!first) event.context += ", ";
							first = false;
							event.context += Models::tradeRuleTypeToString(rule);
						}
					}
					notifier.notifyDisplayRequested(event);
				}
			}
			if (auto* wonder = dynamic_cast<const Models::Wonder*>(&card)) {
				if (wonder->getShieldPoints() > 0) {
					event.context = "Shield Points: " + std::to_string(wonder->getShieldPoints());
					notifier.notifyDisplayRequested(event);
				}
				if (wonder->getResourceProduction() != Models::ResourceType::NO_RESOURCE) {
					event.context = "Produces: " + Models::ResourceTypeToString(wonder->getResourceProduction());
					notifier.notifyDisplayRequested(event);
				}
			}
			event.context = "====================\n";
			notifier.notifyDisplayRequested(event);
		}
		void displayPlayerResources(const Player& player, const std::string& label) {
			if (!player.m_player) return;
			auto& notifier = GameState::getInstance().getEventNotifier();

			DisplayRequestEvent event;
			event.displayType = DisplayRequestEvent::Type::MESSAGE;
			event.context = "\n--- " + std::string(player.m_player->getPlayerUsername()) + " RESOURCES ---";
			notifier.notifyDisplayRequested(event);

			auto coins = player.m_player->getRemainingCoins();
			uint32_t totalCoins = player.m_player->totalCoins(coins);
			event.context = "Coins: " + std::to_string(totalCoins);
			notifier.notifyDisplayRequested(event);

			const auto& permRes = player.m_player->getOwnedPermanentResources();
			event.context = "Permanent Resources: ";
			if (permRes.empty()) {
				event.context += "None";
			}
			else {
				bool first = true;
				for (const auto& [res, amt] : permRes) {
					if (!first) event.context += ", ";
					first = false;
					event.context += Models::ResourceTypeToString(res) + " x" + std::to_string(amt);
				}
			}
			notifier.notifyDisplayRequested(event);

			const auto& tradingRes = player.m_player->getOwnedTradingResources();
			if (!tradingRes.empty()) {
				event.context = "Trading Resources: ";
				bool first = true;
				for (const auto& [res, amt] : tradingRes) {
					if (!first) event.context += ", ";
					first = false;
					event.context += Models::ResourceTypeToString(res) + " x" + std::to_string(amt);
				}
				notifier.notifyDisplayRequested(event);
			}

			event.context = "----------------------------\n";
			notifier.notifyDisplayRequested(event);
		}

		bool hasTokenByName(const Models::Player& player, const std::string& tokenName)
		{
			for (const auto& t : player.getOwnedTokens()) {
				if (t && t->getName() == tokenName) return true;
			}
			return false;
		}

		int tradeDiscountForResource(const Models::Player& player, Models::ResourceType resource)
		{
			const auto& tradeRules = player.getTradeRules();
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
			if (it != tradeRules.end() && it->second) return 1;
			return -1;
		}

		std::unordered_map<Models::ResourceType, uint8_t> computeOpponentBrownGreyProductionFor(
			const Models::Player& cur,
			const Models::Player& opp,
			const std::unordered_map<Models::ResourceType, uint8_t>& missingResources)
		{
			std::unordered_map<Models::ResourceType, uint8_t> opponentBrownGreyProduction;
			if (missingResources.empty()) return opponentBrownGreyProduction;

			for (const auto& card : opp.getOwnedCards()) {
				if (!card) continue;
				if (card->getColor() != Models::ColorType::BROWN && card->getColor() != Models::ColorType::GREY) continue;

				const auto* ageCard = dynamic_cast<const Models::AgeCard*>(card.get());
				if (!ageCard) continue;

				for (const auto& pair : ageCard->getResourcesProduction()) {
					auto res = pair.first;
					auto amt = pair.second;
					if (missingResources.find(res) == missingResources.end()) continue;
					if (tradeDiscountForResource(cur, res) != -1) continue;
					opponentBrownGreyProduction[res] = static_cast<uint8_t>(opponentBrownGreyProduction[res] + amt);
				}
			}

			return opponentBrownGreyProduction;
		}
	}

	GameEventNotifier& Game::getNotifier()
	{
		static GameEventNotifier notifier;
		return notifier;
	}

	void Game::preparation() {
		try {
			PrepareBoardCardPools();
			auto& board = Core::Board::getInstance();
			board.setPawnPos(9);
			std::bitset<19> pawnTrack;
			pawnTrack.set(8);
			board.setPawnTrack(pawnTrack);

			auto& notifier = GameState::getInstance().getEventNotifier();
			DisplayRequestEvent debugEvent;
			debugEvent.displayType = DisplayRequestEvent::Type::MESSAGE;

			auto findExistingPath = [](const std::vector<std::string>& candidates) -> std::string {
				for (const auto& p : candidates) {
					try { if (std::filesystem::exists(p)) return p; }
					catch (...) {}
				}
				return std::string{};
				};

			const std::vector<std::string> tokenCandidates = {
				"Config/Tokens.csv", "../Config/Tokens.csv", "../../Config/Tokens.csv",
				"Core/Config/Tokens.csv", "../Core/Config/Tokens.csv", "../../Core/Config/Tokens.csv",
				"Tokens.csv", "Resources/Tokens.csv", "Core/Resources/Tokens.csv",
				"../Core/Resources/Tokens.csv", "../ModelsDLL/Tokens.csv", "ModelsDLL/Tokens.csv"
			};

			std::string tokenPath = findExistingPath(tokenCandidates);
			if (tokenPath.empty()) {
				debugEvent.context = "[ERROR] Tokens.csv file not found in any candidate path!";
				notifier.notifyDisplayRequested(debugEvent);
				throw std::runtime_error("Tokens.csv not found");
			}

			debugEvent.context = "Loading tokens from: " + tokenPath;
			notifier.notifyDisplayRequested(debugEvent);

			auto allTokens = parseTokensFromCSV(tokenPath);

			debugEvent.context = "Loaded " + std::to_string(allTokens.size()) + " tokens from CSV";
			notifier.notifyDisplayRequested(debugEvent);

			auto [progressSelected, military] = startGameTokens(std::move(allTokens));

			debugEvent.context = "Selected " + std::to_string(progressSelected.size()) + " progress tokens and " +
				std::to_string(military.size()) + " military tokens";
			notifier.notifyDisplayRequested(debugEvent);

			Core::Board::getInstance().setProgressTokens(std::move(progressSelected));
			Core::Board::getInstance().setMilitaryTokens(std::move(military));
			Core::Board::getInstance().setUnusedProgressTokens(std::move(*setupUnusedProgressTokens));

			debugEvent.context = "Tokens setup complete - board should now have " +
				std::to_string(board.getProgressTokens().size()) + " progress tokens";
			notifier.notifyDisplayRequested(debugEvent);
		}
		catch (const std::exception& ex) {
			auto& notifier = GameState::getInstance().getEventNotifier();
			DisplayRequestEvent event;
			event.displayType = DisplayRequestEvent::Type::ERROR;
			event.context = std::string("Preparation exception: ") + ex.what();
			notifier.notifyDisplayRequested(event);
			throw;
		}
		catch (...) {
			auto& notifier = GameState::getInstance().getEventNotifier();
			DisplayRequestEvent event;
			event.displayType = DisplayRequestEvent::Type::ERROR;
			event.context = "Unknown exception during preparation";
			notifier.notifyDisplayRequested(event);
			throw;
		}
	}
	void Game::PrepareBoardCardPools() {
		auto& board = Board::getInstance();
		auto& notifier = GameState::getInstance().getEventNotifier();
		board.setupCardPools();
		uint32_t seed = static_cast<uint32_t>(std::random_device{}());
		std::ofstream log("Preparation.log", std::ios::app);
		auto configureRowVisibility = [](std::vector<std::shared_ptr<Node>>& nodes, const std::vector<size_t>& rowPattern) {
			size_t idx = 0;
			bool rowVisible = true;
			for (size_t row = 0; row < rowPattern.size() && idx < nodes.size(); ++row) {
				const bool isLastRow = (row == rowPattern.size() - 1);
				const size_t rowCount = rowPattern[row];
				for (size_t col = 0; col < rowCount && idx < nodes.size(); ++col, ++idx) {
					if (auto node = nodes[idx]) {
						if (auto card = node->getCard(); card.has_value()) {
							card->get().setIsVisible(rowVisible);
							card->get().setIsAvailable(isLastRow);
						}
					}
				}
				rowVisible = !rowVisible;
			}
			};
		try {
			auto cwd = std::filesystem::current_path();
			if (log.is_open()) log << "Current working directory: " << cwd.string() << "\n";
			DisplayRequestEvent event;
			event.displayType = DisplayRequestEvent::Type::MESSAGE;
			event.context = "Current working directory: " + cwd.string();
			notifier.notifyDisplayRequested(event);
		}
		catch (...) {
			if (log.is_open()) log << "[Warning] Unable to determine current working directory\n";
		}
		auto findExistingPath = [&](const std::vector<std::string>& candidates) -> std::string {
			for (const auto& p : candidates) {
				try { if (std::filesystem::exists(p)) return p; }
				catch (...) {}
			}
			return std::string{};
			};
		try {
			std::vector<Models::AgeCard> allAgeCards;
			auto loadFn = [&](const std::vector<std::string>& cols) {
				Models::AgeCard card = ageCardFactory(cols);
				allAgeCards.push_back(std::move(card));
				};
			const std::vector<std::string> ageCandidates = {
				"Core/Config/AgeCards.csv", "../Core/Config/AgeCards.csv", "../../Core/Config/AgeCards.csv",
				"AgeCards.csv", "Resources/AgeCards.csv", "Core/Resources/AgeCards.csv",
				"../Core/Resources/AgeCards.csv", "../ModelsDLL/AgeCards.csv", "ModelsDLL/AgeCards.csv", "../Models/AgeCards.csv"
			};
			std::string agePath = findExistingPath(ageCandidates);
			if (agePath.empty()) {
				if (log.is_open()) log << "[Error] Age file not found in candidates\n";
				DisplayRequestEvent event;
				event.displayType = DisplayRequestEvent::Type::ERROR;
				event.context = "[Error] Age file not found in candidates";
				notifier.notifyDisplayRequested(event);
			}
			else {
				if (log.is_open()) log << "Using Age file: " << agePath << "\n";
				DisplayRequestEvent event;
				event.displayType = DisplayRequestEvent::Type::MESSAGE;
				event.context = "Using Age file: " + agePath;
				notifier.notifyDisplayRequested(event);
				loadGenericFile(agePath, loadFn, log);
			}
			std::vector<std::unique_ptr<Models::Card>> age1, age2, age3;
			for (size_t i = 0; i < allAgeCards.size(); ++i) {
				auto& c = allAgeCards[i];
				switch (c.getAge()) {
				case Models::Age::AGE_I:
					age1.push_back(std::make_unique<Models::AgeCard>(std::move(c)));
					break;
				case Models::Age::AGE_II:
					age2.push_back(std::make_unique<Models::AgeCard>(std::move(c)));
					break;
				case Models::Age::AGE_III:
				default:
					age3.push_back(std::make_unique<Models::AgeCard>(std::move(c)));
					break;
				}
			}
			ShuffleInplace(age1, seed);
			ShuffleInplace(age2, seed + 1);
			ShuffleInplace(age3, seed + 2);
			board.setUnusedAgeOneCards(std::move(age1));
			board.setUnusedAgeTwoCards(std::move(age2));
			board.setUnusedAgeThreeCards(std::move(age3));
			if (log.is_open()) log << "Loaded ages: I=" << board.getUnusedAgeOneCards().size()
				<< " II=" << board.getUnusedAgeTwoCards().size() << " III=" << board.getUnusedAgeThreeCards().size() << "\n";
			DisplayRequestEvent ageEvent;
			ageEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
			ageEvent.context = "Loaded ages: I=" + std::to_string(board.getUnusedAgeOneCards().size()) +
				" II=" + std::to_string(board.getUnusedAgeTwoCards().size()) +
				" III=" + std::to_string(board.getUnusedAgeThreeCards().size());
			notifier.notifyDisplayRequested(ageEvent);
		}
		catch (const std::exception& ex) {
			if (log.is_open()) log << "[Exception] While processing age cards: " << ex.what() << "\n";
		}
		try {
			std::vector<Models::GuildCard> tempGuilds;
			auto loadG = [&](const std::vector<std::string>& cols) {
				Models::GuildCard g = guildCardFactory(cols);
				tempGuilds.push_back(std::move(g));
				};
			const std::vector<std::string> guildCandidates = {
				"Core/Config/Guilds.csv", "../Core/Config/Guilds.csv", "../../Core/Config/Guilds.csv",
				"Guilds.csv", "Resources/Guilds.csv", "Core/Resources/Guilds.csv",
				"../Core/Resources/Guilds.csv", "../ModelsDLL/Guilds.csv", "ModelsDLL/Guilds.csv", "../Models/Guilds.csv"
			};
			std::string guildPath = findExistingPath(guildCandidates);
			if (guildPath.empty()) {
				if (log.is_open()) log << "[Error] Guild file not found in candidates\n";
				DisplayRequestEvent event;
				event.displayType = DisplayRequestEvent::Type::ERROR;
				event.context = "[Error] Guild file not found in candidates";
				notifier.notifyDisplayRequested(event);
			}
			else {
				if (log.is_open()) log << "Using Guild file: " << guildPath << "\n";
				DisplayRequestEvent event;
				event.displayType = DisplayRequestEvent::Type::MESSAGE;
				event.context = "Using Guild file: " + guildPath;
				notifier.notifyDisplayRequested(event);
				loadGenericFile(guildPath, loadG, log);
			}
			ShuffleInplace(tempGuilds, seed);
			std::vector<std::unique_ptr<Models::Card>> guildPool;
			for (auto& g : tempGuilds) guildPool.push_back(std::make_unique<Models::GuildCard>(std::move(g)));
			board.setUnusedGuildCards(std::move(guildPool));
			if (log.is_open()) log << "Loaded guilds: " << board.getUnusedGuildCards().size() << "\n";
			DisplayRequestEvent guildEvent;
			guildEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
			guildEvent.context = "Loaded guilds: " + std::to_string(board.getUnusedGuildCards().size());
			notifier.notifyDisplayRequested(guildEvent);
		}
		catch (const std::exception& ex) {
			if (log.is_open()) log << "[Exception] While processing guild cards: " << ex.what() << "\n";
		}
		try {
			std::vector<Models::Wonder> tempWonders;
			auto loadW = [&](const std::vector<std::string>& cols) {
				Models::Wonder w = wonderFactory(cols);
				tempWonders.push_back(std::move(w));
				};
			const std::vector<std::string> wonderCandidates = {
				"Core/Config/Wonders.csv", "../Core/Config/Wonders.csv", "../../Core/Config/Wonders.csv",
				"Wonders.csv", "Resources/Wonders.csv", "Core/Resources/Wonders.csv",
				"../Core/Resources/Wonders.csv", "../ModelsDLL/Wonders.csv", "ModelsDLL/Wonders.csv", "../Models/Wonders.csv"
			};
			std::string wonderPath = findExistingPath(wonderCandidates);
			if (wonderPath.empty()) {
				if (log.is_open()) log << "[Error] Wonder file not found in candidates\n";
				auto& pool = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedAgeTwoCards());
				DisplayRequestEvent event;
				event.displayType = DisplayRequestEvent::Type::ERROR;
				event.context = "[Error] Wonder file not found in candidates";
				notifier.notifyDisplayRequested(event);
			}
			else {
				if (log.is_open()) log << "Using Wonder file: " << wonderPath << "\n";
				DisplayRequestEvent event;
				event.displayType = DisplayRequestEvent::Type::MESSAGE;
				event.context = "Using Wonder file: " + wonderPath;
				notifier.notifyDisplayRequested(event);
				loadGenericFile(wonderPath, loadW, log);
			}
			ShuffleInplace(tempWonders, seed);
			std::vector<std::unique_ptr<Models::Card>> wonderPool;
			for (auto& w : tempWonders) wonderPool.push_back(std::make_unique<Models::Wonder>(std::move(w)));
			board.setUnusedWonders(std::move(wonderPool));
			if (log.is_open()) log << "Loaded wonders: " << board.getUnusedWonders().size() << "\n";
			DisplayRequestEvent wonderEvent;
			wonderEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
			wonderEvent.context = "Loaded wonders: " + std::to_string(board.getUnusedWonders().size());
			notifier.notifyDisplayRequested(wonderEvent);
		}
		catch (const std::exception& ex) {
			if (log.is_open()) log << "[Exception] While processing wonder cards: " << ex.what() << "\n";
		}
		{
			std::vector<std::unique_ptr<Models::Card>> selected;
			auto& pool = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedAgeOneCards());
			size_t take = std::min<size_t>(20, pool.size());
			size_t i = 0;
			while (i < pool.size() && selected.size() < take) {
				if (!pool[i]) { ++i; continue; }
				if (dynamic_cast<Models::AgeCard*>(pool[i].get())) {
					selected.push_back(std::move(pool[i]));
					pool.erase(pool.begin() + i);
				}
				else {
					++i;
				}
			}
			selected = ShuffleAndMove(std::move(selected), seed);
			Core::Age1Tree tree(std::move(selected));
			auto age1Nodes = tree.releaseNodes();
			configureRowVisibility(age1Nodes, { 2,3,4,5,6 });
			board.setAge1Nodes(std::move(age1Nodes));
		}
		{
			std::vector<std::unique_ptr<Models::Card>> selected;
			auto& pool = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedAgeTwoCards());
			size_t take = std::min<size_t>(20, pool.size());
			size_t i = 0;
			while (i < pool.size() && selected.size() < take) {
				if (!pool[i]) { ++i; continue; }
				if (dynamic_cast<Models::AgeCard*>(pool[i].get())) {
					selected.push_back(std::move(pool[i]));
					pool.erase(pool.begin() + i);
				}
				else {
					++i;
				}
			}
			selected = ShuffleAndMove(std::move(selected), seed + 1);
			Core::Age2Tree tree(std::move(selected));
			auto age2Nodes = tree.releaseNodes();
			configureRowVisibility(age2Nodes, { 6,5,4,3,2 });
			board.setAge2Nodes(std::move(age2Nodes));
		}
		{
			std::vector<std::unique_ptr<Models::Card>> selected;
			auto& pool3 = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedAgeThreeCards());
			auto& poolG = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedGuildCards());
			size_t take3 = std::min<size_t>(17, pool3.size());
			size_t takeG = std::min<size_t>(3, poolG.size());
			size_t i = 0;
			while (i < pool3.size() && selected.size() < take3) {
				if (!pool3[i]) { ++i; continue; }
				if (dynamic_cast<Models::AgeCard*>(pool3[i].get())) {
					selected.push_back(std::move(pool3[i]));
					pool3.erase(pool3.begin() + i);
				}
				else {
					++i;
				}
			}
			i = 0;
			size_t movedG = 0;
			while (i < poolG.size() && movedG < takeG) {
				if (!poolG[i]) { ++i; continue; }
				if (dynamic_cast<Models::GuildCard*>(poolG[i].get())) {
					selected.push_back(std::move(poolG[i]));
					poolG.erase(poolG.begin() + i);
					++movedG;
				}
				else {
					++i;
				}
			}
			selected = ShuffleAndMove(std::move(selected), seed + 2);
			Core::Age3Tree tree(std::move(selected));
			auto age3Nodes = tree.releaseNodes();
			configureRowVisibility(age3Nodes, { 2,3,4,2,4,3,2 });
			board.setAge3Nodes(std::move(age3Nodes));
		}
		auto printNodes = [](const char* title, const std::vector<std::shared_ptr<Node>>& nodes) {
			auto& notifier = GameState::getInstance().getEventNotifier();
			DisplayRequestEvent event;
			event.displayType = DisplayRequestEvent::Type::MESSAGE;
			event.context = "===== " + std::string(title) + " (" + std::to_string(nodes.size()) + ") =====";
			notifier.notifyDisplayRequested(event);

			auto idxOf = [&](const Node* ptr) -> std::string {
				if (!ptr) return "-";
				for (size_t j = 0; j < nodes.size(); ++j) {
					if (nodes[j].get() == ptr) return std::to_string(j);
				}
				return "?";
				};
			auto nameOf = [&](const Node* ptr) -> std::string {
				if (!ptr) return "-";
				auto c = ptr->getCard();
				return c.has_value() ? c->get().getName() : std::string{ "<none>" };
				};
			for (size_t i = 0; i < nodes.size(); ++i) {
				const auto& n = nodes[i];
				if (!n) continue;
				auto c = n->getCard();
				event.context = "[" + std::to_string(i) + "] " + (c.has_value() ? c->get().getName() : std::string{ "<none>" });
				notifier.notifyDisplayRequested(event);
				const Node* p1 = n->getParent1().get();
				const Node* p2 = n->getParent2().get();
				const Node* ch1 = n->getChild1().get();
				const Node* ch2 = n->getChild2().get();
				event.context = "  Parents: (" + idxOf(p1) + ") " + nameOf(p1) + ", (" + idxOf(p2) + ") " + nameOf(p2);
				notifier.notifyDisplayRequested(event);
				event.context = "  Children: (" + idxOf(ch1) + ") " + nameOf(ch1) + ", (" + idxOf(ch2) + ") " + nameOf(ch2);
				notifier.notifyDisplayRequested(event);
			}
			};
		printNodes("Age I", Board::getInstance().getAge1Nodes());
		printNodes("Age II", Board::getInstance().getAge2Nodes());
		printNodes("Age III", Board::getInstance().getAge3Nodes());
	}
	void Game::wonderSelection(std::shared_ptr<Core::Player>& p1, std::shared_ptr<Core::Player>& p2, 
	                           std::optional<std::reference_wrapper<IPlayerDecisionMaker>> p1Decisions, 
	                           std::optional<std::reference_wrapper<IPlayerDecisionMaker>> p2Decisions) {
		std::unique_ptr<IPlayerDecisionMaker> p1DecisionsOwner;
		std::unique_ptr<IPlayerDecisionMaker> p2DecisionsOwner;
		IPlayerDecisionMaker* p1DecisionsPtr = nullptr;
		IPlayerDecisionMaker* p2DecisionsPtr = nullptr;
		
		if (!p1Decisions.has_value()) {
			p1DecisionsOwner = std::make_unique<HumanDecisionMaker>();
			p1DecisionsPtr = p1DecisionsOwner.get();
		} else {
			p1DecisionsPtr = &p1Decisions->get();
		}
		if (!p2Decisions.has_value()) {
			p2DecisionsOwner = std::make_unique<HumanDecisionMaker>();
			p2DecisionsPtr = p2DecisionsOwner.get();
		} else {
			p2DecisionsPtr = &p2Decisions->get();
		}
		auto& notifier = GameState::getInstance().getEventNotifier();
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, 1);
		bool player1Starts = (dis(gen) == 0);
		std::vector<std::unique_ptr<Models::Wonder>> availableWonders;
		auto& wondersPool = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(Board::getInstance().getUnusedWonders());
		debugWonders(wondersPool);

		auto displayAvailableWonders = [&](const std::vector<std::unique_ptr<Models::Wonder>>& wonders) {
			DisplayRequestEvent event;
			event.displayType = DisplayRequestEvent::Type::MESSAGE;
			event.context = "\n=== AVAILABLE WONDERS ===";
			notifier.notifyDisplayRequested(event);
			for (size_t i = 0; i < wonders.size(); ++i) {
				event.context = "\n[" + std::to_string(i) + "] ";
				notifier.notifyDisplayRequested(event);
				notifier.notifyDisplayWonderInfo(*wonders[i]);
			}
			event.context = "=========================";
			notifier.notifyDisplayRequested(event);
			};

		for (size_t sel = 0; sel < 4 && !wondersPool.empty(); ++sel) {
			size_t idx = 0;
			bool found = false;
			for (; idx < wondersPool.size(); ++idx) {
				if (!wondersPool[idx]) continue;
				if (dynamic_cast<Models::Wonder*>(wondersPool[idx].get())) { found = true; break; }
			}
			if (!found) break;
			std::unique_ptr<Models::Card> cardPtr = std::move(wondersPool[idx]);
			wondersPool.erase(wondersPool.begin() + idx);
			Models::Wonder* raw = static_cast<Models::Wonder*>(cardPtr.release());
			availableWonders.emplace_back(raw);
		}

		DisplayRequestEvent headerEvent;
		headerEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
		headerEvent.context = "\n+-----------------------------------+\n|   WONDER SELECTION - ROUND 1     |\n+-----------------------------------+";
		notifier.notifyDisplayRequested(headerEvent);
		displayAvailableWonders(availableWonders);

		auto draftWonders = [&](bool startWithP1) {
			std::vector<bool> playerOrder;
			if (startWithP1) {
				playerOrder = { true, false, false, true };
			}
			else {
				playerOrder = { false, true, true, false };
			}

			for (size_t i = 0; i < playerOrder.size(); ++i) {
				if (availableWonders.empty()) break;

				bool isPlayer1 = playerOrder[i];
				IPlayerDecisionMaker* decisionMaker = isPlayer1 ? p1DecisionsPtr : p2DecisionsPtr;
				std::shared_ptr<Core::Player> currentPlayer = isPlayer1 ? p1 : p2;

				std::string playerName = currentPlayer->m_player ? currentPlayer->m_player->getPlayerUsername() : (isPlayer1 ? "Player 1" : "Player 2");

				if (availableWonders.size() == 1) {
					DisplayRequestEvent event;
					event.displayType = DisplayRequestEvent::Type::MESSAGE;
					event.context = "\n> " + playerName +
						" automatically receives the final wonder: " + availableWonders[0]->getName();
					notifier.notifyDisplayRequested(event);
					currentPlayer->chooseWonder(availableWonders, 0);
					break;
				}

				DisplayRequestEvent promptEvent;
				promptEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
				promptEvent.context = "\n> " + playerName +
					", choose your wonder (0-" + std::to_string(availableWonders.size() - 1) + "): ";
				notifier.notifyDisplayRequested(promptEvent);

				std::vector<size_t> wonderIndices;
				for (size_t j = 0, end = availableWonders.size(); j < end; ++j) wonderIndices.push_back(j);

				size_t choice = 0;
				if (auto* mctsDM = dynamic_cast<MCTSDecisionMaker*>(decisionMaker)) {
					AIConfig cfg(mctsDM->getPlaystyle());
					const auto w = cfg.getWeights();

					auto scoreWonder = [&](const Models::Wonder& wonder) -> double {
						double score = 0.0;
						score += static_cast<double>(wonder.getVictoryPoints()) * w.victoryPointValue * w.wonderVPBonus;
						score += static_cast<double>(wonder.getShieldPoints()) * w.militaryPriority * w.wonderMilitaryBonus;
						if (wonder.getResourceProduction() != Models::ResourceType::NO_RESOURCE) {
							score += 1.0 * w.resourceValue;
						}
						for (const auto& [res, amt] : wonder.getResourceCost()) {
							(void)res;
							score -= static_cast<double>(amt) * w.resourceValue * 0.5;
						}
						// Light parsing of on-play action labels for extra signal.
						for (const auto& act : wonder.getOnPlayActions()) {
							const std::string& label = act.second;
							if (label.find("takeNewCard") != std::string::npos) score += 1.0 * w.economyPriority;
							if (label.find("drawToken") != std::string::npos) score += 1.0 * w.sciencePriority;
							if (label.find("discard") != std::string::npos) score += 1.0 * w.opponentDenial;
							if (label.find("playAnotherTurn") != std::string::npos) score += 1.0 * w.wonderEconomyBonus;
						}
						return score;
					};

					double bestScore = -1e18;
					for (size_t j = 0; j < availableWonders.size(); ++j) {
						if (!availableWonders[j]) continue;
						double s = scoreWonder(*availableWonders[j]);
						if (s > bestScore) {
							bestScore = s;
							choice = j;
						}
					}
				}
				else {
					choice = decisionMaker->selectCard(wonderIndices);
				}

				currentPlayer->chooseWonder(availableWonders, static_cast<uint8_t>(choice));

				if (!availableWonders.empty()) {
					displayAvailableWonders(availableWonders);
				}
			}
			};

		draftWonders(player1Starts);

		for (size_t sel = 0; sel < 4 && !wondersPool.empty(); ++sel) {
			size_t idx = 0;
			bool found = false;
			for (; idx < wondersPool.size(); ++idx) {
				if (!wondersPool[idx]) continue;
				if (dynamic_cast<Models::Wonder*>(wondersPool[idx].get())) { found = true; break; }
			}
			if (!found) break;
			std::unique_ptr<Models::Card> cardPtr = std::move(wondersPool[idx]);
			wondersPool.erase(wondersPool.begin() + idx);
			Models::Wonder* raw = static_cast<Models::Wonder*>(cardPtr.release());
			availableWonders.emplace_back(raw);
		}

		DisplayRequestEvent round2Event;
		round2Event.displayType = DisplayRequestEvent::Type::MESSAGE;
		round2Event.context = "\n+-----------------------------------+\n|   WONDER SELECTION - ROUND 2     |\n+-----------------------------------+";
		notifier.notifyDisplayRequested(round2Event);
		displayAvailableWonders(availableWonders);

		draftWonders(!player1Starts);

		DisplayRequestEvent completeEvent;
		completeEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
		completeEvent.context = "\n+-----------------------------------+\n|   WONDER SELECTION COMPLETE!     |\n+-----------------------------------+\n";
		notifier.notifyDisplayRequested(completeEvent);
	}
	void Game::debugWonders(const std::vector<std::unique_ptr<Models::Card>>& pool) {
		auto& notifier = GameState::getInstance().getEventNotifier();
		DisplayRequestEvent event;
		event.displayType = DisplayRequestEvent::Type::MESSAGE;
		event.context = "WondersPool";
		notifier.notifyDisplayRequested(event);
		for (const auto& uptr : pool) {
			if (!uptr) continue;
			const auto* w = dynamic_cast<const Models::Wonder*>(uptr.get());
			if (!w) continue;
			event.context = "- Name: " + std::string(w->getName());
			notifier.notifyDisplayRequested(event);
			event.context = "  Victory Points: " + std::to_string(w->getVictoryPoints());
			notifier.notifyDisplayRequested(event);
			event.context = "  Color: " + Models::ColorTypeToString(w->getColor());
			notifier.notifyDisplayRequested(event);
			event.context = "  Caption: " + std::string(w->getCaption());
			notifier.notifyDisplayRequested(event);
			const auto& cost = w->getResourceCost();
			event.context = "  Cost: ";
			if (cost.empty()) {
				event.context += "-";
			}
			else {
				bool first = true;
				for (const auto& kv : cost) {
					if (!first) event.context += ", ";
					first = false;
					event.context += Models::ResourceTypeToString(kv.first) + ":" + std::to_string(kv.second);
				}
			}
			notifier.notifyDisplayRequested(event);
			event.context = "  Shield Points: " + std::to_string(w->getShieldPoints());
			notifier.notifyDisplayRequested(event);
			event.context = "  Resource Production: ";
			if (w->getResourceProduction() == Models::ResourceType::NO_RESOURCE) {
				event.context += "-";
			}
			else {
				event.context += Models::ResourceTypeToString(w->getResourceProduction());
			}
			notifier.notifyDisplayRequested(event);
			const auto& actions = w->getOnPlayActions();
			event.context = "  On-Play Actions: ";
			if (actions.empty()) {
				event.context += "-";
			}
			else {
				bool firstA = true;
				for (const auto& p : actions) {
					if (!firstA) event.context += ", ";
					firstA = false;
					event.context += p.second;
				}
			}
			notifier.notifyDisplayRequested(event);
		}
	}
	void Game::awardMilitaryTokenIfPresent(Player& receiver, Player& opponent) {
		auto& board = Board::getInstance();
		auto& notifier = GameState::getInstance().getEventNotifier();
		int pos = board.getPawnPos();
		for (int p : kMilitaryTokenPositions) {
			if (p == pos) {
				auto& military = const_cast<std::vector<std::unique_ptr<Models::Token>>&>(board.getMilitaryTokens());
				if (!military.empty()) {
					std::unique_ptr<Models::Token> t = std::move(military.back());
					military.pop_back();
					if (t) {
						std::string desc = t->getDescription();
						uint8_t coinsToLose = 0;

						if (desc.find("loses 2 coins") != std::string::npos) {
							coinsToLose = 2;
						}
						else if (desc.find("loses 5 coins") != std::string::npos) {
							coinsToLose = 5;
						}

						if (coinsToLose > 0 && opponent.m_player) {
							auto currentCoins = opponent.m_player->getRemainingCoins();
							uint32_t totalCoins = opponent.m_player->totalCoins(currentCoins);

							if (totalCoins >= coinsToLose) {
								totalCoins -= coinsToLose;
							}
							else {
								totalCoins = 0;
							}

							uint8_t ones = totalCoins % 3;
							uint8_t threes = (totalCoins / 3) % 2;
							uint8_t sixes = totalCoins / 6;

							opponent.m_player->setRemainingCoins({ ones, threes, sixes });

							DisplayRequestEvent event;
							event.displayType = DisplayRequestEvent::Type::MESSAGE;
							event.context = std::string(opponent.m_player->getPlayerUsername()) + " loses " +
								std::to_string(coinsToLose) + " coins from military token!";
							notifier.notifyDisplayRequested(event);
						}

						if (receiver.m_player) receiver.m_player->addToken(std::move(t));
					}
				}
				break;
			}
		}
	}
	void Game::playAllPhases(Player& p1, Player& p2,
		IPlayerDecisionMaker& p1Decisions,
		IPlayerDecisionMaker& p2Decisions,
		std::optional<std::reference_wrapper<TrainingLogger>> logger)
	{
		GameState& gameState = GameState::getInstance();
		auto& notifier = gameState.getEventNotifier();
		auto& board = Board::getInstance();

		int currentPhase = 1;
		int nrOfRounds = 1;

		std::random_device rd;
		std::mt19937 gen(rd());
		bool playerOneTurn = std::uniform_int_distribution<>(0, 1)(gen) == 0;

		while (currentPhase <= 3) {

			const std::vector<std::shared_ptr<Node>>* nodes = nullptr;
			std::string phaseName;

			if (currentPhase == 1) {
				nodes = &board.getAge1Nodes();
				phaseName = "PHASE I";
			}
			else if (currentPhase == 2) {
				nodes = &board.getAge2Nodes();
				phaseName = "PHASE II";
			}
			else {
				nodes = &board.getAge3Nodes();
				phaseName = "PHASE III";
			}

			DisplayRequestEvent phaseEvent;
			phaseEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
			phaseEvent.context = "=== Starting " + phaseName + " ===";
			notifier.notifyDisplayRequested(phaseEvent);

			bool phaseComplete = false;
			while (!phaseComplete) {
				std::vector<size_t> availableIndex;
				availableIndex.reserve(nodes->size());

				for (size_t i = 0; i < nodes->size(); ++i) {
					const auto& node = (*nodes)[i];
					if (!node) continue;
					auto card = node->getCard();
					if (card.has_value() && node->isAvailable() && card->get().isAvailable()) {
						availableIndex.push_back(i);
					}
				}

				if (availableIndex.empty()) {
					phaseComplete = true;
					break;
				}

				DisplayRequestEvent availEvent;
				availEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
				availEvent.context = phaseName + ": " + std::to_string(availableIndex.size()) + " cards available";
				notifier.notifyDisplayRequested(availEvent);

				for (size_t k = 0; k < availableIndex.size(); ++k) {
					size_t index = availableIndex[k];
					const auto& node = (*nodes)[index];
					auto card = node->getCard();
					DisplayRequestEvent cardEvent;
					cardEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
					cardEvent.context = "\n[" + std::to_string(k) + "] ";
					notifier.notifyDisplayRequested(cardEvent);
					if (card.has_value()) {
						notifier.notifyDisplayCardInfo(card->get());
						if (auto ageCard = dynamic_cast<const Models::AgeCard*>(&card->get())) {
							if (ageCard->getScientificSymbols().has_value()) {
								cardEvent.context = " Science: " + Models::ScientificSymbolTypeToString(ageCard->getScientificSymbols().value());
								notifier.notifyDisplayRequested(cardEvent);
							}
						}
					}
				}

				Player& cur = playerOneTurn ? p1 : p2;
				Player& opp = playerOneTurn ? p2 : p1;

				auto& gs = GameState::getInstance();
				std::shared_ptr<Player> curPtr = playerOneTurn ? gs.GetPlayer1() : gs.GetPlayer2();
				Core::setCurrentPlayer(curPtr);
				// Ensure GameState reflects the current loop turn before any AI reads it (e.g. MCTS capture).
				gs.setCurrentPhase(currentPhase, nrOfRounds, playerOneTurn);

				IPlayerDecisionMaker& curDecisionMaker = playerOneTurn ? p1Decisions : p2Decisions;

				std::string currentPlayerName = cur.m_player ? cur.m_player->getPlayerUsername() : "Unknown";

				displayPlayerResources(cur, currentPlayerName);
				DisplayRequestEvent promptEvent;
				promptEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
				promptEvent.context = currentPlayerName + " choose index (0-" + std::to_string(availableIndex.size() - 1) + "): ";
				notifier.notifyDisplayRequested(promptEvent);

				size_t chosenNodeIndex = 0;
				int action = 0;
				std::optional<size_t> mctsWonderOwnedIndex = std::nullopt;
				bool isMctsDriven = false;

				if (auto* mctsDM = dynamic_cast<MCTSDecisionMaker*>(&curDecisionMaker)) {
					isMctsDriven = true;
					MCTSAction mctsAction = mctsDM->selectTurnAction();
					chosenNodeIndex = mctsAction.cardNodeIndex;
					action = mctsAction.actionType;
					mctsWonderOwnedIndex = mctsAction.wonderIndex;

					if (std::find(availableIndex.begin(), availableIndex.end(), chosenNodeIndex) == availableIndex.end()) {
						chosenNodeIndex = availableIndex.front();
					}
				}
				else if (auto* aiDM = dynamic_cast<AIDecisionMaker*>(&curDecisionMaker)) {
					isMctsDriven = true;
					MCTSAction mctsAction = aiDM->selectTurnAction();
					chosenNodeIndex = mctsAction.cardNodeIndex;
					action = mctsAction.actionType;
					mctsWonderOwnedIndex = mctsAction.wonderIndex;

					if (std::find(availableIndex.begin(), availableIndex.end(), chosenNodeIndex) == availableIndex.end()) {
						chosenNodeIndex = availableIndex.front();
					}
				}
				else {
					size_t choice = curDecisionMaker.selectCard(availableIndex);
					if (choice >= availableIndex.size()) choice = 0;
					chosenNodeIndex = availableIndex[choice];
				}

				std::unique_ptr<Models::Card> cardPtr = (*nodes)[chosenNodeIndex]->releaseCard();
				if (!cardPtr) {
					DisplayRequestEvent errEvent;
					errEvent.displayType = DisplayRequestEvent::Type::ERROR;
					errEvent.context = "Node releaseCard failed.";
					notifier.notifyDisplayRequested(errEvent);
					continue;
				}

				std::string cardName = cardPtr->getName();
				displayCardDetails(*cardPtr);
				uint8_t shields = getShieldPointsFromCard(*cardPtr, std::cref(cur));

				std::vector<std::string> effects;
				if (auto ageCard = dynamic_cast<Models::AgeCard*>(cardPtr.get())) {
					const auto& actions = ageCard->getOnPlayActions();
					for (const auto& actionPair : actions) {
						effects.push_back(actionPair.second);
					}
				}

				std::optional<Models::ScientificSymbolType> symbolToCheck;
				bool potentialPair = false;
				if (auto ageCard = dynamic_cast<Models::AgeCard*>(cardPtr.get())) {
					symbolToCheck = ageCard->getScientificSymbols();
					if (symbolToCheck.has_value()) potentialPair = true;
				}

				DisplayRequestEvent choiceEvent;
				choiceEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
				choiceEvent.context = " You chose " + std::string(cardPtr->getName()) + " . Action: [0]=build, [1]=sell, [2]=wonder";
				notifier.notifyDisplayRequested(choiceEvent);

				bool actionSucceeded = false;
				bool cancelled = false;
				bool mctsActionUsed = false;
				while (!actionSucceeded && cardPtr) {
					if (!isMctsDriven) {
						action = curDecisionMaker.selectCardAction();
					}
					else {
						// Try the MCTS-chosen action once; if it fails (e.g. affordability), fall back to SELL.
						if (!mctsActionUsed) {
							mctsActionUsed = true;
						}
						else {
							action = 1;
						}
					}

					switch (action) {
					case 0: {
						if (!cur.canAffordCard(*cardPtr, opp.m_player)) {
							DisplayRequestEvent errEvent;
							errEvent.displayType = DisplayRequestEvent::Type::ERROR;
							errEvent.context = "Cannot afford this card. Choose another action: [0]=build, [1]=sell, [2]=wonder";
							notifier.notifyDisplayRequested(errEvent);
							break;
						}
						try {
							cur.playCardBuilding(cardPtr, opp.m_player);
							actionSucceeded = true;
						}
						catch (const std::exception& ex) {
							DisplayRequestEvent errEvent;
							errEvent.displayType = DisplayRequestEvent::Type::ERROR;
							errEvent.context = "Build failed: " + std::string(ex.what()) + ". Choose another action: [0]=build, [1]=sell, [2]=wonder";
							notifier.notifyDisplayRequested(errEvent);
						}
						break;
					}
					case 1: {
						auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
						cur.sellCard(cardPtr, discarded);
						actionSucceeded = true;
						break;
					}
					case 2: {
						if (Models::Wonder::getWondersBuilt() >= Models::Wonder::MaxWonders) {
							DisplayRequestEvent errEvent;
							errEvent.displayType = DisplayRequestEvent::Type::ERROR;
							errEvent.context = "Maximum wonders already built. Choose another action: [0]=build, [1]=sell, [2]=wonder";
							notifier.notifyDisplayRequested(errEvent);
							break;
						}

						auto& owned = cur.m_player->getOwnedWonders();
						std::vector<size_t> candidates;
						for (size_t i = 0; i < owned.size(); ++i) {
							if (owned[i] && !owned[i]->IsConstructed()) candidates.push_back(i);
						}
						if (candidates.empty()) {
							DisplayRequestEvent errEvent;
							errEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
							errEvent.context = "No unbuilt wonders available. Choose another action: [0]=build, [1]=sell, [2]=wonder";
							notifier.notifyDisplayRequested(errEvent);
							break;
						}

						auto& notifier2 = GameState::getInstance().getEventNotifier();
						DisplayRequestEvent headerEvent;
						headerEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
						headerEvent.context = "Choose wonder to construct:";
						notifier2.notifyDisplayRequested(headerEvent);

						for (size_t i = 0; i < candidates.size(); ++i) {
							DisplayRequestEvent cardEvent;
							cardEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
							cardEvent.context = "[" + std::to_string(i) + "] " + owned[candidates[i]]->getName();
							notifier2.notifyDisplayRequested(cardEvent);
						}

						size_t wchoice = 0;
						if (isMctsDriven && mctsWonderOwnedIndex.has_value()) {
							size_t desiredOwnedIdx = mctsWonderOwnedIndex.value();
							auto it = std::find(candidates.begin(), candidates.end(), desiredOwnedIdx);
							if (it != candidates.end()) {
								wchoice = static_cast<size_t>(std::distance(candidates.begin(), it));
							}
							else {
								wchoice = 0;
							}
						}
						else {
							wchoice = curDecisionMaker.selectWonder(candidates);
							if (wchoice >= candidates.size()) wchoice = 0;
						}
						std::unique_ptr<Models::Wonder>& chosenWonderPtr = owned[candidates[wchoice]];

						if (!cur.canAffordWonder(chosenWonderPtr, opp.m_player)) {
							DisplayRequestEvent errEvent;
							errEvent.displayType = DisplayRequestEvent::Type::ERROR;
							errEvent.context = "Cannot afford this wonder. Choose another action: [0]=build, [1]=sell, [2]=wonder";
							notifier2.notifyDisplayRequested(errEvent);
							break;
						}

						std::vector<Models::Token> discardedTokens;
						auto& discardedCards = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
						cur.playCardWonder(chosenWonderPtr, cardPtr, opp.m_player, discardedTokens, discardedCards);
						actionSucceeded = true;
						break;
					}
					default: {
						DisplayRequestEvent errEvent;
						errEvent.displayType = DisplayRequestEvent::Type::ERROR;
						errEvent.context = "Invalid action. Choose: [0]=build, [1]=sell, [2]=wonder";
						notifier.notifyDisplayRequested(errEvent);
						break;
					}
					}

					if (!cardPtr && action == 0 && potentialPair) {
						int realCount = 0;
						auto targetSymbol = symbolToCheck.value();
						const auto& inventory = cur.m_player->getOwnedCards();

						for (const auto& ownedCardPtr : inventory) {
							if (auto ageCard = dynamic_cast<Models::AgeCard*>(ownedCardPtr.get())) {
								auto sym = ageCard->getScientificSymbols();
								if (sym.has_value() && sym.value() == targetSymbol) {
									realCount++;
								}
							}
						}

						if (realCount == 2) {
							DisplayRequestEvent pairEvent;
							pairEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
							pairEvent.context = ">>> PAIR FOUND! Choose a token! <<<";
							notifier.notifyDisplayRequested(pairEvent);

							cur.chooseProgressTokenFromBoard(std::ref(curDecisionMaker));
						}
					}
				}

				if (cancelled) {
					continue;
				}

				if ((*nodes)[chosenNodeIndex]->getCard().has_value()) {
					continue;
				}

				if (auto takenNode = (*nodes)[chosenNodeIndex]) {
					auto checkParent = [](const std::shared_ptr<Node>& p) {
						if (p) {
							auto c1 = p->getChild1();
							auto c2 = p->getChild2();
							bool empty1 = (!c1 || !c1->getCard().has_value());
							bool empty2 = (!c2 || !c2->getCard().has_value());
							if (empty1 && empty2) {
								if (auto card = p->getCard(); card.has_value()) {
									card->get().setIsAvailable(true);
									card->get().setIsVisible(true);
								}
							}
						}
						};
					checkParent(takenNode->getParent1());
					checkParent(takenNode->getParent2());
				}

				gameState.setCurrentPhase(currentPhase, nrOfRounds, playerOneTurn);

				gameState.recordAction(
					playerOneTurn ? p1.m_player->getPlayerUsername() : p2.m_player->getPlayerUsername(),
					std::to_string(action),
					cardName,
					effects
				);

				if (logger) {
					MCTSGameState state = MCTS::captureGameState(currentPhase, playerOneTurn);
					MCTSAction mctsAction;
					mctsAction.cardNodeIndex = chosenNodeIndex;
					mctsAction.actionType = action;
					mctsAction.cardName = cardName;
					TurnRecord turn = createTurnRecord(state, mctsAction, nrOfRounds, 0.5, 0.5);
					if (logger.has_value()) {
						logger->get().logTurn(turn);
					}
				}

				gameState.saveGameState("");

				DisplayRequestEvent saveEvent;
				saveEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
				saveEvent.context = "[AUTO-SAVE] " + phaseName + " Round " + std::to_string(nrOfRounds) + " saved.";
				notifier.notifyDisplayRequested(saveEvent);

				if (shields > 0) {
					Game::movePawn(playerOneTurn ? (int)shields : -(int)shields);
					awardMilitaryTokenIfPresent(cur, opp);
					int win = checkImmediateMilitaryVictory();
					if (win != -1) {
						gameState.setVictory(win, "Military Supremacy", 0, 0);
						gameState.saveGameState("");
						announceVictory(win, "Military Supremacy", p1, p2);
						g_last_active_was_player_one = !playerOneTurn;
						return;
					}


				}
				int sv = checkImmediateScientificVictory(p1, p2);
				if (sv != -1) {
					gameState.setVictory(sv, "Scientific Supremacy", 0, 0);
					gameState.saveGameState("");
					announceVictory(sv, "Scientific Supremacy", p1, p2);
					g_last_active_was_player_one = !playerOneTurn;
					return;
				}

				displayPlayerHands(p1, p2);
				displayTurnStatus(p1, p2);

				++nrOfRounds;
				playerOneTurn = !playerOneTurn;



			}
			DisplayRequestEvent completeEvent;
			completeEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
			completeEvent.context = phaseName + " completed.";
			notifier.notifyDisplayRequested(completeEvent);

			currentPhase++;
			nrOfRounds = 1;


		}
		g_last_active_was_player_one = !playerOneTurn;

		DisplayRequestEvent allPhaseEvent;
		allPhaseEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
		allPhaseEvent.context = "=== All Phases Completed ===";
		notifier.notifyDisplayRequested(allPhaseEvent);
	}

	void Game::movePawn(int steps) {
		auto& board = Core::Board::getInstance();
		int previous = static_cast<int>(board.getPawnPos());
		int next = std::clamp(previous + steps, 0, 18);
		if (next == previous) {
			return;
		}

		board.setPawnPos(static_cast<uint8_t>(next));

		Core::PawnEvent evt;
		evt.previousPosition = previous;
		evt.newPosition = next;
		evt.steps = steps;
		evt.reason = "Military track updated";
		Core::Game::getNotifier().notifyPawnMoved(evt);
	}
	void Game::displayPlayerHands(const Player& p1, const Player& p2) {
		auto& notifier = GameState::getInstance().getEventNotifier();
		DisplayRequestEvent event;
		event.displayType = DisplayRequestEvent::Type::MESSAGE;
		event.context = "\n========== PLAYER HANDS ==========";
		notifier.notifyDisplayRequested(event);

		event.context = "\n--- PLAYER 1: " + std::string(p1.m_player ? p1.m_player->getPlayerUsername() : "Unknown") + " ---";
		notifier.notifyDisplayRequested(event);

		if (p1.m_player) {
			auto coins = p1.m_player->getRemainingCoins();
			uint32_t totalCoins = p1.m_player->totalCoins(coins);
			event.context = "Coins: " + std::to_string(totalCoins) + " (1x" + std::to_string(std::get<0>(coins)) +
				" + 3x" + std::to_string(std::get<1>(coins)) + " + 6x" + std::to_string(std::get<2>(coins)) + ")";
			notifier.notifyDisplayRequested(event);

			event.context = "\nOwned Cards (" + std::to_string(p1.m_player->getOwnedCards().size()) + "):";
			notifier.notifyDisplayRequested(event);
			for (const auto& card : p1.m_player->getOwnedCards()) {
				if (card) {
					event.context = "  - ";
					notifier.notifyDisplayRequested(event);
					GameState::getInstance().getEventNotifier().notifyDisplayCardInfo(*card);
				}
			}

			event.context = "\nWonders (" + std::to_string(p1.m_player->getOwnedWonders().size()) + "):";
			notifier.notifyDisplayRequested(event);
			for (const auto& wonder : p1.m_player->getOwnedWonders()) {
				if (wonder) {
					event.context = "  - " + std::string(wonder->getName()) +
						(wonder->IsConstructed() ? " [CONSTRUCTED]" : " [NOT BUILT]");
					notifier.notifyDisplayRequested(event);
				}
			}

			event.context = "\nTokens (" + std::to_string(p1.m_player->getOwnedTokens().size()) + "):";
			notifier.notifyDisplayRequested(event);
			for (const auto& token : p1.m_player->getOwnedTokens()) {
				if (token) {
					event.context = "  - " + std::string(token->getName());
					notifier.notifyDisplayRequested(event);
				}
			}
		}

		event.context = "\n--- PLAYER 2: " + std::string(p2.m_player ? p2.m_player->getPlayerUsername() : "Unknown") + " ---";
		notifier.notifyDisplayRequested(event);

		if (p2.m_player) {
			auto coins = p2.m_player->getRemainingCoins();
			uint32_t totalCoins = p2.m_player->totalCoins(coins);
			event.context = "Coins: " + std::to_string(totalCoins) + " (1x" + std::to_string(std::get<0>(coins)) +
				" + 3x" + std::to_string(std::get<1>(coins)) + " +  6x" + std::to_string(std::get<2>(coins)) + ")";
			notifier.notifyDisplayRequested(event);

			event.context = "\nOwned Cards (" + std::to_string(p2.m_player->getOwnedCards().size()) + "):";
			notifier.notifyDisplayRequested(event);
			for (const auto& card : p2.m_player->getOwnedCards()) {
				if (card) {
					event.context = "  - ";
					notifier.notifyDisplayRequested(event);
					GameState::getInstance().getEventNotifier().notifyDisplayCardInfo(*card);
				}
			}

			event.context = "\nWonders (" + std::to_string(p2.m_player->getOwnedWonders().size()) + "):";
			notifier.notifyDisplayRequested(event);
			for (const auto& wonder : p2.m_player->getOwnedWonders()) {
				if (wonder) {
					event.context = "  - " + std::string(wonder->getName()) +
						(wonder->IsConstructed() ? " [CONSTRUCTED]" : " [NOT BUILT]");
					notifier.notifyDisplayRequested(event);
				}
			}

			event.context = "\nTokens (" + std::to_string(p2.m_player->getOwnedTokens().size()) + "):";
			notifier.notifyDisplayRequested(event);
			for (const auto& token : p2.m_player->getOwnedTokens()) {
				if (token) {
					event.context = "  - " + std::string(token->getName());
					notifier.notifyDisplayRequested(event);
				}
			}
		}

		event.context = "\n==================================\n";
		notifier.notifyDisplayRequested(event);
	}
	void Game::displayTurnStatus(const Player& p1, const Player& p2) {
		auto& notifier = GameState::getInstance().getEventNotifier();
		auto& board = Board::getInstance();
		int pawnPos = board.getPawnPos();
		DisplayRequestEvent event;
		event.displayType = DisplayRequestEvent::Type::MESSAGE;

		event.context = "\n=== CURRENT GAME STATUS ===";
		notifier.notifyDisplayRequested(event);

		event.context = "Military Track:";
		notifier.notifyDisplayRequested(event);

		std::string track = "P1 [";
		for (int i = 0; i <= 18; ++i) {
			if (i == pawnPos) {
				track += "*";
			}
			else if (i == 9) {
				track += "|";
			}
			else {
				track += "-";
			}
		}
		track += "] P2";
		event.context = track;
		notifier.notifyDisplayRequested(event);

		event.context = "Positions: 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18";
		notifier.notifyDisplayRequested(event);

		event.context = "Position: " + std::to_string(pawnPos);
		if (pawnPos < 9) event.context += " (P2 winning by " + std::to_string(9 - pawnPos) + ")";
		else if (pawnPos > 9) event.context += " (P1 winning by " + std::to_string(pawnPos - 9) + ")";
		else event.context += " (Neutral)";
		notifier.notifyDisplayRequested(event);

		auto getMilitaryVP = [](int pawnPos) -> uint32_t {
			if (pawnPos == 9) return 0;

			if (pawnPos > 9) {
				int distance = pawnPos - 9;
				if (distance >= 2 && distance <= 3) return 2;
				if (distance >= 4 && distance <= 6) return 5;
				if (distance >= 7 && distance <= 9) return 10;
			}
			else {
				int distance = 9 - pawnPos;
				if (distance >= 1 && distance <= 2) return 2;
				if (distance >= 3 && distance <= 5) return 5;
				if (distance >= 6 && distance <= 8) return 10;
			}
			return 0;
			};

		uint32_t militaryVP1 = (pawnPos > 9) ? getMilitaryVP(pawnPos) : 0;
		uint32_t militaryVP2 = (pawnPos < 9) ? getMilitaryVP(pawnPos) : 0;

		auto calculateScore = [&](const Player& p, uint32_t militaryVP) -> uint32_t {
			if (!p.m_player) return 0;
			const auto& pts = p.m_player->getPoints();
			uint32_t total = militaryVP +
				static_cast<uint32_t>(pts.m_buildingVictoryPoints) +
				static_cast<uint32_t>(pts.m_wonderVictoryPoints) +
				static_cast<uint32_t>(pts.m_progressVictoryPoints);
			total += p.m_player->totalCoins(p.m_player->getRemainingCoins()) / 3;
			return total;
			};

		uint32_t score1 = calculateScore(p1, militaryVP1);
		uint32_t score2 = calculateScore(p2, militaryVP2);

		event.context = "\nPLAYER 1: " + std::string(p1.m_player ? p1.m_player->getPlayerUsername() : "Unknown");
		notifier.notifyDisplayRequested(event);

		if (p1.m_player) {
			const auto& pts1 = p1.m_player->getPoints();
			event.context = "  Total Score: " + std::to_string(score1) + " VP";
			notifier.notifyDisplayRequested(event);

			event.context = "  Military: " + std::to_string(militaryVP1) + " VP";
			notifier.notifyDisplayRequested(event);

			event.context = "  Buildings: " + std::to_string(pts1.m_buildingVictoryPoints) + " VP";
			notifier.notifyDisplayRequested(event);

			event.context = "  Wonders: " + std::to_string(pts1.m_wonderVictoryPoints) + " VP";
			notifier.notifyDisplayRequested(event);

			event.context = "  Progress: " + std::to_string(pts1.m_progressVictoryPoints) + " VP";
			notifier.notifyDisplayRequested(event);

			uint32_t coinVP = p1.m_player->totalCoins(p1.m_player->getRemainingCoins()) / 3;
			event.context = "  Coins: " + std::to_string(coinVP) + " VP";
			notifier.notifyDisplayRequested(event);
		}

		event.context = "\nPLAYER 2: " + std::string(p2.m_player ? p2.m_player->getPlayerUsername() : "Unknown");
		notifier.notifyDisplayRequested(event);

		if (p2.m_player) {
			const auto& pts2 = p2.m_player->getPoints();
			event.context = "  Total Score: " + std::to_string(score2) + " VP";
			notifier.notifyDisplayRequested(event);

			event.context = "  Military: " + std::to_string(militaryVP2) + " VP";
			notifier.notifyDisplayRequested(event);

			event.context = "  Buildings: " + std::to_string(pts2.m_buildingVictoryPoints) + " VP";
			notifier.notifyDisplayRequested(event);

			event.context = "  Wonders: " + std::to_string(pts2.m_wonderVictoryPoints) + " VP";
			notifier.notifyDisplayRequested(event);

			event.context = "  Progress: " + std::to_string(pts2.m_progressVictoryPoints) + " VP";
			notifier.notifyDisplayRequested(event);

			uint32_t coinVP = p2.m_player->totalCoins(p2.m_player->getRemainingCoins()) / 3;
			event.context = "  Coins: " + std::to_string(coinVP) + " VP";
			notifier.notifyDisplayRequested(event);
		}
		event.context = "\n";
		notifier.notifyDisplayRequested(event);
	}
	void Game::announceVictory(int winner, const std::string& victoryType, const Player& p1, const Player& p2) {
		auto& notifier = GameState::getInstance().getEventNotifier();
		DisplayRequestEvent event;
		event.displayType = DisplayRequestEvent::Type::MESSAGE;

		event.context = "\n=== GAME OVER ===";
		notifier.notifyDisplayRequested(event);

		std::string winnerName;
		if (winner == 0 && p1.m_player) {
			winnerName = p1.m_player->getPlayerUsername();
		}
		else if (winner == 1 && p2.m_player) {
			winnerName = p2.m_player->getPlayerUsername();
		}
		else if (winner == 2) {
			winnerName = "TIE";
		}

		event.context = "Victory Type: " + victoryType;
		notifier.notifyDisplayRequested(event);

		event.context = "Winner: " + winnerName;
		notifier.notifyDisplayRequested(event);

		event.context = "";
		notifier.notifyDisplayRequested(event);

		displayTurnStatus(p1, p2);
	}

	void Game::handleOpponentCardDiscard(Player& cardOwner, Player& discardingPlayer,
		Models::ColorType color,
		IPlayerDecisionMaker& decisionMaker) {
		auto& board = Board::getInstance();
		auto& notifier = GameState::getInstance().getEventNotifier();

		auto& owned = cardOwner.m_player->getOwnedCards();
		std::vector<size_t> candidates;

		for (size_t i = 0; i < owned.size(); ++i) {
			if (!owned[i]) continue;
			if (color == Models::ColorType::NO_COLOR || owned[i]->getColor() == color) {
				candidates.push_back(i);
			}
		}

		if (candidates.empty()) return;

		DisplayRequestEvent event;
		event.displayType = DisplayRequestEvent::Type::CARD_DISCARD_SELECTION_PROMPT;
		event.context = "Choose a card to discard (triggered by " +
			std::string(discardingPlayer.m_player->getPlayerUsername()) + "'s wonder):";

		for (size_t idx : candidates) {
			if (owned[idx]) {
				event.cards.push_back(std::ref(*owned[idx]));
			}
		}

		notifier.notifyDisplayRequested(event);

		size_t choice = decisionMaker.selectCardToDiscard(candidates);
		if (choice >= candidates.size()) choice = 0;

		size_t removeIdx = candidates[choice];
		auto moved = cardOwner.m_player->removeOwnedCardAt(removeIdx);

		if (moved) {
			auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(
				board.getDiscardedCards());
			discarded.push_back(std::move(moved));
		}
	}

	bool hasSavedGame(const std::string& filename) {
		return std::filesystem::exists(filename);
	}

	void Game::initGame() {
		GameState& gameState = GameState::getInstance();
		auto& notifier = gameState.getEventNotifier();
		Core::ConsoleReader reader;

		bool trainingMode = false;
		std::unique_ptr<IPlayerDecisionMaker> p1Decisions;
		std::unique_ptr<IPlayerDecisionMaker> p2Decisions;
		Core::Playstyle p1Playstyle = Core::Playstyle::BRITNEY;
		Core::Playstyle p2Playstyle = Core::Playstyle::BRITNEY;
		std::unique_ptr<TrainingLogger> logger;

		std::vector<int> existingSaves = GameStateSerializer::getAllSaveNumbers();

		if (!existingSaves.empty()) {
			DisplayRequestEvent savesEvent;
			savesEvent.displayType = DisplayRequestEvent::Type::AVAILABLE_SAVES;
			savesEvent.saveNumbers = existingSaves;
			notifier.notifyDisplayRequested(savesEvent);

			int choice = reader.selectSave(existingSaves);
			if (choice > 0 && std::find(existingSaves.begin(), existingSaves.end(), choice) != existingSaves.end()) {
				gameState.loadGameState("", choice);
				GameStateSerializer::setCurrentSaveNumber(choice);

				DisplayRequestEvent event;
				event.displayType = DisplayRequestEvent::Type::MESSAGE;
				event.context = "Game state loaded from save #" + std::to_string(choice);
				notifier.notifyDisplayRequested(event);

				auto p1Ptr = gameState.GetPlayer1();
				auto p2Ptr = gameState.GetPlayer2();

				p1Decisions = std::make_unique<Core::HumanDecisionMaker>();
				p2Decisions = std::make_unique<Core::HumanDecisionMaker>();

				if (logger) {
					playAllPhases(*p1Ptr, *p2Ptr, *p1Decisions, *p2Decisions, std::ref(*logger));
				} else {
					playAllPhases(*p1Ptr, *p2Ptr, *p1Decisions, *p2Decisions, std::nullopt);
				}

				return;
			}
		}

		DisplayRequestEvent gameModeEvent;
		gameModeEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
		gameModeEvent.context = "Current working directory: " + std::filesystem::current_path().string();
		notifier.notifyDisplayRequested(gameModeEvent);

		gameModeEvent.displayType = DisplayRequestEvent::Type::GAME_MODE_MENU;
		notifier.notifyDisplayRequested(gameModeEvent);

		int mode = reader.selectGameMode();
		std::string username;

		if (mode == 2) {
			DisplayRequestEvent msgEvent;
			msgEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
			msgEvent.context = "=== HUMAN VS AI MODE ===";
			notifier.notifyDisplayRequested(msgEvent);

			username = PlayerNameValidator::getValidatedName("Enter your username: ");
			gameState.GetPlayer1()->m_player = std::make_unique<Models::Player>(1, username);
			gameState.GetPlayer2()->m_player = std::make_unique<Models::Player>(2, "AI_Opponent");

			DisplayRequestEvent playstyleEvent;
			playstyleEvent.displayType = DisplayRequestEvent::Type::PLAYSTYLE_MENU;
			playstyleEvent.context = "AI";
			notifier.notifyDisplayRequested(playstyleEvent);

			int aiStyle = reader.selectPlaystyle();
			p2Playstyle = (aiStyle == 1) ? Core::Playstyle::BRITNEY : Core::Playstyle::SPEARS;
			p1Decisions = std::make_unique<Core::HumanDecisionMaker>();
			p2Decisions = std::make_unique<Core::MCTSDecisionMaker>(p2Playstyle, 1000, 1.414, 20);

			DisplayRequestEvent infoEvent;
			infoEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
			infoEvent.context = "You are Player 1";
			notifier.notifyDisplayRequested(infoEvent);

			infoEvent.context = "AI is Player 2 playing as: " + Core::playstyleToString(p2Playstyle);
			notifier.notifyDisplayRequested(infoEvent);

			gameState.setGameMode(mode, false);
			gameState.setPlayerPlaystyles(p1Playstyle, p2Playstyle);
		}
		else if (mode == 3) {
			DisplayRequestEvent msgEvent;
			msgEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
			msgEvent.context = "=== AI VS AI TRAINING MODE ===";
			notifier.notifyDisplayRequested(msgEvent);

			trainingMode = true;

			DisplayRequestEvent p1StyleEvent;
			p1StyleEvent.displayType = DisplayRequestEvent::Type::PLAYSTYLE_MENU;
			p1StyleEvent.context = "AI Player 1";
			notifier.notifyDisplayRequested(p1StyleEvent);

			int p1Style = reader.selectPlaystyle();

			DisplayRequestEvent p2StyleEvent;
			p2StyleEvent.displayType = DisplayRequestEvent::Type::PLAYSTYLE_MENU;
			p2StyleEvent.context = "AI Player 2";
			notifier.notifyDisplayRequested(p2StyleEvent);

			int p2Style = reader.selectPlaystyle();

			p1Playstyle = (p1Style == 1) ? Core::Playstyle::BRITNEY : Core::Playstyle::SPEARS;
			p2Playstyle = (p2Style == 1) ? Core::Playstyle::BRITNEY : Core::Playstyle::SPEARS;

			gameState.GetPlayer1()->m_player = std::make_unique<Models::Player>(1, "AI_P1");
			gameState.GetPlayer2()->m_player = std::make_unique<Models::Player>(2, "AI_P2");

			p1Decisions = std::make_unique<Core::MCTSDecisionMaker>(p1Playstyle, 1000, 1.414, 20);
			p2Decisions = std::make_unique<Core::MCTSDecisionMaker>(p2Playstyle, 1000, 1.414, 20);

			if (trainingMode) {
				logger = std::make_unique<TrainingLogger>();
			}

			DisplayRequestEvent infoEvent;
			infoEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
			infoEvent.context = "AI Player 1 is playing as: " + Core::playstyleToString(p1Playstyle);
			notifier.notifyDisplayRequested(infoEvent);

			infoEvent.context = "AI Player 2 is playing as: " + Core::playstyleToString(p2Playstyle);
			notifier.notifyDisplayRequested(infoEvent);

			infoEvent.context = "Training data will be saved after the game.";
			notifier.notifyDisplayRequested(infoEvent);

			gameState.setGameMode(mode, true);
			gameState.setPlayerPlaystyles(p1Playstyle, p2Playstyle);
		}
		else if (mode == 4) {
			DisplayRequestEvent msgEvent;
			msgEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
			msgEvent.context = "=== HUMAN WITH AI SUGGESTIONS ===";
			notifier.notifyDisplayRequested(msgEvent);

			username = PlayerNameValidator::getValidatedName("Enter Player 1 username: ");
			gameState.GetPlayer1()->m_player = std::make_unique<Models::Player>(1, username);

			username = PlayerNameValidator::getValidatedName("Enter Player 2 username: ");
			gameState.GetPlayer2()->m_player = std::make_unique<Models::Player>(2, username);

			DisplayRequestEvent p1StyleEvent;
			p1StyleEvent.displayType = DisplayRequestEvent::Type::PLAYSTYLE_MENU;
			p1StyleEvent.context = "Player 1";
			notifier.notifyDisplayRequested(p1StyleEvent);

			int p1Style = reader.selectPlaystyle();

			DisplayRequestEvent p2StyleEvent;
			p2StyleEvent.displayType = DisplayRequestEvent::Type::PLAYSTYLE_MENU;
			p2StyleEvent.context = "Player 2";
			notifier.notifyDisplayRequested(p2StyleEvent);

			int p2Style = reader.selectPlaystyle();

			p1Playstyle = (p1Style == 1) ? Core::Playstyle::BRITNEY : Core::Playstyle::SPEARS;
			p2Playstyle = (p2Style == 1) ? Core::Playstyle::BRITNEY : Core::Playstyle::SPEARS;

			p1Decisions = std::make_unique<Core::HumanAssistedDecisionMaker>(p1Playstyle, 500);
			p2Decisions = std::make_unique<Core::HumanAssistedDecisionMaker>(p2Playstyle, 500);

			DisplayRequestEvent infoEvent;
			infoEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
			infoEvent.context = "Player 1 gets " + Core::playstyleToString(p1Playstyle) + " suggestions";
			notifier.notifyDisplayRequested(infoEvent);

			infoEvent.context = "Player 2 gets " + Core::playstyleToString(p2Playstyle) + " suggestions";
			notifier.notifyDisplayRequested(infoEvent);

			gameState.setGameMode(mode, false);
			gameState.setPlayerPlaystyles(p1Playstyle, p2Playstyle);
		}
		else {
			DisplayRequestEvent msgEvent;
			msgEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
			msgEvent.context = "=== HUMAN VS HUMAN MODE ===";
			notifier.notifyDisplayRequested(msgEvent);

			username = PlayerNameValidator::getValidatedName("Enter Player 1 username: ");
			gameState.GetPlayer1()->m_player = std::make_unique<Models::Player>(1, username);

			username = PlayerNameValidator::getValidatedName("Enter Player 2 username: ");
			gameState.GetPlayer2()->m_player = std::make_unique<Models::Player>(2, username);

			p1Decisions = std::make_unique<Core::HumanDecisionMaker>();
			p2Decisions = std::make_unique<Core::HumanDecisionMaker>();

			gameState.setGameMode(mode, false);
			gameState.setPlayerPlaystyles(p1Playstyle, p2Playstyle);
		}

		DisplayRequestEvent prepEvent;
		prepEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
		prepEvent.context = "Starting preparation...";
		notifier.notifyDisplayRequested(prepEvent);

		preparation();

		DisplayRequestEvent wonderEvent;
		wonderEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
		wonderEvent.context = "=== Wonder Selection ===";
		notifier.notifyDisplayRequested(wonderEvent);

		auto p1Ptr = gameState.GetPlayer1();
		auto p2Ptr = gameState.GetPlayer2();
		if (p1Decisions && p2Decisions) {
			wonderSelection(p1Ptr, p2Ptr, std::ref(*p1Decisions), std::ref(*p2Decisions));
		} else {
			wonderSelection(p1Ptr, p2Ptr, std::nullopt, std::nullopt);
		}

		DisplayRequestEvent finishEvent;
		finishEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
		finishEvent.context = "Preparation finished.";
		notifier.notifyDisplayRequested(finishEvent);

		Board::getInstance().displayEntireBoard();

		gameState.saveGameState("");

		DisplayRequestEvent saveEvent;
		saveEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
		saveEvent.context = "[SAVE] Initial game state saved.";
		notifier.notifyDisplayRequested(saveEvent);

		DisplayRequestEvent startGameEvent;
		startGameEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
		startGameEvent.context = "=== GAME START ===";
		notifier.notifyDisplayRequested(startGameEvent);

		if (logger) {
			playAllPhases(*p1Ptr, *p2Ptr, *p1Decisions, *p2Decisions, std::ref(*logger));
		} else {
			playAllPhases(*p1Ptr, *p2Ptr, *p1Decisions, *p2Decisions, std::nullopt);
		}
	}
	
	void Game::updateTreeAfterPick(int age, int emptiedNodeIndex)
	{
		auto& board = Board::getInstance();
		const auto& nodes = (age == 1) ? board.getAge1Nodes() : (age == 2) ? board.getAge2Nodes() : board.getAge3Nodes();
		if (emptiedNodeIndex < 0 || static_cast<size_t>(emptiedNodeIndex) >= nodes.size()) return;
		auto emptiedNode = nodes[static_cast<size_t>(emptiedNodeIndex)];
		if (!emptiedNode) return;

		Core::TreeNodeEvent emptiedEvt;
		emptiedEvt.ageIndex = age;
		emptiedEvt.nodeIndex = emptiedNodeIndex;
		emptiedEvt.cardName = "";
		emptiedEvt.isEmpty = true;
		Core::Game::getNotifier().notifyTreeNodeEmptied(emptiedEvt);

		auto findIndex = [&](const std::shared_ptr<Node>& target) -> int {
			if (!target) return -1;
			for (size_t i = 0; i < nodes.size(); ++i) {
				if (nodes[i] && nodes[i].get() == target.get()) return static_cast<int>(i);
			}
			return -1;
		};

		auto updateAndNotifyParent = [&](const std::shared_ptr<Node>& parent) {
			if (!parent) return;

			auto c1 = parent->getChild1();
			auto c2 = parent->getChild2();
			bool empty1 = (!c1 || !c1->getCard().has_value());
			bool empty2 = (!c2 || !c2->getCard().has_value());
			if (empty1 && empty2) {
				if (auto pc = parent->getCard(); pc.has_value()) {
					pc->get().setIsAvailable(true);
					pc->get().setIsVisible(true);
				}
			}

			int pi = findIndex(parent);
			Core::TreeNodeEvent changedEvt;
			changedEvt.ageIndex = age;
			changedEvt.nodeIndex = pi;
			auto parentCard = parent->getCard();
			changedEvt.cardName = parentCard.has_value() ? parentCard->get().getName() : std::string();
			changedEvt.isEmpty = !parentCard.has_value();
			changedEvt.isAvailable = parent->isAvailable();
			changedEvt.isVisible = parentCard.has_value() ? parentCard->get().isVisible() : false;
			Core::Game::getNotifier().notifyTreeNodeChanged(changedEvt);
		};

		updateAndNotifyParent(emptiedNode->getParent1());
		updateAndNotifyParent(emptiedNode->getParent2());
	}
	bool Game::applyTreeCardAction(int age, int nodeIndex, int action, std::optional<size_t> wonderIndex)
	{
		auto& gs = GameState::getInstance();
		auto p1 = gs.GetPlayer1();
		auto p2 = gs.GetPlayer2();
		auto cur = Core::getCurrentPlayer();
		if (!cur) {
			cur = gs.isPlayer1Turn() ? p1 : p2;
			Core::setCurrentPlayer(cur);
		}
		if (!cur || !cur->m_player) return false;
		auto opp = (cur.get() == p1.get()) ? p2 : p1;
		if (!opp || !opp->m_player) return false;

		auto& board = Board::getInstance();
		const auto& nodes = (age == 1) ? board.getAge1Nodes() : (age == 2) ? board.getAge2Nodes() : board.getAge3Nodes();
		if (nodeIndex < 0 || static_cast<size_t>(nodeIndex) >= nodes.size()) return false;
		auto node = nodes[static_cast<size_t>(nodeIndex)];
		if (!node) return false;
		if (!node->isAvailable()) return false;
		if (!node->getCard().has_value()) return false;

		std::unique_ptr<Models::Card> cardPtr = node->releaseCard();
		if (!cardPtr) return false;

		auto notifyError = [&](const std::string& msg) {
			auto& notifier = GameState::getInstance().getEventNotifier();
			DisplayRequestEvent ev;
			ev.displayType = DisplayRequestEvent::Type::ERROR;
			ev.context = msg;
			notifier.notifyDisplayRequested(ev);
		};

		switch (action) {
		case 0: {
			if (!cur->canAffordCard(*cardPtr, opp->m_player)) {
				notifyError("You cannot afford to build this card.");
				break;
			}
			cur->playCardBuilding(cardPtr, opp->m_player);
			break;
		}
		case 1: {
			auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
			cur->sellCard(cardPtr, discarded);
			break;
		}
		case 2: {
			if (Models::Wonder::getWondersBuilt() >= Models::Wonder::MaxWonders) {
				notifyError("Cannot build a wonder: maximum of 7 wonders have already been built.");
				break;
			}
			if (!cur->m_player) {
				notifyError("Player data is invalid.");
				break;
			}

			auto& owned = cur->m_player->getOwnedWonders();
			std::vector<size_t> candidates;
			for (size_t i = 0; i < owned.size(); ++i) {
				if (owned[i] && !owned[i]->IsConstructed()) candidates.push_back(i);
			}
			if (candidates.empty()) {
				notifyError("No unbuilt wonders available.");
				break;
			}

			size_t chosenIdx = candidates.front();
			if (wonderIndex.has_value()) {
				size_t wi = wonderIndex.value();
				if (wi < candidates.size()) chosenIdx = candidates[wi];
				else if (wi < owned.size()) chosenIdx = wi;
			}

			std::unique_ptr<Models::Wonder>& chosenWonderPtr = owned[chosenIdx];
			if (!chosenWonderPtr) {
				notifyError("Selected wonder is invalid.");
				break;
			}
			if (!cur->canAffordWonder(chosenWonderPtr, opp->m_player)) {
				notifyError("You cannot afford to build this wonder.");
				break;
			}

			std::vector<Models::Token> discardedTokens;
			auto& discardedCards = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
			cur->playCardWonder(chosenWonderPtr, cardPtr, opp->m_player, discardedTokens, discardedCards);
			break;
		}
		default: {
			auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
			discarded.push_back(std::move(cardPtr));
			break;
		}
		}

		if (cardPtr) {
			node->setCard(std::move(cardPtr));
			return false;
		}

		if (action == 0) { 
			uint8_t shields = 0;
			if (cur->m_player) {
				const auto& ownedCards = cur->m_player->getOwnedCards();
				if (!ownedCards.empty()) {
					const auto* lastCard = ownedCards.back().get();
					if (auto ac = dynamic_cast<const Models::AgeCard*>(lastCard)) {
						shields = static_cast<uint8_t>(ac->getShieldPoints());
					}
				}
			}
			
			if (shields > 0) {
				bool isPlayer1 = (cur.get() == p1.get());
				int steps = isPlayer1 ? static_cast<int>(shields) : -static_cast<int>(shields);
				Game::movePawn(steps);
			}
		}

		Game::updateTreeAfterPick(age, nodeIndex);
		return true;
	}

	WonderTradeCostBreakdown Game::computeWonderTradeCost(const Player& cur, const Models::Wonder& wonder, const Player& opp)
	{
		WonderTradeCostBreakdown out;
		if (!cur.m_player || !opp.m_player) return out;

		const auto& cost = wonder.getResourceCost();
		const auto& ownPermanent = cur.m_player->getOwnedPermanentResources();
		const auto& ownTrading = cur.m_player->getOwnedTradingResources();

		out.availableCoins = cur.m_player->totalCoins(cur.m_player->getRemainingCoins());

		std::unordered_map<Models::ResourceType, uint8_t> missingResources;
		int totalMissingUnits = 0;

		for (const auto& kv : cost) {
			auto resource = kv.first;
			auto requiredAmount = kv.second;
			uint8_t produced = 0;
			if (auto it = ownPermanent.find(resource); it != ownPermanent.end()) produced = static_cast<uint8_t>(produced + it->second);
			if (auto it = ownTrading.find(resource); it != ownTrading.end()) produced = static_cast<uint8_t>(produced + it->second);
			if (produced < requiredAmount) {
				uint8_t missing = static_cast<uint8_t>(requiredAmount - produced);
				missingResources[resource] = missing;
				totalMissingUnits += missing;
			}
		}

		if (missingResources.empty()) {
			out.canAfford = true;
			out.totalCost = 0;
			return out;
		}

		bool hasArchitecture = hasTokenByName(*cur.m_player, "Architecture");

		if (hasArchitecture && totalMissingUnits > 0) {
			out.architectureTokenApplied = true;

			struct CostCandidate { uint8_t costPerUnit; Models::ResourceType resource; };
			std::vector<CostCandidate> candidates;
			candidates.reserve(missingResources.size());

			auto opponentProdPre = computeOpponentBrownGreyProductionFor(*cur.m_player, *opp.m_player, missingResources);
			for (const auto& kv : missingResources) {
				auto res = kv.first;
				int discount = tradeDiscountForResource(*cur.m_player, res);
				uint8_t oppAmt = opponentProdPre.count(res) ? opponentProdPre.at(res) : 0;
				uint8_t cpu = (discount != -1) ? static_cast<uint8_t>(discount) : static_cast<uint8_t>(2 + oppAmt);
				candidates.push_back(CostCandidate{ cpu, res });
			}

			std::sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b) {
				return a.costPerUnit > b.costPerUnit;
				});

			int discountUnits = 2;
			for (const auto& cand : candidates) {
				if (discountUnits <= 0) break;
				uint8_t needed = missingResources[cand.resource];
				uint8_t used = static_cast<uint8_t>(std::min<int>(needed, discountUnits));
				missingResources[cand.resource] = static_cast<uint8_t>(missingResources[cand.resource] - used);
				discountUnits -= used;
			}
		}

		auto opponentProd = computeOpponentBrownGreyProductionFor(*cur.m_player, *opp.m_player, missingResources);

		uint8_t totalCost = 0;
		out.lines.clear();
		out.lines.reserve(missingResources.size());

		for (const auto& kv : missingResources) {
			auto res = kv.first;
			auto amt = kv.second;
			if (amt == 0) continue;

			int discount = tradeDiscountForResource(*cur.m_player, res);
			bool discounted = (discount != -1);
			uint8_t oppAmt = opponentProd.count(res) ? opponentProd.at(res) : 0;
			uint8_t cpu = discounted ? static_cast<uint8_t>(discount) : static_cast<uint8_t>(2 + oppAmt);
			uint8_t lineTotal = static_cast<uint8_t>(cpu * amt);

			totalCost = static_cast<uint8_t>(totalCost + lineTotal);
			out.lines.push_back(WonderTradeCostLine{ res, amt, cpu, lineTotal, discounted });
		}

		out.totalCost = totalCost;
		out.canAfford = (out.availableCoins >= out.totalCost);
		return out;
	}

} 