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
#include <QShowEvent>
#include <QMetaObject>
#include <QPixmap>

class SpineList : public QListWidget {
public:
	explicit SpineList(QWidget* parent = nullptr, int spineWidth = 20)
		: QListWidget(parent), m_spineWidth(spineWidth)
	{
		setViewMode(QListView::IconMode);
		setResizeMode(QListView::Fixed);
		setUniformItemSizes(true);
	}
	
	void forceUpdateSizes() {
		updateItemSizes();
	}
	
protected:
	void resizeEvent(QResizeEvent* event) override {
		QListWidget::resizeEvent(event);
		updateItemSizes();
	}
	void showEvent(QShowEvent* event) override {
		QListWidget::showEvent(event);
		updateItemSizes();
	}
private:
	void updateItemSizes() {
		QSize sz(m_spineWidth, 45);
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

PlayerPanelWidget::PlayerPanelWidget(std::shared_ptr<Core::Player> player, QWidget* parent, bool isLeftPanel)
	: QWidget(parent), m_player(player)
{
	m_isLeftPanel = isLeftPanel;
	
	setStyleSheet(
		"PlayerPanelWidget {"
		"  background-color: #1e1e1e;" 
		"  border-radius: 12px;"
		"  border: 2px solid #374151;"
		"}"
	);
	
	m_layout = new QVBoxLayout(this);
	m_layout->setContentsMargins(8, 8, 8, 8);
	m_layout->setSpacing(10);
	buildUi();
}

void PlayerPanelWidget::buildUi()
{
	QString username;
	
	if (m_player && m_player->m_player) {
		QString playerUsername = QStringBuilder(m_player->m_player->getPlayerUsername());
		// If username is empty or whitespace, use default
		if (playerUsername.trimmed().isEmpty()) {
			username = m_isLeftPanel ? QString("Player 1") : QString("Player 2");
		} else {
			username = playerUsername;
		}
	} else {
		username = m_isLeftPanel ? QString("Player 1") : QString("Player 2");
	}

	m_usernameLabel = new QLabel(username, this); // Store the label!
	m_usernameLabel->setStyleSheet("font-size:18px; font-weight: bold; margin-bottom:3px;");
	m_usernameLabel->setAlignment(m_isLeftPanel ? Qt::AlignRight : Qt::AlignLeft);
	m_layout->addWidget(m_usernameLabel);

	addStatsRow();
	addProgressTokensSection();
	addCardSections();
	addWonderSection();
}

void PlayerPanelWidget::refreshUsername()
{
	if (!m_usernameLabel) return;
	
	QString username;
	
	if (m_player && m_player->m_player) {
		QString playerUsername = QStringBuilder(m_player->m_player->getPlayerUsername());
		// If username is empty or whitespace, use default
		if (playerUsername.trimmed().isEmpty()) {
			username = m_isLeftPanel ? QString("Player 1") : QString("Player 2");
		} else {
			username = playerUsername;
		}
	} else {
		username = m_isLeftPanel ? QString("Player 1") : QString("Player 2");
	}
	
	m_usernameLabel->setText(username);
}

void PlayerPanelWidget::addWonderSection()
{
	QWidget* wondersSection = new QWidget(this);

	m_wondersGrid = new QGridLayout(wondersSection);
	m_wondersGrid->setContentsMargins(4,4,4,4);
	m_wondersGrid->setSpacing(6);

	m_layout->addWidget(wondersSection);

	refreshWonders();
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
	statsRow->setSpacing(8);

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
		// Size will be set by SpineList::updateItemSizes()
		item->setData(Qt::BackgroundRole, QColor(ColorToCss(uptr->getColor())));
		list->addItem(item);
	}
	
	// Force update of item sizes after adding items
	for (auto* l : lists) {
		if (l && l->count() > 0) {
			if (auto* spine = dynamic_cast<SpineList*>(l)) {
				spine->forceUpdateSizes();
			}
		}
	}
}
void PlayerPanelWidget::refreshWonders()
{
	if (!m_wondersGrid) return;

	// Check if title exists, if not create it
	QLayoutItem* titleItem = m_wondersGrid->itemAtPosition(0, 0);
	if (!titleItem || !titleItem->widget()) {
		auto title = new QLabel("Wonders:", this);
		title->setStyleSheet("font-weight: bold; margin-right:6px; font-size:12px;");
		title->setAlignment(m_isLeftPanel ? Qt::AlignRight : Qt::AlignLeft);
		m_wondersGrid->addWidget(title, 0, 0, 1, 2);
	}

	bool hasPlayer = (m_player && m_player->m_player);

	for (int i = 0; i < 4; ++i) {
		int row = (i / 2) + 1;
		int col = i % 2;

		if (m_isLeftPanel) {
			col = 1 - col;
		}

		// Find or create the slot widget
		QLayoutItem* item = m_wondersGrid->itemAtPosition(row, col);
		QLabel* slot = nullptr;

		if (item && item->widget()) {
			slot = qobject_cast<QLabel*>(item->widget());
		}

		if (!slot) {
			slot = new QLabel(this);
			slot->setAlignment(Qt::AlignCenter);
			slot->setMinimumSize(110, 80);
			slot->setMaximumSize(130, 90);
			slot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			slot->setWordWrap(true);
			m_wondersGrid->addWidget(slot, row, col);
		}

		QString emptyStyle = "background-color: rgba(255,255,255,0.04); border:1px solid #374151; border-radius:6px; color:#9CA3AF; font-style:italic; font-size:10px; padding:2px;";

		if (hasPlayer) {
			auto& wonders = m_player->m_player->getOwnedWonders();

			if (static_cast<size_t>(i) < wonders.size() && wonders[i]) {
				const auto* w = wonders[i].get();
				QString name = QStringBuilder(w->getName());
				bool built = w->IsConstructed();

				// Try to load wonder image
				QString imagePath = QString("Resources/wonders/%1.png").arg(name);
				QPixmap wonderPixmap(imagePath);

				// If file system load fails, try from Qt resources
				if (wonderPixmap.isNull()) {
					imagePath = QString(":/wonders/%1.png").arg(name);
					wonderPixmap = QPixmap(imagePath);
				}

				if (!wonderPixmap.isNull()) {
					// Scale image to fit the slot
					QPixmap scaledPixmap = wonderPixmap.scaled(110, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
					slot->setPixmap(scaledPixmap);
					slot->setText(""); // Clear text when showing image

					QString style;
					if (built) {
						// Gold border for constructed wonders
						style = "border:3px solid #FFD700; border-radius:6px; padding:2px; background: transparent;";
					}
					else {
						// Brown border for unconstructed wonders
						style = "border:1px solid #374151; border-radius:6px; padding:2px; background: transparent;";
					}
					slot->setStyleSheet(style);
				}
				else {
					// Fallback to text if no image
					slot->setPixmap(QPixmap()); // Clear pixmap
					slot->setText(name + (built ? "\n✓" : ""));

					QString style;
					if (built) {
						// Gold highlight for constructed wonders
						style = QString(
							"background: qlineargradient(x1:%1, y1:0, x2:%2, y2:0, stop:0 #FFD700, stop:1 #B8860B);"
							"border:2px solid #8B6508; border-radius:6px; color:#111827; font-weight:700; font-size:10px; padding:2px;")
							.arg(m_isLeftPanel ? 1 : 0)
							.arg(m_isLeftPanel ? 0 : 1);
					}
					else {
						QString bg = ColorToCss(w->getColor());
						style = QString(
							"background: qlineargradient(x1:%1, y1:0, x2:%2, y2:0, stop:0 %3, stop:1 rgba(0,0,0,0.15));"
							"border:1px solid #111; border-radius:6px; color:#fff; font-weight:600; font-size:10px; padding:2px;")
							.arg(m_isLeftPanel ? 1 : 0)
							.arg(m_isLeftPanel ? 0 : 1)
							.arg(bg);
					}
					slot->setStyleSheet(style);
				}

				slot->setToolTip(name);
			}
			else {
				slot->setPixmap(QPixmap()); // Clear pixmap
				slot->setText("<empty>");
				slot->setToolTip(QString());
				slot->setStyleSheet(emptyStyle);
			}
		}
		else {
			slot->setPixmap(QPixmap()); // Clear pixmap
			slot->setText("<empty>");
			slot->setToolTip(QString());
			slot->setStyleSheet(emptyStyle);
		}
	}
}

void PlayerPanelWidget::addProgressTokensSection()
{
	// Create section only if it doesn't exist
	if (m_tokensSection) {
		return; // Already exists, will be updated by refreshTokens()
	}

	m_tokensSection = new QWidget(this);
	auto* tokensLayout = new QHBoxLayout(m_tokensSection);
	tokensLayout->setContentsMargins(4, 4, 4, 4);
	tokensLayout->setSpacing(8);
	tokensLayout->setAlignment(m_isLeftPanel ? Qt::AlignRight : Qt::AlignLeft);

	// Create 3 placeholder labels that we'll update in refreshTokens()
	for (int i = 0; i < 3; ++i) {
		auto tokenLbl = new QLabel(QString(), m_tokensSection);
		tokenLbl->setObjectName(QString("token_%1").arg(i)); // Give unique names for finding later
		tokenLbl->setFixedSize(36, 36);
		tokenLbl->setAlignment(Qt::AlignCenter);
		tokensLayout->addWidget(tokenLbl);
	}

	// Insert after stats row (position 2, after username=0 and stats=1)
	m_layout->insertWidget(2, m_tokensSection);
}

void PlayerPanelWidget::refreshTokens()
{
	if (!m_tokensSection) {
		addProgressTokensSection();
	}

	QString baseStyle = "color:#F9FAFB; font-size:11px; padding:0;";
	QString circleStyleEmpty = baseStyle + "border:2px dotted #9CA3AF; border-radius:18px; width:36px; height:36px; background-color:transparent;";

	QList<const Models::Token*> tokens;

	if (m_player && m_player->m_player) {
		const auto& ownedTokens = m_player->m_player->getOwnedTokens();
		for (const auto& t : ownedTokens) {
			if (!t) continue;
			tokens.append(t.get());
			if (tokens.size() >= 3) break;
		}
	}

	// Update the 3 token labels
	for (int i = 0; i < 3; ++i) {
		auto tokenLbl = m_tokensSection->findChild<QLabel*>(QString("token_%1").arg(i));
		if (!tokenLbl) continue;

		if (i < tokens.size()) {
			const Models::Token* t = tokens[i];
			QString tokenName = QStringBuilder(t->getName());

			// Try to load token image
			QString imagePath = QString("Resources/tokens/%1.png").arg(tokenName);
			QPixmap tokenPixmap(imagePath);

			// If file system load fails, try from Qt resources
			if (tokenPixmap.isNull()) {
				imagePath = QString(":/tokens/%1.png").arg(tokenName);
				tokenPixmap = QPixmap(imagePath);
			}

			tokenLbl->setToolTip(tokenName);

			if (!tokenPixmap.isNull()) {
				// Scale image to fit the circle
				QPixmap scaledPixmap = tokenPixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
				tokenLbl->setPixmap(scaledPixmap);
				tokenLbl->setText("");
				tokenLbl->setStyleSheet(baseStyle + "border:2px solid #0ea5e9; border-radius:18px; background-color:#0284c7; padding:2px;");
			}
			else {
				// Fallback to initials if image not found
				QString initials = tokenName.section(' ', 0, 0).left(2).toUpper();
				tokenLbl->setPixmap(QPixmap()); // Clear any existing pixmap
				tokenLbl->setText(initials);
				tokenLbl->setStyleSheet(baseStyle + "border:2px dotted #9CA3AF; border-radius:18px; background-color:#4B5563;");
			}
		}
		else {
			// Empty slot
			tokenLbl->setPixmap(QPixmap()); // Clear any existing pixmap
			tokenLbl->setText(QString());
			tokenLbl->setToolTip(QString());
			tokenLbl->setStyleSheet(circleStyleEmpty);
		}
	}
}

void PlayerPanelWidget::refreshStats()
{
	if (!m_player || !m_player->m_player) return;

	auto coinsTuple = m_player->m_player->getRemainingCoins();
	uint32_t coinsVal = static_cast<uint32_t>(m_player->m_player->totalCoins(coinsTuple));
	int vpVal = static_cast<int>(m_player->m_player->getTotalVictoryPoints());

	// Block signals during updates to prevent cascading repaints
	if (m_coinsLabel) {
		m_coinsLabel->blockSignals(true);
		m_coinsLabel->setText(QString::number(coinsVal));
		m_coinsLabel->blockSignals(false);
	}
	if (m_vpLabel) {
		m_vpLabel->blockSignals(true);
		m_vpLabel->setText(QString::number(vpVal));
		m_vpLabel->blockSignals(false);
	}
}


void PlayerPanelWidget::addCardSections()
{
	auto makeSection = [&](Models::ColorType col, QListWidget*& outList) {
		QWidget* section = new QWidget(this);
		auto sectionLayout = new QVBoxLayout(section);
		sectionLayout->setContentsMargins(4,4,4,4);
		sectionLayout->setSpacing(0);
		section->setMinimumHeight(54);
		section->setMaximumHeight(54);
		section->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

		QString bg = this->palette().color(QPalette::Window).name();
		QString sectionStyle = QString(
			"background: qlineargradient(x1:%3, y1:0, x2:%4, y2:0, stop:0 %1,stop:0.05 %1, stop:0.2 %2);"
			"border:1px solid #374151; border-radius:6px;"
		).arg(ColorToCss(col)).arg(bg).arg(m_isLeftPanel ?1 :0).arg(m_isLeftPanel ?0 :1);
		section->setStyleSheet(sectionStyle);

		outList = new SpineList(section, 20);
		outList->setFlow(QListView::LeftToRight);
		outList->setWrapping(false);
		outList->setResizeMode(QListView::Fixed);
		outList->setSpacing(3);
		outList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		outList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		outList->setSelectionMode(QAbstractItemView::NoSelection);
		outList->setFixedHeight(50);

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
