module Core.GameMetadata;
import <fstream>;
import <sstream>;
import <iostream>;

namespace Core {
    GameMetadata::GameMetadata() 
        : m_mode(1), m_p1Playstyle(Playstyle::BRITNEY), m_p2Playstyle(Playstyle::BRITNEY) {
    }
    
    GameMetadata GameMetadata::loadFromFile(const std::string& filename) {
        GameMetadata meta;
        std::ifstream file(filename);
        
        if (!file.is_open()) {
            return meta; // Return defaults if file doesn't exist
        }
        
        std::string line;
        while (std::getline(file, line)) {
            size_t commaPos = line.find(',');
            if (commaPos == std::string::npos) continue;
            
            std::string key = line.substr(0, commaPos);
            std::string value = line.substr(commaPos + 1);
            
            if (key == "GameMode") {
                meta.m_mode = std::stoi(value);
            } else if (key == "Player1Playstyle") {
                meta.m_p1Playstyle = static_cast<Playstyle>(std::stoi(value));
            } else if (key == "Player2Playstyle") {
                meta.m_p2Playstyle = static_cast<Playstyle>(std::stoi(value));
            }
        }
        
        file.close();
        return meta;
    }
    
    void GameMetadata::saveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open " << filename << " for writing.\n";
            return;
        }
        
        file << "GameMode," << m_mode << "\n";
        file << "Player1Playstyle," << static_cast<int>(m_p1Playstyle) << "\n";
        file << "Player2Playstyle," << static_cast<int>(m_p2Playstyle) << "\n";
        
        file.close();
    }
}
