#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <iostream>
#include <random>

namespace Core
{
	struct CardChoice
	{
		size_t nodeIndex;
		int action; // 0=build, 1=sell, 2=wonder
		size_t wonderIndex; // only relevant if action==2
	};

	class IPlayerDecisionMaker
	{
	public:
		virtual ~IPlayerDecisionMaker() = default;

		virtual size_t selectCard(const std::vector<size_t>& availableIndices) = 0;
		virtual int selectCardAction() = 0;
		virtual size_t selectWonder(const std::vector<size_t>& wonderIndices) = 0;
		virtual uint8_t selectStartingPlayer() = 0;
		virtual size_t selectToken(const std::vector<std::string>& tokenNames) = 0;
	};

	class HumanDecisionMaker : public IPlayerDecisionMaker
	{
	public:
		HumanDecisionMaker() = default;
		virtual ~HumanDecisionMaker() = default;

		size_t selectCard(const std::vector<size_t>& availableIndices) override {
			if (availableIndices.empty()) return 0;
			int choice = 0;
			if (!(std::cin >> choice) || choice < 0 || static_cast<size_t>(choice) >= availableIndices.size()) {
				if (!std::cin) { std::cin.clear(); std::string discard; std::getline(std::cin, discard); }
				choice = 0;
			}
			return static_cast<size_t>(choice);
		}

		int selectCardAction() override {
			int action = 0;
			if (!(std::cin >> action) || action < 0 || action > 2) {
				if (!std::cin) { std::cin.clear(); std::string discard; std::getline(std::cin, discard); }
				action = 0;
			}
			return action;
		}

		size_t selectWonder(const std::vector<size_t>& wonderIndices) override {
			if (wonderIndices.empty()) return 0;
			int choice = 0;
			if (!(std::cin >> choice) || choice < 0 || static_cast<size_t>(choice) >= wonderIndices.size()) {
				if (!std::cin) { std::cin.clear(); std::string discard; std::getline(std::cin, discard); }
				choice = 0;
			}
			return static_cast<size_t>(choice);
		}

		uint8_t selectStartingPlayer() override {
			int choice = 0;
			if (!(std::cin >> choice) || (choice != 0 && choice != 1)) {
				if (!std::cin) { std::cin.clear(); std::string discard; std::getline(std::cin, discard); }
				choice = 0;
			}
			return static_cast<uint8_t>(choice);
		}

		size_t selectToken(const std::vector<std::string>& tokenNames) override {
			if (tokenNames.empty()) return 0;
			int choice = 0;
			if (!(std::cin >> choice) || choice < 0 || static_cast<size_t>(choice) >= tokenNames.size()) {
				if (!std::cin) { std::cin.clear(); std::string discard; std::getline(std::cin, discard); }
				choice = 0;
			}
			return static_cast<size_t>(choice);
		}
	};

	class RandomDecisionMaker : public IPlayerDecisionMaker
	{
	public:
		RandomDecisionMaker() = default;
		virtual ~RandomDecisionMaker() = default;

		size_t selectCard(const std::vector<size_t>& availableIndices) override {
			if (availableIndices.empty()) return 0;
			static std::random_device rd;
			static std::mt19937 gen(rd());
			std::uniform_int_distribution<> dis(0, static_cast<int>(availableIndices.size()) - 1);
			return static_cast<size_t>(dis(gen));
		}

		int selectCardAction() override {
			static std::random_device rd;
			static std::mt19937 gen(rd());
			std::uniform_int_distribution<> dis(0, 2);
			return dis(gen);
		}

		size_t selectWonder(const std::vector<size_t>& wonderIndices) override {
			if (wonderIndices.empty()) return 0;
			static std::random_device rd;
			static std::mt19937 gen(rd());
			std::uniform_int_distribution<> dis(0, static_cast<int>(wonderIndices.size()) - 1);
			return static_cast<size_t>(dis(gen));
		}

		uint8_t selectStartingPlayer() override {
			static std::random_device rd;
			static std::mt19937 gen(rd());
			std::uniform_int_distribution<> dis(0, 1);
			return static_cast<uint8_t>(dis(gen));
		}

		size_t selectToken(const std::vector<std::string>& tokenNames) override {
			if (tokenNames.empty()) return 0;
			static std::random_device rd;
			static std::mt19937 gen(rd());
			std::uniform_int_distribution<> dis(0, static_cast<int>(tokenNames.size()) - 1);
			return static_cast<size_t>(dis(gen));
		}
	};
}
