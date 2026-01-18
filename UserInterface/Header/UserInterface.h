#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_UserInterface.h"
#include <vector>
#include <memory>
#include <functional>
#include <QtCore/QPointer>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSplitter>
class WonderSelectionWidget;
class PlayerPanelWidget;
class BoardWidget;
class AgeTreeWidget;
class WonderSelectionController;
class GameListenerBridge;

namespace Models {
	class Wonder;
	class Card;
}

namespace Core {
	class Player;
}

class UserInterface : public QMainWindow
{
	Q_OBJECT

public:
	UserInterface(QWidget* parent = nullptr);
	~UserInterface();

	enum class GameMode {
		PvP,
		PvAI,
		AIvAI
	};

private:
	void setupLayout();
	void setupCenterPanel(QSplitter* splitter);
	void initializeGame();
	void initializePlayers();
	void startWonderSelection();
	void showAgeTree(int age);
	void updateTurnLabel();
	void onWonderSelected(int index);
	void showGameModeSelection();
	void showPhaseTransitionMessage(int age);
	void finishAction(int age, bool parentBecameAvailable);

private:
	Ui::UserInterfaceClass ui;
	WonderSelectionWidget* m_centerWidget = nullptr;
	PlayerPanelWidget* m_leftPanel = nullptr;
	PlayerPanelWidget* m_rightPanel = nullptr;
	BoardWidget* m_boardWidget = nullptr;
	QPointer<AgeTreeWidget> m_ageTreeWidget = nullptr;
	WonderSelectionController* m_wonderController = nullptr;
	std::shared_ptr<GameListenerBridge> m_gameListener;

	QWidget* m_centerContainer = nullptr;
	QWidget* m_centerTop = nullptr;
	QWidget* m_centerMiddle = nullptr;
	QWidget* m_centerBottom = nullptr;
	QLabel* m_phaseBanner = nullptr;

	int m_selectionPhase = 0;
	int m_cardsPickedInPhase = 0;
	std::vector<Models::Wonder*> m_currentBatch;
	int m_currentPlayerIndex = 0;
	GameMode m_gameMode = GameMode::PvP;
};