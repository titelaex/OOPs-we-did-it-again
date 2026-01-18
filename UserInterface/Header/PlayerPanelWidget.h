#pragma once

#include <QWidget>
#include <memory>
#include <unordered_map>
#include <QString>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
import Core.Player; 
import  Models.ColorType;

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
	void refreshTokens();
	void refreshUsername();

private:
	std::shared_ptr<Core::Player> m_player;
	QVBoxLayout* m_layout;
	bool m_isLeftPanel{ false };

	QGridLayout* m_wondersGrid = nullptr;
	QLabel* m_coinsLabel = nullptr;
	QLabel* m_vpLabel = nullptr;
	QLabel* m_usernameLabel = nullptr;

	QListWidget* m_cardsBrown = nullptr;
	QListWidget* m_cardsGrey = nullptr;
	QListWidget* m_cardsRed = nullptr;
	QListWidget* m_cardsYellow = nullptr;
	QListWidget* m_cardsGreen = nullptr;
	QListWidget* m_cardsBlue = nullptr;
	QListWidget* m_cardsPurple = nullptr;
	
	QWidget* m_tokensSection = nullptr; 

	void buildUi();
	void addStatsRow();
	void addCardSections();
	void addWonderSection();
	void addProgressTokensSection();
	QListWidget* listForColor(Models::ColorType col) const;

	QString QStringBuilder(const std::string& s) const;
	QString ColorToCss(Models::ColorType c) const;

	std::unordered_map<Models::ColorType, QWidget*> m_colorSections;

public:
	void refresh();
	void showPlayedCard(const QString& name, Models::ColorType color);
};

inline void PlayerPanelWidget::showPlayedCard(const QString& name, Models::ColorType color)
{
	auto it = m_colorSections.find(color);
	if (it == m_colorSections.end()) return;
	QWidget* section = it->second;
	if (!section) return;

	QLabel* label = new QLabel(name, section);
	label->setStyleSheet("color: #111; background-color: rgba(255,255,255,0.9); border-radius:4px; padding:4px; font-weight:bold;");
	label->setAlignment(Qt::AlignCenter);
	if (section->layout()) {
		section->layout()->addWidget(label);
	} else {
		label->setParent(section);
	}

	QTimer::singleShot(2500, label, [label]() { label->deleteLater(); });
}
