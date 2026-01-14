export module Core.GameMetadata;
import Core.AIConfig;
import <string>;

export namespace Core {
    class GameMetadata {
    public:
        GameMetadata();
        
        int getMode() const { return m_mode; }
        void setMode(int mode) { m_mode = mode; }
        
        Playstyle getPlayer1Playstyle() const { return m_p1Playstyle; }
        void setPlayer1Playstyle(Playstyle playstyle) { m_p1Playstyle = playstyle; }
        
        Playstyle getPlayer2Playstyle() const { return m_p2Playstyle; }
        void setPlayer2Playstyle(Playstyle playstyle) { m_p2Playstyle = playstyle; }
        
        static GameMetadata loadFromFile(const std::string& filename);
        void saveToFile(const std::string& filename) const;
        
    private:
        int m_mode = 1;
        Playstyle m_p1Playstyle = Playstyle::BRITNEY;
        Playstyle m_p2Playstyle = Playstyle::BRITNEY;
    };
}
