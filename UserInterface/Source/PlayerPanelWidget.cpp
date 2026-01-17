#include "Header/PlayerPanelWidget.h"
#include "Header/CardSpineDelegate.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QScrollArea>
#include <QPalette>
#include <QMap>
#include <QResizeEvent>

class SpineList : public QListWidget {
public:
	explicit SpineList(QWidget* parent = nullptr, int spineWidth = 24)
		: QListWidget(parent), m_spineWidth(spineWidth)
	{
		setViewMode(QListView::IconMode);
		setResizeMode(QListView::Adjust);
		setUniformItemSizes(false);
	}
protected:
	void resizeEvent(QResizeEvent* event) override {
		QListWidget::resizeEvent(event);
		updateItemSizes();
	}
private:
	void updateItemSizes() {
		int h = viewport()->height();
		if (h <= 0) return;
		int vpad = 8;
		int itemH = std::max(24, h - vpad);
		QSize sz(m_spineWidth, itemH);
		for (int i = 0; i < count(); ++i) {
			if (auto it = item(i)) it->setSizeHint(sz);
		}
	}
	int m_spineWidth;
};


QString PlayerPanelWidget::QStringBuilder(const std::string& s) const {
	return QString::fromStdString(s);
}

QString PlayerPanelWidget::ColorToCss(Models::ColorType c) const {
	switch (c) {
	case Models::ColorType::BROWN: return "#A0522D";
	case Models::ColorType::GREY: return "#808080";
	case Models::ColorType::RED: return "#DC2626";
	case Models::ColorType::YELLOW: return "#F59E0B";
	case Models::ColorType::GREEN: return "#10B981";
	case Models::ColorType::BLUE: return "#3B82F6";
	case Models::ColorType::PURPLE: return "#8B5CF6";
	default: return "#6B7280"; // gray
	}
}

QListWidget* PlayerPanelWidget::listForColor(Models::ColorType col) const
{
	switch (col) {
	case Models::ColorType::BROWN: return m_cardsBrown;
	case Models::ColorType::GREY: return m_cardsGrey;
	case Models::ColorType::RED: return m_cardsRed;
	case Models::ColorType::YELLOW: return m_cardsYellow;
	case Models::ColorType::GREEN: return m_cardsGreen;
	case Models::ColorType::BLUE: return m_cardsBlue;
	case Models::ColorType::PURPLE: return m_cardsPurple;
	default: return nullptr;
	}
}

void PlayerPanelWidget::refreshWonders()
{
	if (!m_wondersGrid) return;

	QLayoutItem* item;
	while ((item = m_wondersGrid->takeAt(0)) != nullptr) {
		if (item->widget()) {
			delete item->widget();
		}
		delete item;
	}

	auto title = new QLabel("Wonders:", this);
	title->setStyleSheet("font-weight: bold; margin-right:6px;");
	title->setAlignment(m_isLeftPanel ? Qt::AlignRight : Qt::AlignLeft);
	m_wondersGrid->addWidget(title, 0, 0, 1, 2);

	bool hasPlayer = (m_player && m_player->m_player);

	for (int i = 0; i < 4; ++i) {
		auto slot = new QLabel(this);
		slot->setAlignment(Qt::AlignCenter);
		slot->setFixedSize(150, 70);

		int row = (i / 2) + 1;
		int col = i % 2;

		if (m_isLeftPanel) {
			col = 1 - col;
		}

		QString emptyStyle = "background-color: rgba(255,255,255,0.04); border:1px solid #374151; border-radius:6px; color:#9CA3AF; font-style:italic;";

		if (hasPlayer) {
			auto& wonders = m_player->m_player->getOwnedWonders();

			if (static_cast<size_t>(i) < wonders.size() && wonders[i]) {
				const auto* w = wonders[i].get();
				QString name = QStringBuilder(w->getName());
				bool built = w->IsConstructed();

				slot->setText(name + (built ? "\n(constructed)" : "\n(unbuilt)"));

				QString style;
				if (built) {
					// Gold highlight for constructed wonders
					style = QString(
						"background: qlineargradient(x1:%1, y1:0, x2:%2, y2:0, stop:0 #FFD700, stop:1 #B8860B);"
						"border:2px solid #8B6508; border-radius:6px; color:#111827; font-weight:700;")
						.arg(m_isLeftPanel ? 1 : 0)
						.arg(m_isLeftPanel ? 0 : 1);
				} else {
					QString bg = ColorToCss(w->getColor());
					style = QString(
						"background: qlineargradient(x1:%1, y1:0, x2:%2, y2:0, stop:0 %3, stop:1 rgba(0,0,0,0.15));"
						"border:1px solid #111; border-radius:6px; color:#fff; font-weight:600;")
						.arg(m_isLeftPanel ? 1 : 0)
						.arg(m_isLeftPanel ? 0 : 1)
						.arg(bg);
				}

				slot->setStyleSheet(style);
				slot->setToolTip(name);
			}
			else {
				slot->setText("<empty>");
				slot->setStyleSheet(emptyStyle);
			}
		}
		else {
			slot->setText("<empty>");
			slot->setStyleSheet(emptyStyle);
		}

		m_wondersGrid->addWidget(slot, row, col);
	}
}

