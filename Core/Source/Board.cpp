module Core.Board;
import Core.Node;
import Models.AgeCard;
import Models.GuildCard;
import Models.Wonder;
import Models.Token;
import Models.Card;
import Models.Bank;


import <vector>;
import <algorithm>;
import <bitset>;
import <iostream>;
import <deque>;
import <string>;
import <sstream>;
import Core.CardCsvParser;

using namespace Core;

Board::Board() : bank{}, pawnTrack{}, pawnPos{ 0 } {}

Models::Bank& Board::getBank() { return bank; }

const std::vector<std::unique_ptr<Models::Card>>& Board::getUnusedAgeOneCards() const { return unusedAgeOneCards; }
void Board::setUnusedAgeOneCards(std::vector<std::unique_ptr<Models::Card>> v) { unusedAgeOneCards = std::move(v); }
const std::vector<std::unique_ptr<Models::Card>>& Board::getUnusedAgeTwoCards() const { return unusedAgeTwoCards; }
void Board::setUnusedAgeTwoCards(std::vector<std::unique_ptr<Models::Card>> v) { unusedAgeTwoCards = std::move(v); }
const std::vector<std::unique_ptr<Models::Card>>& Board::getUnusedAgeThreeCards() const { return	unusedAgeThreeCards; }
void Board::setUnusedAgeThreeCards(std::vector<std::unique_ptr<Models::Card>> v) { unusedAgeThreeCards = std::move(v); }
const std::vector<std::unique_ptr<Models::Card>>& Board::getUnusedGuildCards() const { return unusedGuildCards; }
void Board::setUnusedGuildCards(std::vector<std::unique_ptr<Models::Card>> v) { unusedGuildCards = std::move(v); }
const std::vector<std::unique_ptr<Models::Card>>& Board::getUnusedWonders() const { return unusedWonders; }
void Board::setUnusedWonders(std::vector<std::unique_ptr<Models::Card>> v) { unusedWonders = std::move(v); }

const std::vector<std::unique_ptr<Models::Card>>& Board::getDiscardedCards() const { return discardedCards; }
void Board::setDiscardedCards(std::vector<std::unique_ptr<Models::Card>> v) { discardedCards = std::move(v); }

const std::bitset<19>& Board::getPawnTrack() const { return pawnTrack; }
void Board::setPawnTrack(const std::bitset<19>& track) { pawnTrack = track; }
uint8_t Board::getPawnPos() const { return pawnPos; }
void Board::setPawnPos(uint8_t pos) { pawnPos = pos; }

const std::vector<std::unique_ptr<Models::Token>>& Board::getProgressTokens() const { return progressTokens; }
void Board::setProgressTokens(std::vector<std::unique_ptr<Models::Token>> v) { progressTokens = std::move(v); }
const std::vector<std::unique_ptr<Models::Token>>& Core::Board::getUnusedProgressTokens() const{ return unusedProgressTokens; }
void Core::Board::setUnusedProgressTokens(std::vector<std::unique_ptr<Models::Token>> v)
{
	unusedProgressTokens = std::move(v);
}
const std::vector<std::unique_ptr<Models::Token>>& Board::getMilitaryTokens() const { return militaryTokens; }
void Board::setMilitaryTokens(std::vector<std::unique_ptr<Models::Token>> v) { militaryTokens = std::move(v); }

const std::vector<std::shared_ptr<Node>>& Board::getAge1Nodes() const { return age1Nodes; }
void Board::setAge1Nodes(std::vector<std::shared_ptr<Node>> v) { age1Nodes = std::move(v); }
const std::vector<std::shared_ptr<Node>>& Board::getAge2Nodes() const { return age2Nodes; }
void Board::setAge2Nodes(std::vector<std::shared_ptr<Node>> v) { age2Nodes = std::move(v); }
const std::vector<std::shared_ptr<Node>>& Board::getAge3Nodes() const { return age3Nodes; }
void Board::setAge3Nodes(std::vector<std::shared_ptr<Node>> v) { age3Nodes = std::move(v); }

std::deque<Models::Card*> Board::getAvailableCardsByAge(int age) const
{
    const std::vector<std::shared_ptr<Node>>* source = nullptr;
    switch (age)
    {
    case 1: source = &age1Nodes; break;
    case 2: source = &age2Nodes; break;
    case 3: source = &age3Nodes; break;
    default: return {};
    }

    std::deque<Models::Card*> available;
    if (!source) return available;

    for (const auto& node : *source)
    {
        if (!node) continue;
        Models::Card* card = node->getCard();
        if (!card) continue;
        if (card->isAvailable()) available.push_back(card);
    }
    return available;
}

