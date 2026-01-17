// Handle turn changes
	m_ageTreeWidget->onPlayerTurnChanged = [this](int newPlayerIndex, const QString& playerName) {
		m_currentPlayerIndex = newPlayerIndex;
		if (m_phaseBanner) {
			m_phaseBanner->setText("Turn: " + playerName);
			m_phaseBanner->show();
		}
		if (m_boardWidget) m_boardWidget->refresh();

		// Disabled auto-advance phase detection to prevent full tree rebuilds
		// AgeTreeWidget now handles selective updates; phase changes should be triggered explicitly
		#if0
		if (m_ageTreeWidget && m_ageTreeWidget->getCurrentAge() ==1) {
			QTimer::singleShot(0, this, [this]() {
				auto& board = Core::Board::getInstance();
				const auto& nodes = board.getAge1Nodes();
				bool anyAvailable = false;
				for (const auto& n : nodes) {
					if (!n) continue;
					auto* c = n->getCard();
					if (!c) continue;
					if (n->isAvailable() && c->isAvailable()) { anyAvailable = true; break; }
				}
				if (!anyAvailable) {
					this->showAgeTree(2);
				}
			});
		}
		#endif
	};