PlayerPanelWidget::PlayerPanelWidget(std::shared_ptr<Core::Player> player, QWidget* parent, bool isLeftPanel)
	: QWidget(parent), m_player(player)
{
	m_isLeftPanel = isLeftPanel;
	m_layout = new QVBoxLayout(this);
	m_layout->setContentsMargins(8, 8, 8, 8);
	m_layout->setSpacing(10);
	buildUi();
}

void PlayerPanelWidget::buildUi()
{

	QString username = m_player && m_player->m_player ? QStringBuilder(m_player->m_player->getPlayerUsername()) : QString("<unknown>");

	auto usernameLbl = new QLabel(username, this);
	usernameLbl->setStyleSheet("font-size:18px; font-weight: bold; margin-bottom:6px;");
	usernameLbl->setAlignment(m_isLeftPanel ? Qt::AlignRight : Qt::AlignLeft);
	m_layout->addWidget(usernameLbl);

	addStatsRow();
	addProgressTokensSection();
	addCardSections();
	addWonderSection();

	m_layout->addStretch(1);
}

void PlayerPanelWidget::addWonderSection()
{
	QWidget* wondersSection = new QWidget(this);

	m_wondersGrid = new QGridLayout(wondersSection);
	m_wondersGrid->setContentsMargins(4, 4, 4, 4);
	m_wondersGrid->setSpacing(8);

	m_layout->addWidget(wondersSection);

	refreshWonders();
}


void PlayerPanelWidget::addProgressTokensSection()
{
	QWidget* tokensSection = new QWidget(this);
	auto* tokensLayout = new QHBoxLayout(tokensSection);
	tokensLayout->setContentsMargins(4, 4, 4, 4);
	tokensLayout->setSpacing(8);
	tokensLayout->setAlignment(m_isLeftPanel ? Qt::AlignRight : Qt::AlignLeft);

	QString baseStyle = "color:#F9FAFB; font-size:11px; padding:0;";
	QString circleStyleFilled = baseStyle + "border:2px dotted #9CA3AF; border-radius:18px; width:36px; height:36px; background-color:#4B5563;";
	QString circleStyleEmpty = baseStyle + "border:2px dotted #9CA3AF; border-radius:18px; width:36px; height:36px; background-color:transparent;";

	int rendered = 0;
	if (m_player && m_player->m_player) {
		const auto& ownedTokens = m_player->m_player->getOwnedTokens();
		for (const auto& t : ownedTokens) {
			if (!t) continue;
			auto tokenLbl = new QLabel(QString(), tokensSection);
			tokenLbl->setFixedSize(36, 36);
			tokenLbl->setAlignment(Qt::AlignCenter);
			tokenLbl->setStyleSheet(circleStyleFilled);
			tokenLbl->setToolTip(QStringBuilder(t->getName()));

			QString name = QStringBuilder(t->getName());
			if (!name.isEmpty()) {
				QString initials = name.section(' ', 0, 0).left(2).toUpper();
				tokenLbl->setText(initials);
			}
			tokensLayout->addWidget(tokenLbl);
			if (++rendered >= 3) break;
		}
	}
	for (; rendered < 3; ++rendered) {
		auto placeholder = new QLabel(QString(), tokensSection);
		placeholder->setFixedSize(36, 36);
		placeholder->setAlignment(Qt::AlignCenter);
		placeholder->setStyleSheet(circleStyleEmpty);
		tokensLayout->addWidget(placeholder);
	}
	m_layout->addWidget(tokensSection);
}

void PlayerPanelWidget::refreshStats()
{
	if (!m_player || !m_player->m_player) return;

	auto coinsTuple = m_player->m_player->getRemainingCoins();
	uint32_t coinsVal = static_cast<uint32_t>(m_player->m_player->totalCoins(coinsTuple));
	int vpVal = static_cast<int>(m_player->m_player->getTotalVictoryPoints());

	if (m_coinsLabel) m_coinsLabel->setText(QString::number(coinsVal));
	if (m_vpLabel) m_vpLabel->setText(QString::number(vpVal));
}