void Board::setupCardPools()
{
	unusedAgeOneCards.reserve(23);
	unusedAgeTwoCards.reserve(23);
	unusedAgeThreeCards.reserve(20);
	unusedGuildCards.reserve(7);
	unusedWonders.reserve(12);
}

static void displayProgressTokens(const std::vector< std::unique_ptr<Models::Token >>& progressTokens)
{
	std::cout << "Progress Tokens: ";
	for (const auto& t : progressTokens) {
		if (t) std::cout << t->getName() << " ";
	}
	std::cout << "\n\n";
}

static void displayMilitaryTokens(const std::vector< std::unique_ptr<Models::Token >>& militaryTokens)
{
	std::cout << "Military Tokens: ";
	for (const auto& t : militaryTokens) {
		if (t) std::cout << t->getName() << " ";
	}
	std::cout << "\n\n";
}

void Board::displayBoard()
{
	displayProgressTokens(progressTokens);

	std::cout << "C1 |";
	for (int i = 0; i < 19; ++i) {
		if (pawnTrack.test(i))
			std::cout << " ! ";
		else
			std::cout << " _ ";

		if (((i + 1) % 3 == 0 && i < 9) || (i % 3 == 0 && i >= 9))
			std::cout << "|";
	}
	std::cout << " C2\n\n";

	displayMilitaryTokens(militaryTokens);
}

static void displayUnusedPools(const std::vector<std::unique_ptr<Models::Card>>& age1,
	const std::vector<std::unique_ptr<Models::Card>>& age2,
	const std::vector<std::unique_ptr<Models::Card>>& age3,
	const std::vector<std::unique_ptr<Models::Card>>& guilds,
	const std::vector<std::unique_ptr<Models::Card>>& wonders)
{
	std::cout << "Age I unused: " << age1.size() << " cards\n";
	for (size_t i = 0; i < age1.size(); ++i) {
		auto& p = age1[i];
		if (p) {
			p->displayCardInfo();
		}
	}
	std::cout << "\n";
	std::cout << "Age II unused: " << age2.size() << " cards\n";
	for (size_t i = 0; i < age2.size(); ++i) {
		auto& p = age2[i];
		if (p) {
			p->displayCardInfo();
		}
	}
	std::cout << "\n";
	std::cout << "Age III unused: " << age3.size() << " cards\n";
	for (size_t i = 0; i < age3.size(); ++i) {
		auto& p = age3[i];
		if (p) {
		 p->displayCardInfo();
		}
	}
	std::cout << "\n";
	std::cout << "Guild unused: " << guilds.size() << " cards\n";
	for (size_t i = 0; i < guilds.size(); ++i) {
		auto& p = guilds[i];
		if (p) {
			p->displayCardInfo();
		}
	}
	std::cout << "\n";
	std::cout << "Wonders unused: " << wonders.size() << " cards\n";
	for (size_t i = 0; i < wonders.size(); ++i) {
		auto& p = wonders[i];
		if (p) {
			std::cout << "\n";
			p->displayCardInfo();
		}
	}
}

static void displayAgeCards(const char* title, const std::vector<std::shared_ptr<Node>>& nodes)
{
	std::cout << title << " (" << nodes.size() << " nodes) ---\n";
	for (size_t i = 0; i < nodes.size(); ++i) {
		auto& n = nodes[i];
		if (!n) continue;
		Models::Card* c = n->getCard();
		if (!c) continue;
		c->displayCardInfo();
	}
}

