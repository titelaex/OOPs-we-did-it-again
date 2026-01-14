#pragma once

#include <QWidget>
#include <memory>
import Core.Player;
import Models.ColorType;

class QGridLayout;
class QLabel;
class QVBoxLayout;
class QListWidget;

class PlayerPanelWidget : public QWidget
{
	Q_OBJECT

public:
	explicit PlayerPanelWidget(std::shared_ptr<Core::Player> player, QWidget* parent = nullptr, bool isLeftPanel = false);

	void refreshStats();
	void refreshWonders();
	void refreshCards();

private:
	std::shared_ptr<Core::Player> m_player;
	QVBoxLayout* m_layout;
	bool m_isLeftPanel{ false };

	QGridLayout* m_wondersGrid = nullptr;
	QLabel* m_coinsLabel = nullptr;
	QLabel* m_vpLabel = nullptr;

	QListWidget* m_cardsBrown = nullptr;
	QListWidget* m_cardsGrey = nullptr;
	QListWidget* m_cardsRed = nullptr;
	QListWidget* m_cardsYellow = nullptr;
	QListWidget* m_cardsGreen = nullptr;
	QListWidget* m_cardsBlue = nullptr;
	QListWidget* m_cardsPurple = nullptr;

	void buildUi();
	void addStatsRow();
	void addCardSections();
	void addWonderSection();
	void addProgressTokensSection();
	QListWidget* listForColor(Models::ColorType col) const;

	QString QStringBuilder(const std::string& s) const;
	QString ColorToCss(Models::ColorType c) const;
};