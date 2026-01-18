export module Core.AIConfig;
import <string>;
import <fstream>;
import <sstream>;
export namespace Core {
    enum class Playstyle : uint8_t {
        BRITNEY,  
        SPEARS    
    };
    inline std::string playstyleToString(Playstyle style) {
        switch (style) {
            case Playstyle::BRITNEY: return "Britney";
            case Playstyle::SPEARS:  return "Spears";
            default: return "Unknown";
        }
    }
    class AIWeights {
    public:
        double victoryPointValue = 1.0;      
        double militaryPriority = 1.0;       
        double sciencePriority = 1.0;        
        double economyPriority = 1.0;        
        double resourceValue = 1.0;          
        double coinValue = 1.0;              
        double wonderVPBonus = 1.0;          
        double wonderMilitaryBonus = 1.0;    
        double wonderEconomyBonus = 1.0;     
        double opponentDenial = 1.0;         
        double futureValueDiscount = 0.95;   
        void normalizeWeights() {
            auto clamp = [](double& val) {
                if (val < 0.1) val = 0.1;
                if (val > 3.0) val = 3.0;
            };
            clamp(victoryPointValue);
            clamp(militaryPriority);
            clamp(sciencePriority);
            clamp(economyPriority);
            clamp(resourceValue);
            clamp(coinValue);
            clamp(wonderVPBonus);
            clamp(wonderMilitaryBonus);
            clamp(wonderEconomyBonus);
            clamp(opponentDenial);
            if (futureValueDiscount < 0.8) futureValueDiscount = 0.8;
            if (futureValueDiscount > 1.0) futureValueDiscount = 1.0;
        }
        bool loadFromFile(const std::string& filename) {
            std::ifstream file(filename);
            if (!file.is_open()) {
                return false;
            }
            std::string line;
            while (std::getline(file, line)) {
                if (line.empty() || line[0] == '#') continue;
                size_t pos = line.find('=');
                if (pos == std::string::npos) continue;
                std::string key = line.substr(0, pos);
                double value = std::stod(line.substr(pos + 1));
                if (key == "victoryPointValue") victoryPointValue = value;
                else if (key == "militaryPriority") militaryPriority = value;
                else if (key == "sciencePriority") sciencePriority = value;
                else if (key == "economyPriority") economyPriority = value;
                else if (key == "resourceValue") resourceValue = value;
                else if (key == "coinValue") coinValue = value;
                else if (key == "wonderVPBonus") wonderVPBonus = value;
                else if (key == "wonderMilitaryBonus") wonderMilitaryBonus = value;
                else if (key == "wonderEconomyBonus") wonderEconomyBonus = value;
                else if (key == "opponentDenial") opponentDenial = value;
                else if (key == "futureValueDiscount") futureValueDiscount = value;
            }
            file.close();
            normalizeWeights();
            return true;
        }
    };
    inline AIWeights getDefaultWeights(Playstyle style) {
        AIWeights weights;
        switch (style) {
            case Playstyle::BRITNEY:
                weights.victoryPointValue = 1.2;
                weights.militaryPriority = 0.8;      
                weights.sciencePriority = 1.2;       
                weights.economyPriority = 1.3;       
                weights.resourceValue = 1.1;
                weights.coinValue = 1.2;
                weights.wonderVPBonus = 1.3;
                weights.wonderMilitaryBonus = 1.0;
                weights.wonderEconomyBonus = 1.2;
                weights.opponentDenial = 0.9;        
                weights.futureValueDiscount = 0.95;  
                break;
            case Playstyle::SPEARS:
                weights.victoryPointValue = 0.9;
                weights.militaryPriority = 1.5;      
                weights.sciencePriority = 0.9;
                weights.economyPriority = 1.0;
                weights.resourceValue = 1.0;
                weights.coinValue = 1.0;
                weights.wonderVPBonus = 1.1;
                weights.wonderMilitaryBonus = 1.4;   
                weights.wonderEconomyBonus = 1.0;
                weights.opponentDenial = 1.3;        
                weights.futureValueDiscount = 0.92;  
                break;
        }
        return weights;
    }
    class AIConfig {
    private:
        Playstyle m_playstyle;
        double m_explorationConstant;
        int m_mctsIterations;
        int m_maxSimulationDepth;
    public:
        AIConfig(Playstyle style = Playstyle::BRITNEY, 
                 double explorationConstant = 1.414,
                 int mctsIterations = 1000,
                 int maxSimulationDepth = 20)
            : m_playstyle(style)
            , m_explorationConstant(explorationConstant)
            , m_mctsIterations(mctsIterations)
            , m_maxSimulationDepth(maxSimulationDepth)
        {}
        Playstyle getPlaystyle() const { return m_playstyle; }
        double getExplorationConstant() const { return m_explorationConstant; }
        int getMCTSIterations() const { return m_mctsIterations; }
        int getMaxSimulationDepth() const { return m_maxSimulationDepth; }
        void setPlaystyle(Playstyle style) { m_playstyle = style; }
        void setExplorationConstant(double val) { m_explorationConstant = val; }
        void setMCTSIterations(int val) { m_mctsIterations = val; }
        void setMaxSimulationDepth(int val) { m_maxSimulationDepth = val; }
        AIWeights getWeights() const {
            AIWeights weights;
            std::string styleStr = playstyleToString(m_playstyle);
            std::string optimizedPath = "OptimizedWeights/" + styleStr + "_optimized.weights";
            if (weights.loadFromFile(optimizedPath)) {
                return weights;
            }
            return Core::getDefaultWeights(m_playstyle);
        }
    };
} 