void Board::displayEntireBoard()
{
    std::cout << "=== Board State ===\n";
    displayBoard();
    std::cout << "===================\n";
    std::cout << "--- UNUSED POOLS ---\n";
    displayUnusedPools(unusedAgeOneCards, unusedAgeTwoCards, unusedAgeThreeCards, unusedGuildCards, unusedWonders);
    displayAgeCards("--- Age I Cards", age1Nodes);
    displayAgeCards("--- Age II Cards", age2Nodes);
    displayAgeCards("--- Age III Cards", age3Nodes);
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

namespace Core {
    std::ostream& operator<<(std::ostream& out, const Board& board)
    {
        out << "Section,Type,Data\n";

        // Pawn Track and Position
        out << "Pawn,Track," << board.getPawnTrack().to_string() << "\n";
        out << "Pawn,Position," << static_cast<int>(board.getPawnPos()) << "\n";

        // Progress Tokens
        for (const auto& token : board.getProgressTokens()) {
            if (token) {
                out << "Token,Progress," << *token << "\n";
            }
        }

        // Military Tokens
        for (const auto& token : board.getMilitaryTokens()) {
            if (token) {
                out << "Token,Military," << *token << "\n";
            }
        }

        // Age 1 Nodes
        for (const auto& node : board.getAge1Nodes()) {
            if (node && node->getCard()) {
                out << "Node,Age1,";
                streamCardByType(out, node->getCard());
                out << "\n";
            }
        }

        // Age 2 Nodes
        for (const auto& node : board.getAge2Nodes()) {
            if (node && node->getCard()) {
                out << "Node,Age2,";
                streamCardByType(out, node->getCard());
                out << "\n";
            }
        }

        // Age 3 Nodes
        for (const auto& node : board.getAge3Nodes()) {
            if (node && node->getCard()) {
                out << "Node,Age3,";
                streamCardByType(out, node->getCard());
                out << "\n";
            }
        }

        // Unused Cards
        for (const auto& card : board.getUnusedAgeOneCards()) {
            if (card) {
                out << "Unused,Age1,";
                streamCardByType(out, card.get());
                out << "\n";
            }
        }
        for (const auto& card : board.getUnusedAgeTwoCards()) {
            if (card) {
                out << "Unused,Age2,";
                streamCardByType(out, card.get());
                out << "\n";
            }
        }
        for (const auto& card : board.getUnusedAgeThreeCards()) {
            if (card) {
                out << "Unused,Age3,";
                streamCardByType(out, card.get());
                out << "\n";
            }
        }
        for (const auto& card : board.getUnusedGuildCards()) {
            if (card) {
                out << "Unused,Guild,";
                streamCardByType(out, card.get());
                out << "\n";
            }
        }
        for (const auto& card : board.getUnusedWonders()) {
            if (card) {
                out << "Unused,Wonder,";
                streamCardByType(out, card.get());
                out << "\n";
            }
        }

        // Discarded Cards
        for (const auto& card : board.getDiscardedCards()) {
            if (card) {
                out << "Discarded,Card,";
                streamCardByType(out, card.get());
                out << "\n";
            }
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
	std::istream& operator>>(std::istream& in, Board& board)
	{
		board.setupCardPools();
		board.setAge1Nodes({});
		board.setAge2Nodes({});
		board.setAge3Nodes({});
		board.setProgressTokens({});
		board.setMilitaryTokens({});
		board.setUnusedProgressTokens({});

		std::vector<std::unique_ptr<Models::Token>> progressTokens;
		std::vector<std::unique_ptr<Models::Token>> militaryTokens;

		std::string line;
		// Skip header
		std::getline(in, line);

		while (std::getline(in, line))
		{
			if (line.empty()) continue;

			auto columns = splitCsvLine(line);
			if (columns.size() < 2) continue;

			const std::string& section = columns[0];
			const std::string& type = columns[1];

			if (section == "Pawn") {
				if (type == "Track" && columns.size() > 2) {
					board.setPawnTrack(std::bitset<19>(columns[2]));
				}
				else if (type == "Position" && columns.size() > 2) {
					board.setPawnPos(static_cast<uint8_t>(std::stoi(columns[2])));
				}
			}
			else if (section == "Token" && columns.size() > 2) {
				// Reconstruct token data from remaining columns
				std::vector<std::string> token_cols(columns.begin() + 2, columns.end());
				if (token_cols.size() >= 5) {
					auto csvUnescape = [](const std::string& s) -> std::string {
						if (s.empty()) return s;
						std::string result;
						result.reserve(s.size());
						size_t start = 0;
						size_t end = s.size();
						if (s.front() == '"' && s.back() == '"' && s.size() >= 2) {
							start = 1;
							end = s.size() - 1;
						}
						for (size_t i = start; i < end; ++i) {
							if (s[i] == '"' && i + 1 < end && s[i + 1] == '"') {
								result.push_back('"');
								++i;
							} else {
								result.push_back(s[i]);
							}
						}
						return result;
					};

					auto parseCoins = [](const std::string& s) -> std::tuple<uint8_t,uint8_t,uint8_t> {
						if (s.empty()) return {0,0,0};
						std::istringstream ss(s);
						std::string a,b,c;
						if (!std::getline(ss,a,':')) return {0,0,0};
						if (!std::getline(ss,b,':')) return {static_cast<uint8_t>(std::stoi(a)),0,0};
						if (!std::getline(ss,c)) return {static_cast<uint8_t>(std::stoi(a)), static_cast<uint8_t>(std::stoi(b)),0};
						return {static_cast<uint8_t>(std::stoi(a)), static_cast<uint8_t>(std::stoi(b)), static_cast<uint8_t>(std::stoi(c))};
					};

					std::string tokenTypeStr = csvUnescape(token_cols[0]);
					std::string tokenName = csvUnescape(token_cols[1]);
					std::string tokenDesc = csvUnescape(token_cols[2]);
					std::string coinsStr = csvUnescape(token_cols[3]);
					std::string victoryStr = token_cols.size() > 4 ? csvUnescape(token_cols[4]) : "";
					std::string shieldStr = token_cols.size() > 5 ? csvUnescape(token_cols[5]) : "";

					Models::TokenType tokenType = Models::TokenType::PROGRESS;
					try {
						tokenType = Models::tokenTypeFromString(tokenTypeStr);
					} catch (...) {}

					auto coins = parseCoins(coinsStr);
					uint8_t victory = 0;
					uint8_t shield = 0;
					if (!victoryStr.empty()) {
						try { victory = static_cast<uint8_t>(std::stoi(victoryStr)); } catch (...) {}
					}
					if (!shieldStr.empty()) {
						try { shield = static_cast<uint8_t>(std::stoi(shieldStr)); } catch (...) {}
					}

					auto token = std::make_unique<Models::Token>(tokenType, tokenName, tokenDesc, coins, victory, shield);

					if (type == "Progress") {
						progressTokens.push_back(std::move(token));
					}
					else if (type == "Military") {
						militaryTokens.push_back(std::move(token));
					}
				}
			}
			else if (section == "Node" && columns.size() > 2) {
				std::vector<std::string> card_cols(columns.begin() + 2, columns.end());
				if (type == "Age1") {
					auto card = std::make_unique<Models::AgeCard>(ageCardFactory(card_cols));
					auto& nodes = const_cast<std::vector<std::shared_ptr<Node>>&>(board.getAge1Nodes());
					nodes.push_back(std::make_shared<Node>(std::move(card)));
				}
				else if (type == "Age2") {
					auto card = std::make_unique<Models::AgeCard>(ageCardFactory(card_cols));
					auto& nodes = const_cast<std::vector<std::shared_ptr<Node>>&>(board.getAge2Nodes());
					nodes.push_back(std::make_shared<Node>(std::move(card)));
				}
				else if (type == "Age3") {
					auto card = std::make_unique<Models::AgeCard>(ageCardFactory(card_cols));
					auto& nodes = const_cast<std::vector<std::shared_ptr<Node>>&>(board.getAge3Nodes());
					nodes.push_back(std::make_shared<Node>(std::move(card)));
				}
			}
			else if (section == "Unused" && columns.size() > 2) {
				std::vector<std::string> card_cols(columns.begin() + 2, columns.end());
				if (type == "Age1") {
					const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedAgeOneCards()).push_back(
					std::make_unique<Models::AgeCard>(ageCardFactory(card_cols))
);
				}
				else if (type == "Age2") {
					const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedAgeTwoCards()).push_back(std::make_unique<Models::AgeCard>(ageCardFactory(card_cols)));
				}
				else if (type == "Age3") {
					const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedAgeThreeCards()).push_back(std::make_unique<Models::AgeCard>(ageCardFactory(card_cols)));
				}
				else if (type == "Guild") {
					const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedGuildCards()).push_back(
					std::make_unique<Models::GuildCard>(guildCardFactory(card_cols))
);
				}
				else if (type == "Wonder") {
					auto wonderPtr = std::make_unique<Models::Wonder>(wonderFactory(card_cols));
					auto& wonders = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedWonders());
					wonders.push_back(std::move(wonderPtr));
				}
			}
			else if (section == "Discarded" && columns.size() > 2) {
				std::vector<std::string> card_cols(columns.begin() + 2, columns.end());
				auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
				discarded.push_back(std::make_unique<Models::AgeCard>(ageCardFactory(card_cols)));
			}
			else {
					// If the line doesn't belong to the board, put it back for the next >> operation
				in.seekg(-static_cast<std::streamoff>(line.length()) - 1, std::ios_base::cur);
				break;
			}
		}

		// Set the loaded tokens
		board.setProgressTokens(std::move(progressTokens));
		board.setMilitaryTokens(std::move(militaryTokens));

		return in;
	}
}