void PlayerPanelWidget::addStatsRow()
{
	uint32_t coinsVal = 0;
	int vpVal = 0;
	if (m_player && m_player->m_player) {
		auto coinsTuple = m_player->m_player->getRemainingCoins();
		coinsVal = static_cast<uint32_t>(m_player->m_player->totalCoins(coinsTuple));
		vpVal = static_cast<int>(m_player->m_player->getTotalVictoryPoints());
	}

	auto statsRow = new QHBoxLayout();
	statsRow->setSpacing(12);

	m_coinsLabel = new QLabel(QString::number(coinsVal), this);
	m_coinsLabel->setAlignment(Qt::AlignCenter);
	m_coinsLabel->setFixedSize(40, 40);
	m_coinsLabel->setStyleSheet("background-color: #FFD700; color: #333; "
		"border:2px solid #B8860B; border-radius:20px; font-weight: bold;");

	m_vpLabel = new QLabel(QString::number(vpVal), this);
	m_vpLabel->setAlignment(Qt::AlignCenter);
	m_vpLabel->setFixedSize(40, 40);
	m_vpLabel->setStyleSheet("background-color: #6EE7B7; color: #1F2937; "
		"border:2px solid #10B981; border-radius:20px; font-weight: bold;");

	if (m_isLeftPanel) {
		statsRow->addStretch(1);
		statsRow->addWidget(new QLabel("VP:", this));
		statsRow->addWidget(m_vpLabel);
		statsRow->addSpacing(8);
		statsRow->addWidget(new QLabel("Coins:", this));
		statsRow->addWidget(m_coinsLabel);
	}
	else {
		statsRow->addWidget(new QLabel("Coins:", this));
		statsRow->addWidget(m_coinsLabel);
		statsRow->addSpacing(8);
		statsRow->addWidget(new QLabel("VP:", this));
		statsRow->addWidget(m_vpLabel);
		statsRow->addStretch(1);
	}

	m_layout->addLayout(statsRow);
}

void PlayerPanelWidget::refreshCards()
{
	if (!m_player || !m_player->m_player) return;

	QListWidget* lists[] = { m_cardsBrown, m_cardsGrey, m_cardsRed, m_cardsYellow, m_cardsGreen, m_cardsBlue, m_cardsPurple };
	for (auto* l : lists) {
		if (l) l->clear();
	}

	const auto& owned = m_player->m_player->getOwnedCards();
	for (const auto& uptr : owned) {
		if (!uptr) continue;
		auto* list = listForColor(uptr->getColor());
		if (!list) continue;

		QString name = QStringBuilder(uptr->getName());
		auto* item = new QListWidgetItem(name);
		item->setToolTip(name);
		item->setSizeHint(QSize(24, 120));
		item->setData(Qt::BackgroundRole, QColor(ColorToCss(uptr->getColor())));
		list->addItem(item);
	}
}

void PlayerPanelWidget::addCardSections()
{
	auto makeSection = [&](Models::ColorType col, QListWidget*& outList) {
		QWidget* section = new QWidget(this);
		auto sectionLayout = new QVBoxLayout(section);
		sectionLayout->setContentsMargins(8, 8, 8, 8);
		sectionLayout->setSpacing(6);

		QString bg = this->palette().color(QPalette::Window).name();
		QString sectionStyle = QString(
			"background: qlineargradient(x1:%3, y1:0, x2:%4, y2:0, stop:0 %1,stop:0.05 %1, stop:0.2 %2);"
			"border:1px solid #374151; border-radius:6px;"
		).arg(ColorToCss(col)).arg(bg).arg(m_isLeftPanel ? 1 : 0).arg(m_isLeftPanel ? 0 : 1);
		section->setStyleSheet(sectionStyle);

		outList = new SpineList(section);
		outList->setFlow(QListView::LeftToRight);
		outList->setWrapping(true);
		outList->setResizeMode(QListView::Adjust);
		outList->setSpacing(4);
		outList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		outList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		outList->setSelectionMode(QAbstractItemView::SingleSelection);

		outList->setLayoutDirection(m_isLeftPanel ? Qt::RightToLeft : Qt::LeftToRight);

		outList->setStyleSheet(
			"QListWidget { background: transparent; border: none; }"
			"QListWidget::item { padding:0px; margin:0px; }"
			"QListWidget::item:selected { background: transparent; }"
		);
		outList->setItemDelegate(new CardSpineDelegate(outList));

		sectionLayout->addWidget(outList);
		m_layout->addWidget(section);
		};

	makeSection(Models::ColorType::BROWN, m_cardsBrown);
	makeSection(Models::ColorType::GREY, m_cardsGrey);
	makeSection(Models::ColorType::RED, m_cardsRed);
	makeSection(Models::ColorType::YELLOW, m_cardsYellow);
	makeSection(Models::ColorType::GREEN, m_cardsGreen);
	makeSection(Models::ColorType::BLUE, m_cardsBlue);
	makeSection(Models::ColorType::PURPLE, m_cardsPurple);

	refreshCards();
}
