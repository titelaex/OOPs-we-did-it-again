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
#include <exception>
#include <filesystem>
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
			auto safePointsRaw = [](Models::Player* mp)->uint32_t {
				if (!mp) return 0;
				const auto& ptsRef = mp->getPoints();
				uint32_t pts = static_cast<uint32_t>(ptsRef.m_militaryVictoryPoints)
					+ static_cast<uint32_t>(ptsRef.m_buildingVictoryPoints)
					+ static_cast<uint32_t>(ptsRef.m_wonderVictoryPoints)
					+ static_cast<uint32_t>(ptsRef.m_progressVictoryPoints);
				uint8_t totalCoins = mp->totalCoins(mp->getRemainingCoins());
				pts += static_cast<uint32_t>(totalCoins / 3);
				return pts;
				};
			Models::Player* m1 = p1.m_player.get();
			Models::Player* m2 = p2.m_player.get();
			uint32_t total1 = safePointsRaw(m1);
			uint32_t total2 = safePointsRaw(m2);
			if (total1 > total2) return 0;
			if (total2 > total1) return 1;
			auto bluePointsRaw = [](Models::Player* mp)->uint32_t {
				if (!mp) return 0;
				uint32_t sum = 0;
				for (const auto& cptr : mp->getOwnedCards()) {
					if (!cptr) continue;
					if (cptr->getColor() == Models::ColorType::BLUE) sum += cptr->getVictoryPoints();
				}
				return sum;
				};
			uint32_t b1 = bluePointsRaw(m1);
			uint32_t b2 = bluePointsRaw(m2);
			if (b1 > b2) return 0;
			if (b2 > b1) return 1;
			return -1;
		}
		void performCardAction(int action, Player& cur, Player& opp, std::unique_ptr<Models::Card>& cardPtr, Board& board, IPlayerDecisionMaker* decisionMaker = nullptr) {
			if (!cardPtr) return;
			
			switch (action) {
			case 0: {
				if (!cur.canAffordCard(cardPtr.get(), opp.m_player)) {
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
					discarded.push_back(std::move(cardPtr));
					break;
				}
				if (!decisionMaker) {
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
				}
			 size_t wchoice = decisionMaker ? decisionMaker->selectWonder(candidates) : 0;
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
		uint8_t getShieldPointsFromCard(const Models::Card* card) {
			if (!card) return 0;
			if (auto ac = dynamic_cast<const Models::AgeCard*>(card)) {
				return static_cast<uint8_t>(ac->getShieldPoints());
			}
			return 0;
		}
		void displayCardDetails(const Models::Card* card) {
			if (!card) return;
			auto& notifier = GameState::getInstance().getEventNotifier();
			
			DisplayRequestEvent event;
			event.displayType = DisplayRequestEvent::Type::MESSAGE;
			event.context = "\n=== CARD DETAILS ===";
			notifier.notifyDisplayRequested(event);
			
			event.context = "Name: " + std::string(card->getName());
			notifier.notifyDisplayRequested(event);
			
			event.context = "Color: " + Models::ColorTypeToString(card->getColor());
			notifier.notifyDisplayRequested(event);
			
			const auto& resCost = card->getResourceCost();
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

			if (auto* ageCard = dynamic_cast<const Models::AgeCard*>(card)) {
				uint8_t coinCost = ageCard->getCoinCost();
				if (coinCost > 0) {
					event.context = "Coin Cost: " + std::to_string(coinCost);
					notifier.notifyDisplayRequested(event);
				}
			}

			if (card->getVictoryPoints() > 0) {
				event.context = "Victory Points: " + std::to_string(card->getVictoryPoints());
				notifier.notifyDisplayRequested(event);
			}
			if (auto* ageCard = dynamic_cast<const Models::AgeCard*>(card)) {
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
			if (auto* wonder = dynamic_cast<const Models::Wonder*>(card)) {
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
			event.context = "\n--- " + label + " RESOURCES ---";
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

	}
	void Game::preparation() {
		try {
			PrepareBoardCardPools();
			auto& board = Core::Board::getInstance();
			board.setPawnPos(9);
			std::bitset<19> pawnTrack;
			pawnTrack.set();
			board.setPawnTrack(pawnTrack);
			auto allTokens = parseTokensFromCSV("Config/Tokens.csv");
			auto [progressSelected, military] = startGameTokens(std::move(allTokens));
			Core::Board::getInstance().setProgressTokens(std::move(progressSelected));
			Core::Board::getInstance().setMilitaryTokens(std::move(military));
			Core::Board::getInstance().setUnusedProgressTokens(std::move(*setupUnusedProgressTokens));
		}
		catch (const std::exception& ex) {
			auto& notifier = GameState::getInstance().getEventNotifier();
			DisplayRequestEvent event;
			event.displayType = DisplayRequestEvent::Type::ERROR;
			event.context = std::string("Preparation exception: ") + ex.what();
			notifier.notifyDisplayRequested(event);
		}
		catch (...) {
			auto& notifier = GameState::getInstance().getEventNotifier();
			DisplayRequestEvent event;
			event.displayType = DisplayRequestEvent::Type::ERROR;
			event.context = "Unknown exception during preparation";
			notifier.notifyDisplayRequested(event);
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
						if (auto card = node->getCard()) {
						 card->setIsVisible(rowVisible);
						 card->setIsAvailable(isLastRow);
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
				return c ? c->getName() : std::string{ "<none>" };
				};
			for (size_t i = 0; i < nodes.size(); ++i) {
				const auto& n = nodes[i];
				if (!n) continue;
				auto c = n->getCard();
				event.context = "[" + std::to_string(i) + "] " + (c ? c->getName() : std::string{ "<none>" });
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
	void Game::wonderSelection(std::shared_ptr<Core::Player>& p1, std::shared_ptr<Core::Player>& p2, IPlayerDecisionMaker* p1Decisions, IPlayerDecisionMaker* p2Decisions) {
		bool deleteP1 = false, deleteP2 = false;
		if (!p1Decisions) {
			p1Decisions = new HumanDecisionMaker();
			deleteP1 = true;
		}
		if (!p2Decisions) {
			p2Decisions = new HumanDecisionMaker();
			deleteP2 = true;
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
				wonders[i]->displayCardInfo();
			}
			event.context = "=========================";
			notifier.notifyDisplayRequested(event);
			};

		// Load 4 wonders for Round 1
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
				playerOrder = { true, false, false, true };  // P1, P2, P2, P1
			}
			else {
				playerOrder = { false, true, true, false };  // P2, P1, P1, P2
			}

			for (size_t i = 0; i < playerOrder.size(); ++i) {
				if (availableWonders.empty()) break;

				bool isPlayer1 = playerOrder[i];
				IPlayerDecisionMaker* decisionMaker = isPlayer1 ? p1Decisions : p2Decisions;
				std::shared_ptr<Core::Player> currentPlayer = isPlayer1 ? p1 : p2;

				if (availableWonders.size() == 1) {
					DisplayRequestEvent event;
					event.displayType = DisplayRequestEvent::Type::MESSAGE;
					event.context = "\n> " + std::string(isPlayer1 ? "Player 1" : "Player 2") + 
						" automatically receives the final wonder: " + availableWonders[0]->getName();
					notifier.notifyDisplayRequested(event);
					currentPlayer->chooseWonder(availableWonders, 0);
					break;
				}

				DisplayRequestEvent promptEvent;
				promptEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
				promptEvent.context = "\n> " + std::string(isPlayer1 ? "Player 1" : "Player 2") +
					", choose your wonder (0-" + std::to_string(availableWonders.size() - 1) + "): ";
				notifier.notifyDisplayRequested(promptEvent);

				std::vector<size_t> wonderIndices;
				for (size_t j = 0, end = availableWonders.size(); j < end; ++j) wonderIndices.push_back(j);
				size_t choice = decisionMaker->selectCard(wonderIndices);

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

		if (deleteP1) delete p1Decisions;
		if (deleteP2) delete p2Decisions;
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
	void Game::awardMilitaryTokenIfPresent(Player& receiver) {
		auto& board = Board::getInstance();
		int pos = board.getPawnPos();
		for (int p : kMilitaryTokenPositions) {
			if (p == pos) {
				auto& military = const_cast<std::vector<std::unique_ptr<Models::Token>>&>(board.getMilitaryTokens());
				if (!military.empty()) {
					std::unique_ptr<Models::Token> t = std::move(military.back());
					military.pop_back();
					if (t) {
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
	                         TrainingLogger* logger)
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
			} else if (currentPhase == 2) {
				nodes = &board.getAge2Nodes();
				phaseName = "PHASE II";
			} else {
				nodes = &board.getAge3Nodes();
				phaseName = "PHASE III";
			}

			DisplayRequestEvent phaseEvent;
			phaseEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
			phaseEvent.context = "=== Starting " + phaseName + " ===";
			notifier.notifyDisplayRequested(phaseEvent);

			std::vector<size_t> availableIndex;
			availableIndex.reserve(nodes->size());

			for (size_t i = 0; i < nodes->size(); ++i) {
				const auto& node = (*nodes)[i];
				if (!node) continue;
				auto card = node->getCard();
				if (card && node->isAvailable() && card->isAvailable()) {
					availableIndex.push_back(i);
				}
			}

			if (availableIndex.empty()) {
				DisplayRequestEvent completeEvent;
				completeEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
				completeEvent.context = phaseName + " completed.";
				notifier.notifyDisplayRequested(completeEvent);
				currentPhase++;
				nrOfRounds = 1;
				continue;
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
				if (card) {
					card->displayCardInfo();
					if (auto ageCard = dynamic_cast<const Models::AgeCard*>(card)) {
						if (ageCard->getScientificSymbols().has_value()) {
						 cardEvent.context = " Science: " + Models::ScientificSymbolTypeToString(ageCard->getScientificSymbols().value());
						 notifier.notifyDisplayRequested(cardEvent);
						}
					}
				}
			}

			Player& cur = playerOneTurn ? p1 : p2;
			Player& opp = playerOneTurn ? p2 : p1;

			Core::setCurrentPlayer(&cur);

			IPlayerDecisionMaker& curDecisionMaker = playerOneTurn ? p1Decisions : p2Decisions;

			displayPlayerResources(cur, playerOneTurn ? "Player1" : "Player2");
			DisplayRequestEvent promptEvent;
			promptEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
			promptEvent.context = std::string(playerOneTurn ? "Player1" : "Player2") + " choose index (0-" + std::to_string(availableIndex.size() - 1) + "): ";
			notifier.notifyDisplayRequested(promptEvent);

			size_t choice = curDecisionMaker.selectCard(availableIndex);
			if (choice >= availableIndex.size()) choice = 0;
			size_t chosenNodeIndex = availableIndex[choice];

			std::unique_ptr<Models::Card> cardPtr = (*nodes)[chosenNodeIndex]->releaseCard();
			if (!cardPtr) {
				DisplayRequestEvent errEvent;
				errEvent.displayType = DisplayRequestEvent::Type::ERROR;
				errEvent.context = "Node releaseCard failed.";
				notifier.notifyDisplayRequested(errEvent);
				continue;
			}

			std::string cardName = cardPtr->getName();
			displayCardDetails(cardPtr.get());
			uint8_t shields = getShieldPointsFromCard(cardPtr.get());

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
			int action = curDecisionMaker.selectCardAction();
			int attemptCount = 0;
			const int maxAttempts = 3;
			bool cancelled = false;

			while (attemptCount < maxAttempts && cardPtr) {
				performCardAction(action, cur, opp, cardPtr, board, &curDecisionMaker);

				if (cardPtr) {
					attemptCount++;
					bool isAI = (dynamic_cast<HumanDecisionMaker*>(&curDecisionMaker) == nullptr);

					if (isAI) {
						DisplayRequestEvent retryEvent;
						retryEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
						retryEvent.context = "[AI] Action " + std::to_string(action) + " failed. Retrying...";
						notifier.notifyDisplayRequested(retryEvent);

						if (attemptCount >= 2) {
							DisplayRequestEvent discardEvent;
							discardEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
							discardEvent.context = "Forcing discard.";
							notifier.notifyDisplayRequested(discardEvent);

							auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
							discarded.push_back(std::move(cardPtr));
							break;
						}
						action = 1;
					}
					else {
						DisplayRequestEvent cancelEvent;
						cancelEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
						cancelEvent.context = "*** ACTION CANCELLED ***";
						notifier.notifyDisplayRequested(cancelEvent);

						(*nodes)[chosenNodeIndex]->setCard(std::move(cardPtr));
						cancelled = true;
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

					 cur.chooseProgressTokenFromBoard();
				 }
				}
			}

			if (cancelled) {
				continue;
			}

			if ((*nodes)[chosenNodeIndex]->getCard() != nullptr) {
				continue;
			}

			if (auto takenNode = (*nodes)[chosenNodeIndex]) {
				auto checkParent = [](const std::shared_ptr<Node>& p) {
					if (p) {
						auto c1 = p->getChild1();
						auto c2 = p->getChild2();
						bool empty1 = (!c1 || c1->getCard() == nullptr);
						bool empty2 = (!c2 || c2->getCard() == nullptr);
						if (empty1 && empty2 && p->getCard()) {
							p->getCard()->setIsAvailable(true);
							p->getCard()->setIsVisible(true);
						}
					}
				};
				checkParent(takenNode->getParent1());
				checkParent(takenNode->getParent2());
			}

			if (logger) {
				MCTSGameState state = MCTS::captureGameState(1, playerOneTurn);
				MCTSAction mctsAction;
				mctsAction.cardNodeIndex = chosenNodeIndex;
				mctsAction.actionType = action;
				mctsAction.cardName = cardName;
				TurnRecord turn = createTurnRecord(state, mctsAction, nrOfRounds, 0.5, 0.5);
				logger->logTurn(turn);
			}

			gameState.setCurrentPhase(currentPhase, nrOfRounds, playerOneTurn);
			gameState.saveGameState("");

			DisplayRequestEvent saveEvent;
			saveEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
			saveEvent.context = "[AUTO-SAVE] " + phaseName + " Round " + std::to_string(nrOfRounds) + " saved.";
			notifier.notifyDisplayRequested(saveEvent);

			if (shields > 0) {
				Game::movePawn(playerOneTurn ? (int)shields : -(int)shields);
				awardMilitaryTokenIfPresent(cur);
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

		g_last_active_was_player_one = !playerOneTurn;

		DisplayRequestEvent allPhaseEvent;
		allPhaseEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
		allPhaseEvent.context = "=== All Phases Completed ===";
		notifier.notifyDisplayRequested(allPhaseEvent);
	}


	void Game::movePawn(int steps) {
		auto& board = Core::Board::getInstance();
		auto position = board.getPawnPos();
		position = std::clamp(position + steps, 0, 18);
		board.setPawnPos(position);
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
					card->displayCardInfo();
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
				" + 3x" + std::to_string(std::get<1>(coins)) + " + 6x" + std::to_string(std::get<2>(coins)) + ")";
			notifier.notifyDisplayRequested(event);
			
			event.context = "\nOwned Cards (" + std::to_string(p2.m_player->getOwnedCards().size()) + "):";
			notifier.notifyDisplayRequested(event);
			for (const auto& card : p2.m_player->getOwnedCards()) {
				if (card) {
					event.context = "  - ";
					notifier.notifyDisplayRequested(event);
					card->displayCardInfo();
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
		
		event.context = "\n╔════════════════════════════════════════════════════════════════╗";
		notifier.notifyDisplayRequested(event);
		event.context = "║                      CURRENT GAME STATUS                       ║";
		notifier.notifyDisplayRequested(event);
		event.context = "╠════════════════════════════════════════════════════════════════╣";
		notifier.notifyDisplayRequested(event);
		event.context = "║ MILITARY TRACK:                                                ║";
		notifier.notifyDisplayRequested(event);
		
		std::string track = "║ P1 [";
	for (int i = 0; i <= 18; ++i) {
		if (i == pawnPos) {
			track += "●";
		}
		else if (i == 9) {
			track += "|";
		}
		else {
			track += "-";
		}
	}
	track += "] P2 ║";
	event.context = track;
	notifier.notifyDisplayRequested(event);
	
	event.context = "║    0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18    ║";
	notifier.notifyDisplayRequested(event);
	
	event.context = "║ Position: " + std::to_string(pawnPos);
	if (pawnPos < 9) event.context += " (P1 winning by " + std::to_string(9 - pawnPos) + ")";
	else if (pawnPos > 9) event.context += " (P2 winning by " + std::to_string(pawnPos - 9) + ")";
	else event.context += " (Neutral)";
	event.context += std::string(33 - std::to_string(pawnPos).length(), ' ') + "║";
	notifier.notifyDisplayRequested(event);
	
	event.context = "╠════════════════════════════════════════════════════════════════╣";
	notifier.notifyDisplayRequested(event);
	
	auto calculateScore = [](const Player& p) -> uint32_t {
		if (!p.m_player) return 0;
		const auto& pts = p.m_player->getPoints();
		uint32_t total = static_cast<uint32_t>(pts.m_militaryVictoryPoints) +
			static_cast<uint32_t>(pts.m_buildingVictoryPoints) +
			static_cast<uint32_t>(pts.m_wonderVictoryPoints) +
			static_cast<uint32_t>(pts.m_progressVictoryPoints);
		total += p.m_player->totalCoins(p.m_player->getRemainingCoins()) / 3;
		return total;
		};
	uint32_t score1 = calculateScore(p1);
	uint32_t score2 = calculateScore(p2);
	
	event.context = "║ PLAYER 1: " + std::string(p1.m_player ? p1.m_player->getPlayerUsername() : "Unknown");
	event.context += std::string(51 - (p1.m_player ? p1.m_player->getPlayerUsername().length() : 7), ' ') + "║";
	notifier.notifyDisplayRequested(event);
	
	if (p1.m_player) {
		const auto& pts1 = p1.m_player->getPoints();
		event.context = "║   Total Score: " + std::to_string(score1) + " VP";
		event.context += std::string(47 - std::to_string(score1).length(), ' ') + "║";
		notifier.notifyDisplayRequested(event);
		
		event.context = "║   • Military: " + std::to_string(pts1.m_militaryVictoryPoints) + " VP";
		event.context += std::string(45 - std::to_string(pts1.m_militaryVictoryPoints).length(), ' ') + "║";
		notifier.notifyDisplayRequested(event);
		
		event.context = "║   • Buildings: " + std::to_string(pts1.m_buildingVictoryPoints) + " VP";
		event.context += std::string(44 - std::to_string(pts1.m_buildingVictoryPoints).length(), ' ') + "║";
		notifier.notifyDisplayRequested(event);
		
		event.context = "║   • Wonders: " + std::to_string(pts1.m_wonderVictoryPoints) + " VP";
		event.context += std::string(46 - std::to_string(pts1.m_wonderVictoryPoints).length(), ' ') + "║";
		notifier.notifyDisplayRequested(event);
		
		event.context = "║   • Progress: " + std::to_string(pts1.m_progressVictoryPoints) + " VP";
		event.context += std::string(45 - std::to_string(pts1.m_progressVictoryPoints).length(), ' ') + "║";
		notifier.notifyDisplayRequested(event);
		
		uint32_t coinVP = p1.m_player->totalCoins(p1.m_player->getRemainingCoins()) / 3;
		event.context = "║   • Coins: " + std::to_string(coinVP) + " VP";
		event.context += std::string(48 - std::to_string(coinVP).length(), ' ') + "║";
		notifier.notifyDisplayRequested(event);
	}
	
	event.context = "╠════════════════════════════════════════════════════════════════╣";
	notifier.notifyDisplayRequested(event);
	
	event.context = "║ PLAYER 2: " + std::string(p2.m_player ? p2.m_player->getPlayerUsername() : "Unknown");
	event.context += std::string(51 - (p2.m_player ? p2.m_player->getPlayerUsername().length() : 7), ' ') + "║";
	notifier.notifyDisplayRequested(event);
	
	if (p2.m_player) {
		const auto& pts2 = p2.m_player->getPoints();
		event.context = "║   Total Score: " + std::to_string(score2) + " VP";
		event.context += std::string(47 - std::to_string(score2).length(), ' ') + "║";
		notifier.notifyDisplayRequested(event);
		
		event.context = "║   • Military: " + std::to_string(pts2.m_militaryVictoryPoints) + " VP";
		event.context += std::string(45 - std::to_string(pts2.m_militaryVictoryPoints).length(), ' ') + "║";
		notifier.notifyDisplayRequested(event);
		
		event.context = "║   • Buildings: " + std::to_string(pts2.m_buildingVictoryPoints) + " VP";
		event.context += std::string(44 - std::to_string(pts2.m_buildingVictoryPoints).length(), ' ') + "║";
		notifier.notifyDisplayRequested(event);
		
		event.context = "║   • Wonders: " + std::to_string(pts2.m_wonderVictoryPoints) + " VP";
		event.context += std::string(46 - std::to_string(pts2.m_wonderVictoryPoints).length(), ' ') + "║";
		notifier.notifyDisplayRequested(event);
		
		event.context = "║   • Progress: " + std::to_string(pts2.m_progressVictoryPoints) + " VP";
		event.context += std::string(45 - std::to_string(pts2.m_progressVictoryPoints).length(), ' ') + "║";
		notifier.notifyDisplayRequested(event);
		
		uint32_t coinVP = p2.m_player->totalCoins(p2.m_player->getRemainingCoins()) / 3;
		event.context = "║   • Coins: " + std::to_string(coinVP) + " VP";
		event.context += std::string(48 - std::to_string(coinVP).length(), ' ') + "║";
		notifier.notifyDisplayRequested(event);
	}
	
	event.context = "╚════════════════════════════════════════════════════════════════╝\n";
	notifier.notifyDisplayRequested(event);
}
void Game::announceVictory(int winner, const std::string& victoryType, const Player& p1, const Player& p2) {
	auto& notifier = GameState::getInstance().getEventNotifier();
	DisplayRequestEvent event;
	event.displayType = DisplayRequestEvent::Type::MESSAGE;
	
	event.context = "\n╔════════════════════════════════════════════════════════════════╗";
	notifier.notifyDisplayRequested(event);
	event.context = "║                         GAME OVER!                             ║";
	notifier.notifyDisplayRequested(event);
	event.context = "╠════════════════════════════════════════════════════════════════╣";
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
	
	event.context = "║ Victory Type: " + victoryType;
	event.context += std::string(49 - victoryType.length(), ' ') + "║";
	notifier.notifyDisplayRequested(event);
	
	event.context = "║ Winner: " + winnerName;
	event.context += std::string(55 - winnerName.length(), ' ') + "║";
	notifier.notifyDisplayRequested(event);
	
	event.context = "╚════════════════════════════════════════════════════════════════╝\n";
	notifier.notifyDisplayRequested(event);
	
	displayTurnStatus(p1, p2);

}

// Helper function to check if saved game exists
bool hasSavedGame(const std::string& filename) {
	return std::filesystem::exists(filename);
}

void Game::initGame() {
	GameState& gameState = GameState::getInstance();
	auto& notifier = gameState.getEventNotifier();
	Core::ConsoleReader reader;

	bool trainingMode = false;
	IPlayerDecisionMaker* p1Decisions = nullptr;
	IPlayerDecisionMaker* p2Decisions = nullptr;
	Core::Playstyle p1Playstyle = Core::Playstyle::BRITNEY;
	Core::Playstyle p2Playstyle = Core::Playstyle::BRITNEY;
	TrainingLogger* logger = nullptr;

	std::vector<int> existingSaves = GameStateSerializer::getAllSaveNumbers();

	if (!existingSaves.empty()) {
		DisplayRequestEvent savesEvent;
		savesEvent.displayType = DisplayRequestEvent::Type::AVAILABLE_SAVES;
		savesEvent.saveNumbers = existingSaves;
		notifier.notifyDisplayRequested(savesEvent);

		int choice = reader.selectSave(existingSaves);
		if (choice > 0 && std::find(existingSaves.begin(), existingSaves.end(), choice) != existingSaves.end()) {
			gameState.loadGameState("");
			GameStateSerializer::setCurrentSaveNumber(choice);

			DisplayRequestEvent loadEvent;
			loadEvent.displayType = DisplayRequestEvent::Type::MESSAGE;
			loadEvent.context = "Game state loaded from save #" + std::to_string(choice);
			notifier.notifyDisplayRequested(loadEvent);

			auto p1Ptr = gameState.GetPlayer1();
		 auto p2Ptr = gameState.GetPlayer2();

			p1Decisions = new Core::HumanDecisionMaker();
		 p2Decisions = new Core::HumanDecisionMaker();

			playAllPhases(*p1Ptr, *p2Ptr, *p1Decisions, *p2Decisions, logger);

			delete p1Decisions;
			delete p2Decisions;
			return;
		}
	}

	DisplayRequestEvent gameModeEvent;
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
		p1Decisions = new Core::HumanDecisionMaker();
		p2Decisions = new Core::MCTSDecisionMaker(p2Playstyle, 1000, 1.414, 20);

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

		p1Decisions = new Core::MCTSDecisionMaker(p1Playstyle, 1000, 1.414, 20);
		p2Decisions = new Core::MCTSDecisionMaker(p2Playstyle, 1000, 1.414, 20);

		if (trainingMode) {
			logger = new TrainingLogger();
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

		p1Decisions = new Core::HumanAssistedDecisionMaker(p1Playstyle, 500);
		p2Decisions = new Core::HumanAssistedDecisionMaker(p2Playstyle, 500);

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

		p1Decisions = new Core::HumanDecisionMaker();
		p2Decisions = new Core::HumanDecisionMaker();

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
	wonderSelection(p1Ptr, p2Ptr, p1Decisions, p2Decisions);

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

	playAllPhases(*p1Ptr, *p2Ptr, *p1Decisions, *p2Decisions, logger);

	if (p1Decisions) delete p1Decisions;
	if (p2Decisions) delete p2Decisions;
	if (logger) delete logger;
}
}