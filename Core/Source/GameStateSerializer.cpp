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
import <stdexcept>;
import <unordered_map>;
import <sstream>;
import Core.CardCsvParser;

namespace Core {
    static Playstyle playstyleFromString(const std::string& s) {
        if (s == "Britney" || s == "BRITNEY") return Playstyle::BRITNEY;
        if (s == "Spears" || s == "SPEARS") return Playstyle::SPEARS;
        return Playstyle::BRITNEY;
    }
    
    static std::string readFile(const std::string& path) {
        std::ifstream f(path);
        if (!f.is_open()) throw std::runtime_error("Cannot open " + path);
        return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    }
    
    static int extractInt(const std::string& json, const std::string& key, int def = 0) {
        auto pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return def;
        pos = json.find(':', pos);
        if (pos == std::string::npos) return def;
        return std::stoi(json.substr(pos + 1));
    }
    
    static bool extractBool(const std::string& json, const std::string& key, bool def = false) {
        auto pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return def;
        pos = json.find(':', pos);
        if (pos == std::string::npos) return def;
        auto tail = json.substr(pos + 1);
        return tail.find("true") < tail.find_first_of(",}\n");
    }
    
    static std::string extractString(const std::string& json, const std::string& key, const std::string& def = {}) {
        auto pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return def;
        pos = json.find('"', pos + key.size() + 2);
        if (pos == std::string::npos) return def;
        auto end = json.find('"', pos + 1);
        if (end == std::string::npos) return def;
        return json.substr(pos + 1, end - pos - 1);
    }
    
    static size_t findMatchingBracket(const std::string& s, size_t openPos) {
        int depth = 0;
        for (size_t i = openPos; i < s.size(); ++i) {
            if (s[i] == '[') depth++;
            else if (s[i] == ']') {
                depth--;
                if (depth == 0) return i;
            }
        }
        return std::string::npos;
    }
    
    static size_t findMatchingBrace(const std::string& s, size_t openPos) {
        int depth = 0;
        for (size_t i = openPos; i < s.size(); ++i) {
            if (s[i] == '{') depth++;
            else if (s[i] == '}') {
                depth--;
                if (depth == 0) return i;
            }
        }
        return std::string::npos;
    }
    
