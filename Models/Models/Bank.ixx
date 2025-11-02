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
   
    export class Bank
    {
       
    private:
        uint32_t m_coins = 0;
        std::vector<Token> m_tokens; // token pool; back() == top-of-deck
        std::mt19937 m_rng;          // RNG used for shuffling; seeded in ctor
    };
}