export module Models.Bank;

// Import the token module (you already created Models.Tokens)
import Models.Tokens;

import <vector>;
import <random>;
import <algorithm>;
import <chrono>;
import <cstdint>;
import <utility>;

export namespace Models
{
    // Bank: responsible for coin supply and token pool management.
    // - Keeps a coin counter and a vector of Tokens acting as the token pool.
    // - DrawToken removes from the back (top of the deck). ReturnToken inserts at the front (bottom of the deck).
    // - ShuffleTokens uses an internal mt19937 RNG seeded in the ctor (deterministic if seed provided).
    export class Bank
    {
    public:
        // startingCoins: initial coin supply in the bank.
        // rngSeed: pass non-zero for deterministic shuffles (useful for tests).
        explicit Bank(uint32_t startingCoins = 0, unsigned int rngSeed = 0)
            : m_coins(startingCoins)
        {
            if (rngSeed == 0)
            {
                // Default: seed from high-resolution clock for non-deterministic behavior.
                rngSeed = static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
            }
            m_rng.seed(rngSeed);
        }

        // Coin operations --------------------------------------------------

        // Returns current number of coins in the bank.
        uint32_t GetCoins() const noexcept { return m_coins; }

        // Try to withdraw `amount` coins. Returns true if the bank had enough coins
        // and the withdrawal succeeded; no partial withdrawals.
        // Note: not thread-safe; synchronize externally if used concurrently.
        bool TryWithdraw(uint32_t amount) noexcept
        {
            if (amount == 0) return true;
            if (m_coins < amount) return false;
            m_coins -= amount;
            return true;
        }

        // Deposit `amount` coins into the bank.
        void Deposit(uint32_t amount) noexcept { m_coins += amount; }

        // Token pool operations --------------------------------------------

        // Initialize token pool by copying tokens.
        void InitializeTokens(const std::vector<Token>& tokens) { m_tokens = tokens; }

        // Initialize token pool by moving tokens (avoids copy).
        void InitializeTokens(std::vector<Token>&& tokens) { m_tokens = std::move(tokens); }

        // Helper that fills tokens with the default token set provided by the Tokens module.
        void FillWithDefaultTokens() { m_tokens = CreateDefaultTokenSet(); }

        // Shuffle the token pool. Uses the Bank's RNG.
        // After shuffle, DrawToken will pop from the back() as the "top".
        void ShuffleTokens() { std::shuffle(m_tokens.begin(), m_tokens.end(), m_rng); }

        // Draws a single token from the top of the pool.
        // Returns true and writes the token to outToken when successful.
        // Returns false if the pool is empty.
        bool DrawToken(Token& outToken)
        {
            if (m_tokens.empty()) return false;
            outToken = std::move(m_tokens.back());
            m_tokens.pop_back();
            return true;
        }

        // Draw multiple tokens (useful for setup or multi-draw actions).
        // Returns vector with up to `count` tokens (fewer if pool runs out).
        std::vector<Token> DrawTokens(size_t count)
        {
            std::vector<Token> result;
            result.reserve(count);
            while (count-- > 0 && !m_tokens.empty())
            {
                result.push_back(std::move(m_tokens.back()));
                m_tokens.pop_back();
            }
            return result;
        }

        // Return a token to the bottom of the pool (insert at front).
        // Using bottom semantics is intentional: returned tokens won't be drawn immediately.
        void ReturnToken(const Token& token) { m_tokens.insert(m_tokens.begin(), token); }

        // Number of tokens remaining in the pool.
        size_t TokenCount() const noexcept { return m_tokens.size(); }

        // Convenience: clear the token pool.
        void ClearTokens() noexcept { m_tokens.clear(); }

    private:
        uint32_t m_coins = 0;
        std::vector<Token> m_tokens; // token pool; back() == top-of-deck
        std::mt19937 m_rng;          // RNG used for shuffling; seeded in ctor
    };
}