module Core.GameStateSerializer;
import Core.GameState;
import Core.Board;
import Core.Player;
import Core.Node;
import Models.Card;
import Models.AgeCard;
import Models.GuildCard;
import Models.Wonder;
import Models.Token;
import Models.ResourceType;
import Models.ColorType;
import Models.ScientificSymbolType;
import Models.TradeRuleType;
import <iostream>;
import <algorithm>;
import <bitset>;
import <filesystem>;

namespace Core {
    
    int GameStateSerializer::s_currentSaveNumber = 0;
    GameStateMetadata GameStateSerializer::s_gameMetadata;
    
    void GameStateSerializer::JsonWriter::writeIndent() {
        for (int i = 0; i < indent; ++i) {
            buffer << "  ";
        }
    }
    
    std::string GameStateSerializer::JsonWriter::escapeString(const std::string& str) {
        std::string result;
        for (char c : str) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c;
            }
        }
        return result;
    }
    
    void GameStateSerializer::JsonWriter::startObject() {
        buffer << "{\n";
        indent++;
    }
    
    void GameStateSerializer::JsonWriter::endObject(bool addComma) {
        buffer << "\n";
        indent--;
        writeIndent();
        buffer << "}" << (addComma ? "," : "") << "\n";
    }
    
    void GameStateSerializer::JsonWriter::startArray(const std::string& key) {
        writeIndent();
        buffer << "\"" << key << "\": [\n";
        indent++;
    }
    
    void GameStateSerializer::JsonWriter::endArray(bool addComma) {
        buffer << "\n";
        indent--;
        writeIndent();
        buffer << "]" << (addComma ? "," : "") << "\n";
    }
    
    void GameStateSerializer::JsonWriter::writeKey(const std::string& key) {
        writeIndent();
        buffer << "\"" << key << "\": ";
    }
    
    void GameStateSerializer::JsonWriter::writeString(const std::string& key, const std::string& value, bool addComma) {
        writeIndent();
        buffer << "\"" << key << "\": \"" << escapeString(value) << "\"" << (addComma ? "," : "") << "\n";
    }
    
    void GameStateSerializer::JsonWriter::writeInt(const std::string& key, int value, bool addComma) {
        writeIndent();
        buffer << "\"" << key << "\": " << value << (addComma ? "," : "") << "\n";
    }
    
    void GameStateSerializer::JsonWriter::writeBool(const std::string& key, bool value, bool addComma) {
        writeIndent();
        buffer << "\"" << key << "\": " << (value ? "true" : "false") << (addComma ? "," : "") << "\n";
    }
    
    void GameStateSerializer::JsonWriter::writeStringValue(const std::string& value, bool addComma) {
        writeIndent();
        buffer << "\"" << escapeString(value) << "\"" << (addComma ? "," : "") << "\n";
    }
    
    void GameStateSerializer::JsonWriter::writeIntValue(int value, bool addComma) {
        writeIndent();
        buffer << value << (addComma ? "," : "") << "\n";
    }
    
    std::string GameStateSerializer::getSaveFilename(int number) {
        return "gamestate_" + std::to_string(number) + ".json";
    }
    
    int GameStateSerializer::extractNumberFromFilename(const std::string& filename) {
        size_t underscorePos = filename.find('_');
        size_t dotPos = filename.find(".json");
        
        if (underscorePos != std::string::npos && dotPos != std::string::npos) {
            std::string numStr = filename.substr(underscorePos + 1, dotPos - underscorePos - 1);
            try {
                return std::stoi(numStr);
            } catch (...) {
                return 0;
            }
        }
        return 0;
    }
    
    std::vector<int> GameStateSerializer::getAllSaveNumbers() {
        std::vector<int> saveNumbers;
        try {
            for (const auto& entry : std::filesystem::directory_iterator(".")) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    if (filename.find("gamestate_") == 0 && filename.find(".json") != std::string::npos) {
                        int num = extractNumberFromFilename(filename);
                        if (num > 0) {
                            saveNumbers.push_back(num);
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Failed to list save files: " << e.what() << "\n";
        }
        
        std::sort(saveNumbers.begin(), saveNumbers.end());
        return saveNumbers;
    }
    
    int GameStateSerializer::getHighestSaveNumber() {
        auto saveNumbers = getAllSaveNumbers();
        return saveNumbers.empty() ? 0 : saveNumbers.back();
    }
    
    int GameStateSerializer::getCurrentSaveNumber() {
        return s_currentSaveNumber;
    }
    
    void GameStateSerializer::setCurrentSaveNumber(int number) {
        s_currentSaveNumber = number;
    }
    
    const GameStateMetadata& GameStateSerializer::getMetadata() {
        return s_gameMetadata;
    }
    
    void GameStateSerializer::setMetadata(const GameStateMetadata& metadata) {
        s_gameMetadata = metadata;
    }
    
    void GameStateSerializer::setGameMode(int mode, bool training) {
        s_gameMetadata.gameMode = mode;
        s_gameMetadata.trainingMode = training;
    }
    
    void GameStateSerializer::setPlayerPlaystyles(Playstyle p1, Playstyle p2) {
        s_gameMetadata.player1Playstyle = p1;
        s_gameMetadata.player2Playstyle = p2;
    }
    
    void GameStateSerializer::setCurrentPhase(int phase, int round, bool isP1Turn) {
        s_gameMetadata.currentPhase = phase;
        s_gameMetadata.currentRound = round;
        s_gameMetadata.isPlayer1Turn = isP1Turn;
    }
    
    void GameStateSerializer::recordLastAction(const std::string& playerName, const std::string& actionType,
                                               const std::string& cardName, const std::vector<std::string>& effects) {
        s_gameMetadata.lastAction.playerName = playerName;
        s_gameMetadata.lastAction.actionType = actionType;
        s_gameMetadata.lastAction.cardName = cardName;
        s_gameMetadata.lastAction.effectsApplied = effects;
    }
    
    bool GameStateSerializer::deleteSave(int saveNumber) {
        try {
            std::string filename = getSaveFilename(saveNumber);
            if (std::filesystem::remove(filename)) {
                std::cout << "[JSON] Deleted save: " << filename << "\n";
                return true;
            }
            return false;
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Failed to delete save: " << e.what() << "\n";
            return false;
        }
    }
    
    bool GameStateSerializer::saveGame(bool isNewGame) {
        int saveNumber;
        
        if (isNewGame) {
            saveNumber = getHighestSaveNumber() + 1;
            s_currentSaveNumber = saveNumber;
        } else {
            saveNumber = s_currentSaveNumber;
            if (saveNumber == 0) {
                saveNumber = getHighestSaveNumber() + 1;
                s_currentSaveNumber = saveNumber;
            }
        }
        
        std::string filename = getSaveFilename(saveNumber);
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "[ERROR] Could not open " << filename << " for writing\n";
            return false;
        }
        
        JsonWriter writer;
        writer.startObject();
        
        writer.writeInt("saveNumber", saveNumber);
        
        serializeGameMetadata(writer);
        serializeBoardState(writer);
        serializeTokens(writer);
        serializeTreeNodes(writer, "age1Tree", 0);
        serializeTreeNodes(writer, "age2Tree", 1);
        serializeTreeNodes(writer, "age3Tree", 2);
        serializePlayers(writer);
        serializeUnusedCards(writer);
        serializeDiscardedCards(writer);
        
        writer.endObject(false);
        
        file << writer.toString();
        file.close();
        
        std::cout << "[JSON] Game saved to " << filename << " (Save #" << saveNumber << ")\n";
        std::cout << "       Phase: " << GameState::getInstance().getCurrentPhase() << ", Round: " << GameState::getInstance().getCurrentRound() << "\n";
        return true;
    }
    
    bool GameStateSerializer::loadGame(int saveNumber) {
        std::string filename = getSaveFilename(saveNumber);
        
        if (!std::filesystem::exists(filename)) {
            std::cerr << "[ERROR] Save file not found: " << filename << "\n";
            return false;
        }
        
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "[ERROR] Could not open " << filename << " for reading\n";
            return false;
        }
        
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        
        s_currentSaveNumber = saveNumber;
        
        std::cout << "[JSON] Loaded save #" << saveNumber << " from " << filename << "\n";
        std::cout << "[INFO] Deserialization TODO - game state reconstruction needed\n";
        
        return true;
    }
    
    void GameStateSerializer::serializeGameMetadata(JsonWriter& writer) {
        writer.writeKey("gameMetadata");
        writer.startObject();
        
        writer.writeInt("gameMode", s_gameMetadata.gameMode);
        writer.writeBool("trainingMode", s_gameMetadata.trainingMode);
        writer.writeString("player1Playstyle", playstyleToString(s_gameMetadata.player1Playstyle));
        writer.writeString("player2Playstyle", playstyleToString(s_gameMetadata.player2Playstyle));
        
        writer.writeInt("currentPhase", s_gameMetadata.currentPhase);
        writer.writeInt("currentRound", s_gameMetadata.currentRound);
        writer.writeBool("isPlayer1Turn", s_gameMetadata.isPlayer1Turn);
        
        const auto& lastAction = s_gameMetadata.lastAction;
        writer.writeKey("lastAction");
        writer.startObject();
        writer.writeString("playerName", lastAction.playerName);
        writer.writeString("actionType", lastAction.actionType);
        writer.writeString("cardName", lastAction.cardName);
        writer.writeInt("round", lastAction.round);
        writer.writeInt("phase", lastAction.phase);
        
        writer.startArray("effectsApplied");
        for (size_t i = 0; i < lastAction.effectsApplied.size(); ++i) {
            writer.writeStringValue(lastAction.effectsApplied[i], 
                                   i < lastAction.effectsApplied.size() - 1);
        }
        writer.endArray(false);
        
        writer.endObject(false);
        writer.endObject(true);
    }
    
    void GameStateSerializer::serializeBoardState(JsonWriter& writer) {
        const auto& board = Board::getInstance();
        
        writer.writeKey("board");
        writer.startObject();
        writer.writeInt("pawnPosition", static_cast<int>(board.getPawnPos()));
        writer.writeString("pawnTrack", board.getPawnTrack().to_string(), false);
        writer.endObject(true);
    }
    
    void GameStateSerializer::serializeTokens(JsonWriter& writer) {
        const auto& board = Board::getInstance();
        
        writer.startArray("progressTokens");
        const auto& progressTokens = board.getProgressTokens();
        for (size_t i = 0; i < progressTokens.size(); ++i) {
            if (progressTokens[i]) {
                writer.writeIndent();
                writer.buffer << "{\n";
                writer.indent++;
                writer.writeString("name", progressTokens[i]->getName());
                writer.writeString("description", progressTokens[i]->getDescription());
                writer.writeInt("victoryPoints", static_cast<int>(progressTokens[i]->getVictoryPoints()));
                writer.writeInt("shieldPoints", static_cast<int>(progressTokens[i]->getShieldPoints()), false);
                writer.indent--;
                writer.writeIndent();
                writer.buffer << "}" << (i < progressTokens.size() - 1 ? "," : "") << "\n";
            }
        }
        writer.endArray(true);
        
        writer.startArray("militaryTokens");
        const auto& militaryTokens = board.getMilitaryTokens();
        for (size_t i = 0; i < militaryTokens.size(); ++i) {
            if (militaryTokens[i]) {
                writer.writeIndent();
                writer.buffer << "{\n";
                writer.indent++;
                writer.writeString("name", militaryTokens[i]->getName());
                writer.writeString("description", militaryTokens[i]->getDescription(), false);
                writer.indent--;
                writer.writeIndent();
                writer.buffer << "}" << (i < militaryTokens.size() - 1 ? "," : "") << "\n";
            }
        }
        writer.endArray(true);
    }
    
    void GameStateSerializer::serializeTreeNodes(JsonWriter& writer, const std::string& treeKey, int ageIndex) {
        const auto& board = Board::getInstance();
        const std::vector<std::shared_ptr<Node>>* nodes = nullptr;
        
        switch (ageIndex) {
            case 0: nodes = &board.getAge1Nodes(); break;
            case 1: nodes = &board.getAge2Nodes(); break;
            case 2: nodes = &board.getAge3Nodes(); break;
            default: return;
        }
        
        writer.startArray(treeKey);
        
        for (size_t i = 0; i < nodes->size(); ++i) {
            const auto& node = (*nodes)[i];
            
            writer.writeIndent();
            writer.buffer << "{\n";
            writer.indent++;
            
            writer.writeInt("index", static_cast<int>(i));
            
            auto* card = node ? node->getCard() : nullptr;
            writer.writeString("cardName", card ? card->getName() : "EMPTY");
            writer.writeBool("isEmpty", !card);
            writer.writeBool("isAvailable", card && card->isAvailable());
            writer.writeBool("isVisible", card && card->isVisible());
            
            writer.writeKey("parents");
            writer.buffer << "[";
            if (node) {
                auto p1 = node->getParent1().lock();
                auto p2 = node->getParent2().lock();
                std::vector<int> parentIndices;
                
                for (size_t j = 0; j < nodes->size(); ++j) {
                    if ((*nodes)[j]) {
                        if ((*nodes)[j].get() == p1.get()) parentIndices.push_back(static_cast<int>(j));
                        if ((*nodes)[j].get() == p2.get()) parentIndices.push_back(static_cast<int>(j));
                    }
                }
                
                for (size_t k = 0; k < parentIndices.size(); ++k) {
                    writer.buffer << parentIndices[k];
                    if (k < parentIndices.size() - 1) writer.buffer << ", ";
                }
            }
            writer.buffer << "]\n";
            
            writer.indent--;
            writer.writeIndent();
            writer.buffer << "}" << (i < nodes->size() - 1 ? "," : "") << "\n";
        }
        
        writer.endArray(true);
    }
    
    void GameStateSerializer::serializePlayers(JsonWriter& writer) {
        const auto& gameState = GameState::getInstance();
        
        auto serializePlayer = [&writer](const std::shared_ptr<Player>& player, const std::string& playerKey, bool isLast) {
            writer.writeKey(playerKey);
            writer.startObject();
            
            if (player && player->m_player) {
                auto* p = player->m_player.get();
                
                writer.writeInt("id", p->getkPlayerId());
                writer.writeString("username", p->getPlayerUsername());
                
                const auto& coins = p->getRemainingCoins();
                writer.writeKey("remainingCoins");
                writer.buffer << "{";
                writer.buffer << "\"gold\": " << static_cast<int>(std::get<0>(coins)) << ", ";
                writer.buffer << "\"silver\": " << static_cast<int>(std::get<1>(coins)) << ", ";
                writer.buffer << "\"bronze\": " << static_cast<int>(std::get<2>(coins));
                writer.buffer << "},\n";
                writer.writeInt("totalCoins", p->totalCoins(coins));
                
                writer.startArray("permanentResources");
                const auto& permResources = p->getOwnedPermanentResources();
                bool firstRes = true;
                for (const auto& [resType, amount] : permResources) {
                    if (amount > 0) {
                        if (!firstRes) writer.buffer << ",\n";
                        writer.writeIndent();
                        writer.buffer << "{\"type\": \"" << Models::ResourceTypeToString(resType) 
                                     << "\", \"amount\": " << static_cast<int>(amount) << "}";
                        firstRes = false;
                    }
                }
                if (!firstRes) writer.buffer << "\n";
                writer.endArray(true);
                
                writer.startArray("tradingResources");
                const auto& tradeResources = p->getOwnedTradingResources();
                firstRes = true;
                for (const auto& [resType, amount] : tradeResources) {
                    if (amount > 0) {
                        if (!firstRes) writer.buffer << ",\n";
                        writer.writeIndent();
                        writer.buffer << "{\"type\": \"" << Models::ResourceTypeToString(resType) 
                                     << "\", \"amount\": " << static_cast<int>(amount) << "}";
                        firstRes = false;
                    }
                }
                if (!firstRes) writer.buffer << "\n";
                writer.endArray(true);
                
                writer.startArray("scientificSymbols");
                const auto& symbols = p->getOwnedScientificSymbols();
                bool firstSym = true;
                for (const auto& [symType, amount] : symbols) {
                    if (amount > 0) {
                        if (!firstSym) writer.buffer << ",\n";
                        writer.writeIndent();
                        writer.buffer << "{\"type\": \"" << Models::ScientificSymbolTypeToString(symType) 
                                     << "\", \"amount\": " << static_cast<int>(amount) << "}";
                        firstSym = false;
                    }
                }
                if (!firstSym) writer.buffer << "\n";
                writer.endArray(true);
                
                writer.startArray("tradeRules");
                const auto& tradeRules = p->getTradeRules();
                bool firstRule = true;
                for (const auto& [ruleType, isActive] : tradeRules) {
                    if (isActive) {
                        if (!firstRule) writer.buffer << ",\n";
                        writer.writeIndent();
                        writer.buffer << "\"" << Models::TradeRuleTypeToString(ruleType) << "\"";
                        firstRule = false;
                    }
                }
                if (!firstRule) writer.buffer << "\n";
                writer.endArray(true);
                
                const auto& tokensOwned = p->getTokensOwned();
                writer.writeString("tokensOwnedBitset", tokensOwned.to_string());
                
                writer.startArray("builtCards");
                const auto& builtCards = p->getOwnedCards();
                for (size_t i = 0; i < builtCards.size(); ++i) {
                    writer.writeStringValue(builtCards[i]->getName(), i < builtCards.size() - 1);
                }
                writer.endArray(true);
                
                writer.startArray("wonders");
                auto& wonders = p->getOwnedWonders();
                for (size_t i = 0; i < wonders.size(); ++i) {
                    writer.writeStringValue(wonders[i]->getName(), i < wonders.size() - 1);
                }
                writer.endArray(true);
                
                writer.startArray("ownedTokens");
                const auto& ownedTokens = p->getOwnedTokens();
                for (size_t i = 0; i < ownedTokens.size(); ++i) {
                    writer.writeStringValue(ownedTokens[i]->getName(), i < ownedTokens.size() - 1);
                }
                writer.endArray(true);
                
                const auto& points = p->getPoints();
                writer.writeKey("points");
                writer.startObject();
                writer.writeInt("military", points.m_militaryVictoryPoints);
                writer.writeInt("building", points.m_buildingVictoryPoints);
                writer.writeInt("wonder", points.m_wonderVictoryPoints);
                writer.writeInt("progress", points.m_progressVictoryPoints);
                writer.writeInt("total", points.totalVictoryPoints(), false);
                writer.endObject(false);
            }
            
            writer.endObject(isLast);
        };
        
        serializePlayer(gameState.GetPlayer1(), "player1", false);
        serializePlayer(gameState.GetPlayer2(), "player2", false);
    }
    
    void GameStateSerializer::serializeUnusedCards(JsonWriter& writer) {
        const auto& board = Board::getInstance();
        
        writer.startArray("unusedCards");
        
        bool hasContent = false;
        
        for (const auto& card : board.getUnusedAgeOneCards()) {
            if (card) {
                if (hasContent) writer.buffer << ",\n";
                writer.writeIndent();
                writer.buffer << "{\"age\": \"AGE_I\", \"name\": \"" << writer.escapeString(card->getName()) << "\"}";
                hasContent = true;
            }
        }
        
        for (const auto& card : board.getUnusedAgeTwoCards()) {
            if (card) {
                if (hasContent) writer.buffer << ",\n";
                writer.writeIndent();
                writer.buffer << "{\"age\": \"AGE_II\", \"name\": \"" << writer.escapeString(card->getName()) << "\"}";
                hasContent = true;
            }
        }
        
        for (const auto& card : board.getUnusedAgeThreeCards()) {
            if (card) {
                if (hasContent) writer.buffer << ",\n";
                writer.writeIndent();
                writer.buffer << "{\"age\": \"AGE_III\", \"name\": \"" << writer.escapeString(card->getName()) << "\"}";
                hasContent = true;
            }
        }
        
        for (const auto& card : board.getUnusedGuildCards()) {
            if (card) {
                if (hasContent) writer.buffer << ",\n";
                writer.writeIndent();
                writer.buffer << "{\"age\": \"GUILD\", \"name\": \"" << writer.escapeString(card->getName()) << "\"}";
                hasContent = true;
            }
        }
        
        const auto& wonders = board.getUnusedWonders();
        for (size_t i = 0; i < wonders.size(); ++i) {
            if (wonders[i]) {
                if (hasContent) writer.buffer << ",\n";
                writer.writeIndent();
                writer.buffer << "{\"age\": \"WONDER\", \"name\": \"" << writer.escapeString(wonders[i]->getName()) << "\"}";
                hasContent = true;
            }
        }
        
        if (hasContent) writer.buffer << "\n";
        writer.endArray(true);
    }
    
    void GameStateSerializer::serializeDiscardedCards(JsonWriter& writer) {
        const auto& board = Board::getInstance();
        
        writer.startArray("discardedCards");
        const auto& discarded = board.getDiscardedCards();
        for (size_t i = 0; i < discarded.size(); ++i) {
            if (discarded[i]) {
                writer.writeStringValue(discarded[i]->getName(), i < discarded.size() - 1);
            }
        }
        writer.endArray(false);
    }
    
    void GameStateSerializer::saveVictoryToJson(const GameState& state, std::string& json)
    {
        if (state.hasEnded()) {
            std::string victorySection = R"(
    "victory": {
        "gameEnded": true,
        "winnerId": )" + std::to_string(state.getWinnerId()) + R"(,
        "victoryType": ")" + state.getVictoryType() + R"(",
        "winnerScore": )" + std::to_string(state.getWinnerScore()) + R"(,
        "loserScore": )" + std::to_string(state.getLoserScore()) + R"(
    })";
            size_t closePos = json.rfind('}');
            if (closePos != std::string::npos) {
                json.insert(closePos, ",\n" + victorySection);
            }
        }
    }

    void GameStateSerializer::loadVictoryFromJson(GameState& state, const std::string& json)
    {
        size_t victoryPos = json.find("\"victory\"");
        if (victoryPos != std::string::npos) {
            size_t gameEndedPos = json.find("\"gameEnded\"", victoryPos);
            size_t truePos = json.find("true", gameEndedPos);
            
            if (truePos != std::string::npos) {
                size_t winnerIdPos = json.find("\"winnerId\"", victoryPos);
                size_t winnerIdValue = json.find(":", winnerIdPos) + 1;
                int winnerId = std::stoi(json.substr(winnerIdValue));
                
                size_t victoryTypePos = json.find("\"victoryType\"", victoryPos);
                size_t typeStart = json.find("\"", victoryTypePos + 14) + 1;
                size_t typeEnd = json.find("\"", typeStart);
                std::string victoryType = json.substr(typeStart, typeEnd - typeStart);
                
                size_t winnerScorePos = json.find("\"winnerScore\"", victoryPos);
                size_t scoreValue = json.find(":", winnerScorePos) + 1;
                int winnerScore = std::stoi(json.substr(scoreValue));
                
                size_t loserScorePos = json.find("\"loserScore\"", victoryPos);
                size_t loserValue = json.find(":", loserScorePos) + 1;
                int loserScore = std::stoi(json.substr(loserValue));
                
                state.setVictory(winnerId, victoryType, winnerScore, loserScore);
            }
        }
    }
}
