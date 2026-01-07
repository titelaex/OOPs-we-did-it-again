#pragma once

#include <QWidget>
#include <memory>
import Core.Player; 
import Models.ColorType;

class QGridLayout;
class QLabel;
class QVBoxLayout;

class PlayerPanelWidget : public QWidget
{
	Q_OBJECT

public:
	explicit PlayerPanelWidget(std::shared_ptr<Core::Player> player, QWidget* parent = nullptr, bool isLeftPanel = false);

private:
	std::shared_ptr<Core::Player> m_player;
	QVBoxLayout* m_layout;
	bool m_isLeftPanel{ false };

	QGridLayout* m_wondersGrid = nullptr;

	void buildUi();
	void addStatsRow();
	void addCardSections();
	void addWonderSection();
	void addProgressTokensSection();

	QString QStringBuilder(const std::string& s) const;
	QString ColorToCss(Models::ColorType c) const;

public:
	void refreshWonders(); 
};