	void setCurrentPlayerIndex(int index);
	int getCurrentAge() const { return m_currentAge; }

	std::function<void(int newPlayerIndex, const QString& playerName)> onPlayerTurnChanged;

	std::unordered_map<QWidget*, QGraphicsProxyWidget*> m_proxyMap;
	
	int m_currentAge = 1;
	
	std::function<void(std::function<void(int)>)> onRequestTokenSelection;
