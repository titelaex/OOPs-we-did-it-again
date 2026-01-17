#pragma once

#include <QtWidgets/QMainWindow>
#include <vector>
#include <unordered_map>
#include <memory>
#include "ui_UserInterface.h"
#include <QtWidgets/QDialog>

class QSplitter;
class QLabel;
class QGraphicsProxyWidget;
class QWidget;
class PlayerPanelWidget;
class WonderSelectionWidget;
class WonderSelectionController;
class BoardWidget;
class AgeTreeWidget;
class GameListenerBridge;


namespace Models { class Wonder; }

class UserInterface : public QMainWindow
{
	Q_OBJECT

public:

	enum class GameMode {
		PvP,    // Player vs Player
		PvAI,   // Player vs AI
		AIvAI   // AI vs AI
	};

	explicit  UserInterface(QWidget* parent = nullptr);
	~UserInterface();

private:
	void showGameModeSelection();

	void initializePlayers();
	void setupLayout();
	void setupCenterPanel(QSplitter* splitter);
	void startWonderSelection();
	void showPhaseTransitionMessage();
	void loadNextBatch();
	void updatePanels();
	void handleLeafClicked(int nodeIndex, int age);
	void initializeGame();

	Ui::UserInterfaceClass ui;

	GameMode m_gameMode = GameMode::PvP;

	PlayerPanelWidget* m_leftPanel{ nullptr };
	PlayerPanelWidget* m_rightPanel{ nullptr };
	WonderSelectionWidget* m_centerWidget{ nullptr };
	WonderSelectionController* m_wonderController{ nullptr };

	QWidget* m_centerContainer{ nullptr };
	QWidget* m_centerTop{ nullptr };
	QWidget* m_centerMiddle{ nullptr };
	QWidget* m_centerBottom{ nullptr };

	QLabel* m_phaseBanner{ nullptr };
	BoardWidget* m_boardWidget{ nullptr };
	AgeTreeWidget* m_ageTreeWidget{ nullptr };

	int m_selectionPhase = 0;
	int m_cardsPickedInPhase = 0;

	int m_currentPlayerIndex = 0;

	std::vector<Models::Wonder*> m_currentBatch;

	std::unordered_map<QWidget*, QGraphicsProxyWidget*> m_proxyMap;

	void updateTurnLabel();

	Q_INVOKABLE void showAgeTree(int age);

	std::shared_ptr<GameListenerBridge> m_gameListener;

private Q_SLOTS:
	void finishAction(int age, bool parentBecameAvailable);
	void onWonderSelected(int index);
};