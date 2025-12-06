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

using namespace Core;

Board::Board() : bank{}, pawnTrack{}, pawnPos{ 0 } {}

Models::Bank& Board::getBank() { return bank; }

const std::vector<std::unique_ptr<Models::Card>>& Board::getUnusedAgeOneCards() const { return unusedAgeOneCards; }
void Board::setUnusedAgeOneCards(std::vector<std::unique_ptr<Models::Card>> v) { unusedAgeOneCards = std::move(v); }
const std::vector<std::unique_ptr<Models::Card>>& Board::getUnusedAgeTwoCards() const { return unusedAgeTwoCards; }
void Board::setUnusedAgeTwoCards(std::vector<std::unique_ptr<Models::Card>> v) { unusedAgeTwoCards = std::move(v); }
const std::vector<std::unique_ptr<Models::Card>>& Board::getUnusedAgeThreeCards() const { return unusedAgeThreeCards; }
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
const std::vector<std::unique_ptr<Models::Token>>& Board::getMilitaryTokens() const { return militaryTokens; }
void Board::setMilitaryTokens(std::vector<std::unique_ptr<Models::Token>> v) { militaryTokens = std::move(v); }

const std::vector<std::unique_ptr<Node>>& Board::getAge1Nodes() const { return age1Nodes; }
void Board::setAge1Nodes(std::vector<std::unique_ptr<Node>> v) { age1Nodes = std::move(v); }
const std::vector<std::unique_ptr<Node>>& Board::getAge2Nodes() const { return age2Nodes; }
void Board::setAge2Nodes(std::vector<std::unique_ptr<Node>> v) { age2Nodes = std::move(v); }
const std::vector<std::unique_ptr<Node>>& Board::getAge3Nodes() const { return age3Nodes; }
void Board::setAge3Nodes(std::vector<std::unique_ptr<Node>> v) { age3Nodes = std::move(v); }

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
	for (const auto& t : progressTokens) std::cout << t.get() << " ";
	std::cout << "\n\n";
}

static void displayMilitaryTokens(const std::vector< std::unique_ptr<Models::Token >>& militaryTokens)
{
	std::cout << "Military Tokens: ";
	for (const auto& t : militaryTokens) std::cout << t.get() << " ";
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
			std::cout << "[AgeI] card " << i << ":\n";
			p->displayCardInfo();
		}
	}

	std::cout << "Age II unused: " << age2.size() << " cards\n";
	for (size_t i = 0; i < age2.size(); ++i) {
		auto& p = age2[i];
		if (p) {
			std::cout << "[AgeII] card " << i << ":\n";
			p->displayCardInfo();
		}
	}

	std::cout << "Age III unused: " << age3.size() << " cards\n";
	for (size_t i = 0; i < age3.size(); ++i) {
		auto& p = age3[i];
		if (p) {
			std::cout << "[AgeIII] card " << i << ":\n";
			p->displayCardInfo();
		}
	}

	std::cout << "Guild unused: " << guilds.size() << " cards\n";
	for (size_t i = 0; i < guilds.size(); ++i) {
		auto& p = guilds[i];
		if (p) {
			std::cout << "[Guild] card " << i << ":\n";
			p->displayCardInfo();
		}
	}

	std::cout << "Wonders unused: " << wonders.size() << " cards\n";
	for (size_t i = 0; i < wonders.size(); ++i) {
		auto& p = wonders[i];
		if (p) {
			std::cout << "[Wonder] card " << i << ":\n";
			p->displayCardInfo();
		}
	}
}

static void displayAgeCards(const char* title, const std::vector<std::unique_ptr<Node>>& nodes)
{
	std::cout << title << " (" << nodes.size() << " nodes) ---\n";
	for (size_t i = 0; i < nodes.size(); ++i) {
		auto& n = nodes[i];
		if (!n) continue;
		Models::Card* c = n->getCard();
		if (!c) continue;
		std::cout << "Node[" << i << "]:\n";
		c->displayCardInfo();
		auto p1 = n->getParent1();
		auto p2 = n->getParent2();
		std::cout << " Parents=(" << (p1 ? p1->getCard()->GetName() : " ")
			<< "," << (p2 ? p2->getCard()->GetName() : " ") << ")\n";
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