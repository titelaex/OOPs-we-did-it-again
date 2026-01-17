export module Core.GameStateSerializer;
import <string>;
import <vector>;
import <fstream>;
import <sstream>;
import <filesystem>;
import Core.AIConfig;
import Core.GameState;

export namespace Core {
    
    struct GameStateMetadata {
        int gameMode = 1;
        bool trainingMode = false;
        Playstyle player1Playstyle = Playstyle::BRITNEY;
        Playstyle player2Playstyle = Playstyle::BRITNEY;
        
        int currentPhase = 1;
        int currentRound = 1;
        bool isPlayer1Turn = true;
        
        struct LastAction {
            std::string playerName;
            std::string actionType;
            std::string cardName;
            std::vector<std::string> effectsApplied;
            int round = 0;
            int phase = 0;
        } lastAction;
    };
    
    class GameStateSerializer {
    public:
        static bool saveGame(bool isNewGame = false);
        static bool loadGame(int saveNumber);
        
        static std::vector<int> getAllSaveNumbers();
        static int getHighestSaveNumber();
        static bool deleteSave(int saveNumber);
        
        static int getCurrentSaveNumber();
        static void setCurrentSaveNumber(int number);
        
        static const GameStateMetadata& getMetadata();
        static void setMetadata(const GameStateMetadata& metadata);
        
        static void setGameMode(int mode, bool training = false);
        static void setPlayerPlaystyles(Playstyle p1, Playstyle p2);
        static void setCurrentPhase(int phase, int round, bool isP1Turn);
        static void recordLastAction(const std::string& playerName, const std::string& actionType,
                                     const std::string& cardName, const std::vector<std::string>& effects,
                                     int round, int phase);
        
        static void saveVictoryToJson(const GameState& state, std::string& json);
        static void loadVictoryFromJson(GameState& state, const std::string& json);
        
    private:
        static int s_currentSaveNumber;
        static GameStateMetadata s_gameMetadata;
        static std::string getSaveFilename(int number);
        static int extractNumberFromFilename(const std::string& filename);
        
        class JsonWriter {
        public:
            std::ostringstream buffer;
            int indent = 0;
            
            void writeIndent();
            std::string escapeString(const std::string& str);
            void startObject();
            void endObject(bool addComma = false);
            void startArray(const std::string& key);
            void endArray(bool addComma = false);
            void writeKey(const std::string& key);
            void writeString(const std::string& key, const std::string& value, bool addComma = true);
            void writeInt(const std::string& key, int value, bool addComma = true);
            void writeBool(const std::string& key, bool value, bool addComma = true);
            void writeStringValue(const std::string& value, bool addComma = true);
            void writeIntValue(int value, bool addComma = true);
            
            std::string toString() const { return buffer.str(); }
        };
        
        static void serializeGameMetadata(JsonWriter& writer);
        static void serializeBoardState(JsonWriter& writer);
        static void serializeTokens(JsonWriter& writer);
        static void serializeTreeNodes(JsonWriter& writer, const std::string& treeKey, int ageIndex);
        static void serializePlayers(JsonWriter& writer);
        static void serializeUnusedCards(JsonWriter& writer);
        static void serializeDiscardedCards(JsonWriter& writer);
    };
}