    static std::vector<std::string> splitCSVLineLocal(const std::string& line) {
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
    
    struct CardCatalog {
        std::unordered_map<std::string, std::vector<std::string>> ageRows;
        std::unordered_map<std::string, std::vector<std::string>> guildRows;
        std::unordered_map<std::string, std::vector<std::string>> wonderRows;
        std::unordered_map<std::string, std::unique_ptr<Models::Token>> tokens;
    };
    
    static void loadCardsFromFile(const std::string& path, CardCatalog& cat, int type) {
        std::ifstream ifs(path);
        if (!ifs.is_open()) return;
        std::string header;
        std::getline(ifs, header);
        std::string line;
        while (std::getline(ifs, line)) {
            if (line.empty()) continue;
            auto cols = splitCSVLineLocal(line);
            if (type == 0) {
                auto c = ageCardFactory(cols);
                cat.ageRows[c.getName()] = cols;
            } else if (type == 1) {
                auto g = guildCardFactory(cols);
                cat.guildRows[g.getName()] = cols;
            } else {
                auto w = wonderFactory(cols);
                cat.wonderRows[w.getName()] = cols;
            }
        }
    }
    
    static std::string pickPath(const std::vector<std::string>& cands) {
        for (const auto& p : cands) {
            try { if (std::filesystem::exists(p)) return p; } catch (...) {}
        }
        return {};
    }
    
    static CardCatalog buildCatalog() {
        CardCatalog cat;
        const std::vector<std::string> ageCandidates = {
            "Core/Config/AgeCards.csv","../Core/Config/AgeCards.csv","../../Core/Config/AgeCards.csv",
            "AgeCards.csv","Resources/AgeCards.csv","Core/Resources/AgeCards.csv",
            "../Core/Resources/AgeCards.csv","../ModelsDLL/AgeCards.csv","ModelsDLL/AgeCards.csv","../Models/AgeCards.csv"
        };
        const std::vector<std::string> guildCandidates = {
            "Core/Config/Guilds.csv","../Core/Config/Guilds.csv","../../Core/Config/Guilds.csv",
            "Guilds.csv","Resources/Guilds.csv","Core/Resources/Guilds.csv",
            "../Core/Resources/Guilds.csv","../ModelsDLL/Guilds.csv","ModelsDLL/Guilds.csv","../Models/Guilds.csv"
        };
        const std::vector<std::string> wonderCandidates = {
            "Core/Config/Wonders.csv","../Core/Config/Wonders.csv","../../Core/Config/Wonders.csv",
            "Wonders.csv","Resources/Wonders.csv","Core/Resources/Wonders.csv",
            "../Core/Resources/Wonders.csv","../ModelsDLL/Wonders.csv","ModelsDLL/Wonders.csv","../Models/Wonders.csv"
        };
        const std::vector<std::string> tokenCandidates = {
            "Core/Config/Tokens.csv","../Core/Config/Tokens.csv","../../Core/Config/Tokens.csv",
            "Tokens.csv","Resources/Tokens.csv","Core/Resources/Tokens.csv",
            "../Core/Resources/Tokens.csv","../ModelsDLL/Tokens.csv","ModelsDLL/Tokens.csv","../Models/Tokens.csv"
        };
        if (auto p = pickPath(ageCandidates); !p.empty()) loadCardsFromFile(p, cat, 0);
        if (auto p = pickPath(guildCandidates); !p.empty()) loadCardsFromFile(p, cat, 1);
        if (auto p = pickPath(wonderCandidates); !p.empty()) loadCardsFromFile(p, cat, 2);
        if (auto p = pickPath(tokenCandidates); !p.empty()) {
            auto toks = parseTokensFromCSV(p);
            for (auto& t : toks) if (t) cat.tokens[t->getName()] = std::move(t);
        }
        return cat;
    }
    
    static std::unique_ptr<Models::Card> cloneCard(CardCatalog& cat, const std::string& name) {
        if (auto it = cat.ageRows.find(name); it != cat.ageRows.end()) {
            auto c = ageCardFactory(it->second);
            return std::make_unique<Models::AgeCard>(std::move(c));
        }
        if (auto it = cat.guildRows.find(name); it != cat.guildRows.end()) {
            auto g = guildCardFactory(it->second);
            return std::make_unique<Models::GuildCard>(std::move(g));
        }
        if (auto it = cat.wonderRows.find(name); it != cat.wonderRows.end()) {
            auto w = wonderFactory(it->second);
            return std::make_unique<Models::Wonder>(std::move(w));
        }
        return nullptr;
    }
    
    static std::unique_ptr<Models::Token> cloneToken(CardCatalog& cat, const std::string& name) {
        auto it = cat.tokens.find(name);
        if (it == cat.tokens.end()) return nullptr;
        return std::make_unique<Models::Token>(*it->second);
    }
    
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
        std::string json = readFile(filename);
        s_currentSaveNumber = saveNumber;
        CardCatalog catalog = buildCatalog();
        GameState& gs = GameState::getInstance();
        Board& board = Board::getInstance();
        
        s_gameMetadata.gameMode = extractInt(json, "gameMode", 1);
        s_gameMetadata.trainingMode = extractBool(json, "trainingMode", false);
        s_gameMetadata.player1Playstyle = playstyleFromString(extractString(json, "player1Playstyle", "Britney"));
        s_gameMetadata.player2Playstyle = playstyleFromString(extractString(json, "player2Playstyle", "Britney"));
        s_gameMetadata.currentPhase = extractInt(json, "currentPhase", 1);
        s_gameMetadata.currentRound = extractInt(json, "currentRound", 1);
        s_gameMetadata.isPlayer1Turn = extractBool(json, "isPlayer1Turn", true);
        s_gameMetadata.lastAction.playerName = extractString(json, "playerName");
        s_gameMetadata.lastAction.actionType = extractString(json, "actionType");
        s_gameMetadata.lastAction.cardName = extractString(json, "cardName");
        s_gameMetadata.lastAction.round = extractInt(json, "round", 0);
        s_gameMetadata.lastAction.phase = extractInt(json, "phase", 0);
        
        gs.setGameMode(s_gameMetadata.gameMode, s_gameMetadata.trainingMode);
        gs.setPlayerPlaystyles(s_gameMetadata.player1Playstyle, s_gameMetadata.player2Playstyle);
        gs.setCurrentPhase(s_gameMetadata.currentPhase, s_gameMetadata.currentRound, s_gameMetadata.isPlayer1Turn);
        {
            LastAction la;
            la.playerName = s_gameMetadata.lastAction.playerName;
            la.actionType = s_gameMetadata.lastAction.actionType;
            la.cardName = s_gameMetadata.lastAction.cardName;
            la.effectsApplied = s_gameMetadata.lastAction.effectsApplied;
            la.round = s_gameMetadata.lastAction.round;
            la.phase = s_gameMetadata.lastAction.phase;
            gs.setLastAction(la);
        }
        
        board.setPawnPos(extractInt(json, "pawnPosition", 9));
        std::string trackStr = extractString(json, "pawnTrack", std::bitset<19>().to_string());
        std::bitset<19> trackBits(trackStr);
        board.setPawnTrack(trackBits);
        
        std::vector<std::unique_ptr<Models::Token>> progress;
        std::vector<std::unique_ptr<Models::Token>> military;
        auto parseTokenArray = [&](const std::string& key, std::vector<std::unique_ptr<Models::Token>>& dst) {
            size_t keyPos = json.find("\"" + key + "\"");
            if (keyPos == std::string::npos) return;
            size_t arrStart = json.find('[', keyPos);
            if (arrStart == std::string::npos) return;
            size_t arrEnd = findMatchingBracket(json, arrStart);
            size_t cur = json.find("\"name\"", arrStart);
            while (cur != std::string::npos && cur < arrEnd) {
                std::string name = extractString(json.substr(cur, arrEnd - cur), "name");
                if (!name.empty()) {
                    if (auto t = cloneToken(catalog, name)) dst.push_back(std::move(t));
                }
                cur = json.find("\"name\"", cur + 6);
            }
        };
        parseTokenArray("progressTokens", progress);
        parseTokenArray("militaryTokens", military);
        board.setProgressTokens(std::move(progress));
        board.setMilitaryTokens(std::move(military));
        
        auto rebuildTree = [&](const std::string& key, std::vector<std::shared_ptr<Node>>& dst) {
            dst.clear();
            size_t keyPos = json.find("\"" + key + "\"");
            if (keyPos == std::string::npos) return;
            size_t arrStart = json.find('[', keyPos);
            if (arrStart == std::string::npos) return;
            size_t arrEnd = findMatchingBracket(json, arrStart);
            size_t objPos = json.find('{', arrStart);
            while (objPos != std::string::npos && objPos < arrEnd) {
                size_t objEnd = findMatchingBrace(json, objPos);
                auto objStr = json.substr(objPos, objEnd - objPos + 1);
                int idx = extractInt(objStr, "index", static_cast<int>(dst.size()));
                if (idx >= static_cast<int>(dst.size())) dst.resize(idx + 1);
                std::string cardName = extractString(objStr, "cardName", "EMPTY");
                std::unique_ptr<Models::Card> card;
                if (cardName != "EMPTY") {
                    card = cloneCard(catalog, cardName);
                    if (card) {
                        card->setIsAvailable(extractBool(objStr, "isAvailable", false));
                        card->setIsVisible(extractBool(objStr, "isVisible", false));
                    }
                }
                dst[idx] = std::make_shared<Node>(std::move(card));
                objPos = json.find('{', objEnd);
            }
            objPos = json.find('{', arrStart);
            while (objPos != std::string::npos && objPos < arrEnd) {
                size_t objEnd = findMatchingBrace(json, objPos);
                auto objStr = json.substr(objPos, objEnd - objPos + 1);
                int idx = extractInt(objStr, "index", 0);
                size_t pPos = objStr.find("\"parents\"");
                if (pPos != std::string::npos) {
                    size_t bStart = objStr.find('[', pPos);
                    size_t bEnd = objStr.find(']', bStart);
                    std::string arr = objStr.substr(bStart + 1, bEnd - bStart - 1);
                    std::stringstream ss(arr);
                    int parentIdx;
                    char comma;
                    while (ss >> parentIdx) {
                        if (parentIdx >= 0 && parentIdx < static_cast<int>(dst.size())) {
                            if (!dst[parentIdx]->getChild1()) dst[parentIdx]->setChild1(dst[idx]);
                            else dst[parentIdx]->setChild2(dst[idx]);
                        }
                        ss >> comma;
                    }
                }
                objPos = json.find('{', objEnd);
            }
        };
        
        rebuildTree("age1Tree", const_cast<std::vector<std::shared_ptr<Node>>&>(board.getAge1Nodes()));
        rebuildTree("age2Tree", const_cast<std::vector<std::shared_ptr<Node>>&>(board.getAge2Nodes()));
        rebuildTree("age3Tree", const_cast<std::vector<std::shared_ptr<Node>>&>(board.getAge3Nodes()));
        
        auto& age1Pool = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedAgeOneCards());
        auto& age2Pool = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedAgeTwoCards());
        auto& age3Pool = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedAgeThreeCards());
        auto& guildPool = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedGuildCards());
        auto& wonderPool = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedWonders());
        age1Pool.clear(); age2Pool.clear(); age3Pool.clear(); guildPool.clear(); wonderPool.clear();
        size_t unusedPos = json.find("\"unusedCards\"");
        if (unusedPos != std::string::npos) {
            size_t arrStart = json.find('[', unusedPos);
            if (arrStart != std::string::npos) {
                size_t arrEnd = findMatchingBracket(json, arrStart);
                size_t objPos = json.find('{', arrStart);
                while (objPos != std::string::npos && objPos < arrEnd) {
                    size_t objEnd = findMatchingBrace(json, objPos);
                    auto objStr = json.substr(objPos, objEnd - objPos + 1);
                    std::string name = extractString(objStr, "name");
                    std::string age = extractString(objStr, "age");
                    if (!name.empty()) {
                        auto card = cloneCard(catalog, name);
                        if (card) {
                            if (age == "AGE_I") age1Pool.push_back(std::move(card));
                            else if (age == "AGE_II") age2Pool.push_back(std::move(card));
                            else if (age == "AGE_III") age3Pool.push_back(std::move(card));
                            else if (age == "GUILD") guildPool.push_back(std::move(card));
                            else if (age == "WONDER") wonderPool.push_back(std::move(card));
                        }
                    }
                    objPos = json.find('{', objEnd);
                }
            }
        }
        
        auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
        discarded.clear();
        size_t discPos = json.find("\"discardedCards\"");
        if (discPos != std::string::npos) {
            size_t arrStart = json.find('[', discPos);
            if (arrStart != std::string::npos) {
                size_t arrEnd = findMatchingBracket(json, arrStart);
                size_t cur = json.find('"', arrStart + 1);
                while (cur != std::string::npos && cur < arrEnd) {
                    size_t nxt = json.find('"', cur + 1);
                    std::string name = json.substr(cur + 1, nxt - cur - 1);
                    if (auto c = cloneCard(catalog, name)) discarded.push_back(std::move(c));
                    cur = json.find('"', nxt + 1);
                }
            }
        }
        
        auto parsePlayerArray = [&](const std::string& parentStr, const std::string& key) {
            std::vector<std::string> out;
            size_t kPos = parentStr.find("\"" + key + "\"");
            if (kPos == std::string::npos) return out;
            size_t arrStart = parentStr.find('[', kPos);
            if (arrStart == std::string::npos) return out;
            size_t arrEnd = findMatchingBracket(parentStr, arrStart);
            size_t cur = parentStr.find('"', arrStart + 1);
            while (cur != std::string::npos && cur < arrEnd) {
                size_t nxt = parentStr.find('"', cur + 1);
                out.push_back(parentStr.substr(cur + 1, nxt - cur - 1));
                cur = parentStr.find('"', nxt + 1);
            }
            return out;
        };
        
        auto deserializePlayer = [&](const std::string& key, const std::shared_ptr<Player>& wrapper) {
            size_t pos = json.find("\"" + key + "\"");
            if (pos == std::string::npos || !wrapper) return;
            size_t objStart = json.find('{', pos);
            size_t objEnd = findMatchingBrace(json, objStart);
            std::string objStr = json.substr(objStart, objEnd - objStart + 1);
            int id = extractInt(objStr, "id", 0);
            std::string username = extractString(objStr, "username", "Player");
            wrapper->m_player = std::make_unique<Models::Player>(id, username);
            auto* p = wrapper->m_player.get();
            uint8_t g = static_cast<uint8_t>(extractInt(objStr, "gold", 0));
            uint8_t s = static_cast<uint8_t>(extractInt(objStr, "silver", 0));
            uint8_t b = static_cast<uint8_t>(extractInt(objStr, "bronze", 0));
            p->setRemainingCoins({ g, s, b });
            for (auto& name : parsePlayerArray(objStr, "builtCards")) {
                if (auto c = cloneCard(catalog, name)) p->addCard(std::move(c));
            }
            for (auto& name : parsePlayerArray(objStr, "wonders")) {
                if (auto c = cloneCard(catalog, name)) {
                    if (auto* w = dynamic_cast<Models::Wonder*>(c.get())) {
                        std::unique_ptr<Models::Wonder> wptr(static_cast<Models::Wonder*>(c.release()));
                        wptr->setConstructed(true);
                        p->addWonder(std::move(wptr));
                    }
                }
            }
            for (auto& name : parsePlayerArray(objStr, "ownedTokens")) {
                if (auto t = cloneToken(catalog, name)) p->addToken(std::move(t));
            }
        };
        
        deserializePlayer("player1", gs.GetPlayer1());
        deserializePlayer("player2", gs.GetPlayer2());
        
        std::cout << "[JSON] Loaded save #" << saveNumber << " from " << filename << "\n";
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
            
            auto card = node ? node->getCard() : nullptr;
            writer.writeString("cardName", card ? card->getName() : "EMPTY", true);
            writer.writeBool("isEmpty", !card, true);
            writer.writeBool("isAvailable", card && card->isAvailable(), true);
            writer.writeBool("isVisible", card && card->isVisible(), true);
            
            writer.writeKey("parents");
            writer.buffer << "[";
            if (node) {
                auto p1 = node->getParent1();
                auto p2 = node->getParent2();
                std::vector<int> parentIndices;
                
                for (size_t j = 0; j < nodes->size(); ++j) {
                    if ((*nodes)[j]) {
                        if (p1 && (*nodes)[j].get() == p1.get()) parentIndices.push_back(static_cast<int>(j));
                        if (p2 && (*nodes)[j].get() == p2.get()) parentIndices.push_back(static_cast<int>(j));
                    }
                }
                
                for (size_t k = 0; k < parentIndices.size(); k++) {
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
                
                writer.writeInt("id", p->getkPlayerId(), true);
                writer.writeString("username", p->getPlayerUsername(), true);
                
                const auto& coins = p->getRemainingCoins();
                writer.writeKey("remainingCoins");
                writer.buffer << "{";
                writer.buffer << "\"gold\": " << static_cast<int>(std::get<0>(coins)) << ", ";
                writer.buffer << "\"silver\": " << static_cast<int>(std::get<1>(coins)) << ", ";
                writer.buffer << "\"bronze\": " << static_cast<int>(std::get<2>(coins));
                writer.buffer << "},\n";
                writer.writeInt("totalCoins", p->totalCoins(coins), true);
                
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
                        writer.buffer << "\"" << Models::tradeRuleTypeToString(ruleType) << "\"";
                        firstRule = false;
                    }
                }
                if (!firstRule) writer.buffer << "\n";
                writer.endArray(true);
                
                const auto& tokensOwned = p->getTokensOwned();
                writer.writeString("tokensOwnedBitset", tokensOwned.to_string(), true);
                
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
                writer.writeInt("military", points.m_militaryVictoryPoints, true);
                writer.writeInt("building", points.m_buildingVictoryPoints, true);
                writer.writeInt("wonder", points.m_wonderVictoryPoints, true);
                writer.writeInt("progress", points.m_progressVictoryPoints, true);
                writer.writeInt("total", points.totalVictoryPoints(), false);
                writer.endObject(false);
            }
            
            writer.endObject(isLast);
        };
        
        auto p1 = gameState.GetPlayer1();
        auto p2 = gameState.GetPlayer2();
        
        serializePlayer(p1, "player1", true);
        serializePlayer(p2, "player2", true);
    }
    
    void GameStateSerializer::serializeUnusedCards(JsonWriter& writer) {
        const auto& board = Board::getInstance();
        
        writer.startArray("unusedCards");
        
        bool hasContent = false;
        
        const auto& age1Cards = board.getUnusedAgeOneCards();
        for (const auto& card : age1Cards) {
            if (card) {
                if (hasContent) writer.buffer << ",\n";
                writer.writeIndent();
                writer.buffer << "{\"age\": \"AGE_I\", \"name\": \"" << writer.escapeString(card->getName()) << "\"}";
                hasContent = true;
            }
        }
        
        const auto& age2Cards = board.getUnusedAgeTwoCards();
        for (const auto& card : age2Cards) {
            if (card) {
                if (hasContent) writer.buffer << ",\n";
                writer.writeIndent();
                writer.buffer << "{\"age\": \"AGE_II\", \"name\": \"" << writer.escapeString(card->getName()) << "\"}";
                hasContent = true;
            }
        }
        
        const auto& age3Cards = board.getUnusedAgeThreeCards();
        for (const auto& card : age3Cards) {
            if (card) {
                if (hasContent) writer.buffer << ",\n";
                writer.writeIndent();
                writer.buffer << "{\"age\": \"AGE_III\", \"name\": \"" << writer.escapeString(card->getName()) << "\"}";
                hasContent = true;
            }
        }
        
        const auto& guildCards = board.getUnusedGuildCards();
        for (const auto& card : guildCards) {
            if (card) {
                if (hasContent) writer.buffer << ",\n";
                writer.writeIndent();
                writer.buffer << "{\"age\": \"GUILD\", \"name\": \"" << writer.escapeString(card->getName()) << "\"}";
                hasContent = true;
            }
        }
        
        const auto& wonderCards = board.getUnusedWonders();
        for (size_t i = 0; i < wonderCards.size(); ++i) {
            if (wonderCards[i]) {
                if (hasContent) writer.buffer << ",\n";
                writer.writeIndent();
                writer.buffer << "{\"age\": \"WONDER\", \"name\": \"" << writer.escapeString(wonderCards[i]->getName()) << "\"}";
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
        "winnerId": )" + std::to_string(state.getWinnerId()) + R"(
        "victoryType": ")" + state.getVictoryType() + R"(",
        "winnerScore": )" + std::to_string(state.getWinnerScore()) + R"(
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
