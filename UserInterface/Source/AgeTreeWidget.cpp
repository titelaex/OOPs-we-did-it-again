#include "Header/AgeTreeWidget.h"
#include <QtCore/QDebug>
#include "Header/PlayerPanelWidget.h"
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsRectItem>
#include <QtWidgets/QGraphicsTextItem>
#include <QtWidgets/QGraphicsProxyWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLayout>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QApplication>
#include <QtGui/QPen>
#include <QtGui/QBrush>
#include <QtGui/QPainter>
#include <QtCore/QTimer>
#include <QtCore/QEvent>
#include <QtWidgets/QToolTip>
#include <QtWidgets/QGraphicsSceneHoverEvent>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QVariantAnimation>
#include <QtCore/QDebug>
#include <QtCore/QPointer>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <algorithm>
#include <vector>
#include <unordered_map>

import Core.Board;
import Core.Node;
import Core.Player;
import Core.GameState;
import Core.Game;
import Models.Card;
import Models.AgeCard;
import Models.ColorType;
import Models.ResourceType;

static QString resourceTypeToString(Models::ResourceType r)
{
	switch (r) {
	case Models::ResourceType::WOOD: return "Wood";
	case Models::ResourceType::STONE: return "Stone";
	case Models::ResourceType::CLAY: return "Clay";
	case Models::ResourceType::PAPYRUS: return "Papyrus";
	case Models::ResourceType::GLASS: return "Glass";
	default: return "Unknown";
	}
}

static QString buildCardText(const Models::Card* card)
{
	if (!card) return QString();

	QString text;
	text += "Name: " + QString::fromStdString(card->getName()) + "<br>";
	text += "Color: " + QString::fromStdString(Models::ColorTypeToString(card->getColor())) + "<br>";

	if (auto ageCard = dynamic_cast<const Models::AgeCard*>(card)) {
		text += "Cost: <br>";
		const auto& resourceCost = ageCard->getResourceCost();
		if (resourceCost.empty() && ageCard->getCoinCost() ==0) {
			text += " Free<br>";
		}
		else {
			for (const auto& cost : resourceCost) {
				text += QString(" - %1 x%2<br>").arg(resourceTypeToString(cost.first)).arg(cost.second);
			}
			if (ageCard->getCoinCost() >0) {
				text += QString(" - %1 Coins<br>").arg(ageCard->getCoinCost());
			}
		}

		const auto& resourceProduction = ageCard->getResourcesProduction();
		if (!resourceProduction.empty()) {
			text += "Production:<br>";
			for (const auto& prod : resourceProduction) {
				text += QString(" - %1 x%2<br>").arg(resourceTypeToString(prod.first)).arg(prod.second);
			}
		}

		auto effects = ageCard->getOnPlayActions();
		if (!effects.empty()) {
			text += "Effects:<br>";
			for (const auto& effect : effects) {
				if (effect.second.find("payCoins") != std::string::npos ||
					effect.second.find("getResource") != std::string::npos) {
					continue;
				}
				QString effectText = " - " + QString::fromStdString(effect.second);
				if (effect.second.find("getVictoryPoints") != std::string::npos) {
					effectText += " (" + QString::number(ageCard->getVictoryPoints()) + ")";
				}
				else if (effect.second.find("getShieldPoints") != std::string::npos) {
					effectText += " (" + QString::number(ageCard->getShieldPoints()) + ")";
				}
				else if (effect.second.find("getScientificSymbol") != std::string::npos) {
					if (ageCard->getScientificSymbols().has_value()) {
						auto symbol = ageCard->getScientificSymbols().value();
						effectText += " (" + QString::fromStdString(Models::ScientificSymbolTypeToString(symbol)) + ")";
					}
				}
				else if (effect.second.find("getTradeRule") != std::string::npos) {
					const auto& tradeRules = ageCard->getTradeRules();
					if (!tradeRules.empty()) {
						QString rules;
						for (const auto& rule : tradeRules) {
							if (rule.second) { 
								if (!rules.isEmpty()) {
									rules += ", ";
								}
								rules += QString::fromStdString(Models::tradeRuleTypeToString(rule.first));
							}
						}
						if (!rules.isEmpty()) {
							effectText += " (" + rules + ")";
						}
					}
				}
				text += effectText + "<br>";
			}
		}

		if (ageCard->getHasLinkingSymbol().has_value()) {
			text += "Chains to: " + QString::fromStdString(Models::LinkingSymbolTypeToString(ageCard->getHasLinkingSymbol().value())) + "<br>";
		}
	}

	return QString("<div style='width:250px;'>%1</div>").arg(text);
}

class ClickableRect : public QGraphicsRectItem {
public:
	ClickableRect(const QRectF& r)
		: QGraphicsRectItem(r)
	{
		setAcceptHoverEvents(true);
		setAcceptedMouseButtons(Qt::LeftButton);
		m_radius =8.0;
		m_pen = QPen(QColor("#7C4A1C"),2);
		m_topColor = QColor("#4B5563");
		m_bottomColor = QColor("#1F2937");

		m_shadow = new QGraphicsRectItem(r, this);
		m_shadow->setBrush(QBrush(QColor(0,0,0,40)));
		m_shadow->setPen(Qt::NoPen);
		m_shadow->setZValue(-1);
		m_shadow->setPos(4,4);
	}

	void setGradientColors(const QColor& top, const QColor& bottom) { m_topColor = top; m_bottomColor = bottom; update(); }
	void setBorderColor(const QColor& c) { m_pen.setColor(c); update(); }

	void setCard(Models::Card* card) { m_card = card; }

	std::function<void()> onClicked;

protected:
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override {
		Q_UNUSED(option);
		Q_UNUSED(widget);
		QRectF r = rect();
		QLinearGradient grad(r.topLeft(), r.bottomLeft());
		grad.setColorAt(0.0, m_topColor);
		grad.setColorAt(1.0, m_bottomColor);
		painter->setRenderHint(QPainter::Antialiasing, true);
		painter->setBrush(QBrush(grad));
		painter->setPen(m_pen);
		painter->drawRoundedRect(r, m_radius, m_radius);
	}

	void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override {
		if (m_card) {
			QString tooltipText = buildCardText(m_card);
			if (!tooltipText.isEmpty()) {
				setToolTip(tooltipText);
			}
		}
		QGraphicsRectItem::hoverEnterEvent(event);
	}

	void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override {
		QGraphicsRectItem::hoverLeaveEvent(event);
	}

	void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
		QTimer::singleShot(0, [cb = onClicked]() { if (cb) cb(); });
	}

private:
	Models::Card* m_card{ nullptr };
	QGraphicsRectItem* m_shadow{ nullptr };
	qreal m_radius{8.0 };
	QPen m_pen;
	QColor m_topColor;
	QColor m_bottomColor;
};

AgeTreeWidget::AgeTreeWidget(QWidget* parent)
	: QWidget(parent), m_view(nullptr), m_scene(nullptr)
{
	qDebug() << "AgeTreeWidget::ctor this=" << static_cast<const void*>(this);
}

AgeTreeWidget::~AgeTreeWidget()
{
	qDebug() << "AgeTreeWidget::dtor this=" << static_cast<const void*>(this);
}

void AgeTreeWidget::setPlayerPanels(PlayerPanelWidget* left, PlayerPanelWidget* right)
{
	m_leftPanel = left;
	m_rightPanel = right;
}

void AgeTreeWidget::setCurrentPlayerIndex(int index)
{
	m_currentPlayerIndex = index;
}

void AgeTreeWidget::refreshPanels()
{
	if (m_leftPanel) {
		m_leftPanel->refreshStats();
		m_leftPanel->refreshCards();
		m_leftPanel->refreshWonders();
		m_leftPanel->refreshTokens();
	}
	if (m_rightPanel) {
		m_rightPanel->refreshStats();
		m_rightPanel->refreshCards();
		m_rightPanel->refreshWonders();
		m_rightPanel->refreshTokens();
	}
}

void AgeTreeWidget::handleLeafClicked(int nodeIndex, int age)
{
	qDebug() << "AgeTreeWidget::handleLeafClicked called. currentPlayerIndex=" << m_currentPlayerIndex;

	auto& gs = Core::GameState::getInstance();
	auto cur = (m_currentPlayerIndex == 0) ? gs.GetPlayer1() : gs.GetPlayer2();
	if (!cur) return;
	Core::setCurrentPlayer(cur);

	auto& board = Core::Board::getInstance();
	const auto& nodes = (age ==1) ? board.getAge1Nodes() : (age ==2) ? board.getAge2Nodes() : board.getAge3Nodes();
	if (nodeIndex <0 || static_cast<size_t>(nodeIndex) >= nodes.size()) return;
	auto node = nodes[static_cast<size_t>(nodeIndex)];
	if (!node) return;
	auto card = node->getCard();
	if (!card.has_value()) return;

	while (true) {
		QMessageBox msg(this);
		msg.setWindowTitle("Choose action");
		msg.setText(QString::fromStdString(card->get().getName()));
		QPushButton* buildBtn = msg.addButton("Build", QMessageBox::ActionRole);
		QPushButton* sellBtn = msg.addButton("Sell", QMessageBox::ActionRole);
		QPushButton* wonderBtn = msg.addButton("Use as Wonder", QMessageBox::ActionRole);
		msg.addButton(QMessageBox::Cancel);
		msg.exec();

		QPushButton* clicked = qobject_cast<QPushButton*>(msg.clickedButton());
		if (!clicked) return;

		int action = -1;
		if (clicked == buildBtn) action =0;
		else if (clicked == sellBtn) action =1;
		else if (clicked == wonderBtn) action =2;
		else return;

		std::optional<size_t> wonderChoice;
		if (action ==2) {
			if (!cur->m_player) return;
			auto& owned = cur->m_player->getOwnedWonders();
			QStringList options;
			std::vector<size_t> candidates;
			for (size_t i =0; i < owned.size(); ++i) {
				if (owned[i] && !owned[i]->IsConstructed()) {
					candidates.push_back(i);
					options << QString::fromStdString(owned[i]->getName());
				}
			}
			if (candidates.empty()) {
				QMessageBox::information(this, "Use as Wonder", "No unbuilt wonders available.");
				continue;
			}

			bool ok = false;
			QString picked = QInputDialog::getItem(this, "Choose wonder", "Select wonder to build:", options,0, false, &ok);
			if (!ok) {
				continue;
			}

			int pickedIdx = options.indexOf(picked);
			if (pickedIdx <0 || pickedIdx >= static_cast<int>(candidates.size())) {
				continue;
			}
			wonderChoice = candidates[static_cast<size_t>(pickedIdx)];

			auto& gs2 = Core::GameState::getInstance();
			auto opp = (m_currentPlayerIndex == 0) ? gs2.GetPlayer2().get() : gs2.GetPlayer1().get();
			if (opp && opp->m_player) {
				const Models::Wonder* selectedWonder = (wonderChoice.has_value() && wonderChoice.value() < owned.size())
					? owned[wonderChoice.value()].get()
					: nullptr;
				if (selectedWonder) {
					const auto breakdown = Core::Game::computeWonderTradeCost(*cur, *selectedWonder, *opp);
					QString details;
					details += QString("Available coins: %1\n").arg(breakdown.availableCoins);
					details += QString("Total cost: %1\n").arg(breakdown.totalCost);
					if (breakdown.architectureTokenApplied) {
						details += "Architecture token applied (up to2 units free).\n";
					}
					if (!breakdown.lines.empty()) {
						details += "\nMissing resources to trade:\n";
						for (const auto& line : breakdown.lines) {
							details += QString("- %1 x%2 @ %3 each = %4\n")
								.arg(QString::fromStdString(Models::ResourceTypeToString(line.resource)))
								.arg(line.amount)
								.arg(line.costPerUnit)
								.arg(line.totalCost);
						}
					} else {
						details += "\nNo trading needed (resources covered).\n";
					}

					if (!breakdown.canAfford) {
						QMessageBox::warning(this,
							"Insufficient coins",
							"You do not have enough coins to buy the missing resources for this wonder.\n\n" + details);
						continue;
					}

					auto reply = QMessageBox::question(this,
						"Confirm wonder build",
						"Build this wonder by paying the trade cost?\n\n" + details,
						QMessageBox::Yes | QMessageBox::No);
					if (reply != QMessageBox::Yes) {
						continue;
					}
				}
			}
		}

		bool ok = Core::Game::applyTreeCardAction(age, nodeIndex, action, wonderChoice);
		if (!ok) {
			QMessageBox::warning(this, "Action failed", "Action failed (insufficient resources or invalid choice). Please choose another action.");
			card = node->getCard();
			if (!card.has_value()) return;
			continue;
		}

		if (action ==0 && card) { 
			auto* ageCard = dynamic_cast<const Models::AgeCard*>(card);
			if (ageCard && ageCard->getScientificSymbols().has_value()) {
				auto targetSymbol = ageCard->getScientificSymbols().value();
				
				int symbolCount =0;
				const auto& inventory = cur->m_player->getOwnedCards();
				for (const auto& ownedCardPtr : inventory) {
					if (auto* ownedAgeCard = dynamic_cast<const Models::AgeCard*>(ownedCardPtr.get())) {
						auto sym = ownedAgeCard->getScientificSymbols();
						if (sym.has_value() && sym.value() == targetSymbol) {
							symbolCount++;
						}
					}
				}
				
				if (symbolCount ==2) {
					QString playerName = (m_currentPlayerIndex ==0 && gs.GetPlayer1() && gs.GetPlayer1()->m_player)
						? QString::fromStdString(gs.GetPlayer1()->m_player->getPlayerUsername())
						: (gs.GetPlayer2() && gs.GetPlayer2()->m_player) 
							? QString::fromStdString(gs.GetPlayer2()->m_player->getPlayerUsername()) 
							: QString("Player");
					
					QMessageBox::information(this, 
						"Simboluri Stiintifice!", 
						QString("%1, ai obtinut 2 simboluri stiintifice la fel. Alege un token de pe tabla!").arg(playerName));
					
					int playerIndexForToken = m_currentPlayerIndex;
					if (onRequestTokenSelection) {
						onRequestTokenSelection([playerIndexForToken, this](int tokenIndex) {
							QTimer::singleShot(0, this, [playerIndexForToken, tokenIndex, this]() {
								if (onDisableTokenSelection) onDisableTokenSelection();
								
								auto& gsCallback = Core::GameState::getInstance();
								Core::Player* tokenPlayer = (playerIndexForToken ==0)
									? gsCallback.GetPlayer1().get()
									: gsCallback.GetPlayer2().get();
								if (!tokenPlayer || !tokenPlayer->m_player) {
									return;
								}
								
								auto& boardRef = Core::Board::getInstance();
								auto& availableTokens = const_cast<std::vector<std::unique_ptr<Models::Token>>&>(boardRef.getProgressTokens());
								if (tokenIndex <0 || tokenIndex >= static_cast<int>(availableTokens.size())) {
									return;
								}
								if (!availableTokens[tokenIndex]) {
									return;
								}
								
								auto chosenToken = std::move(availableTokens[tokenIndex]);
								availableTokens.erase(availableTokens.begin() + tokenIndex);
								if (!chosenToken) {
									return;
								}
								
								std::string tokenName;
								std::string tokenDesc;
								try {
									tokenName = chosenToken->getName();
									tokenDesc = chosenToken->getDescription();
								} catch (...) {
									return;
								}
								
								tokenPlayer->m_player->addToken(std::move(chosenToken));
								
								Core::TokenEvent tokenEvent;
								tokenEvent.playerID = static_cast<int>(tokenPlayer->m_player->getkPlayerId());
								tokenEvent.playerName = tokenPlayer->m_player->getPlayerUsername();
								tokenEvent.tokenName = tokenName;
								tokenEvent.tokenType = "PROGRESS";
								tokenEvent.tokenDescription = tokenDesc;
								Core::Game::getNotifier().notifyTokenAcquired(tokenEvent);
								
								refreshPanels();
							});
						});
					}
				}
			}
		}
		break;
	}

	refreshPanels();

	m_currentPlayerIndex = (m_currentPlayerIndex == 0) ? 1 : 0;
	auto newCur = (m_currentPlayerIndex == 0) ? gs.GetPlayer1() : gs.GetPlayer2();
	Core::setCurrentPlayer(newCur);

	if (onPlayerTurnChanged) {
		auto p1 = gs.GetPlayer1();
		auto p2 = gs.GetPlayer2();
		QString curName = (m_currentPlayerIndex ==0 && p1 && p1->m_player)
			? QString::fromStdString(p1->m_player->getPlayerUsername())
			: (p2 && p2->m_player) ? QString::fromStdString(p2->m_player->getPlayerUsername()) : QString("<unknown>");
		onPlayerTurnChanged(m_currentPlayerIndex, curName);
	}

	QTimer::singleShot(0, this, [this, age]() { this->showAgeTree(age); });
}

void AgeTreeWidget::showAgeTree(int age)
{
	qDebug() << "AgeTreeWidget::showAgeTree age=" << age << "this=" << static_cast<const void*>(this)
		<< " m_scene=" << static_cast<const void*>(m_scene) << " m_view=" << static_cast<const void*>(m_view);

	static bool s_phase2Shown = false;
	static bool s_phase3Shown = false;

	auto clearWidgetSurface = [&]() {
		if (auto oldLayout = this->layout()) {
			while (auto item = oldLayout->takeAt(0)) {
				if (auto w = item->widget()) w->deleteLater();
				delete item;
			}
			delete oldLayout;
		}

		if (m_view) {
			m_view->setScene(nullptr);
			m_view->deleteLater();
			m_view = nullptr;
		}
		if (m_scene) {
			m_scene->deleteLater();
			m_scene = nullptr;
		}
		m_proxyMap.clear();
		};

	auto showPhaseMessage = [&](const QString& text, int phaseToShowTree) {
		clearWidgetSurface();

		auto* msgLayout = new QVBoxLayout(this);
		msgLayout->setContentsMargins(0,0,0,0);
		msgLayout->setSpacing(0);
		msgLayout->setAlignment(Qt::AlignCenter);

		QLabel* phaseMsg = new QLabel(text, this);
		phaseMsg->setAlignment(Qt::AlignCenter);
		phaseMsg->setStyleSheet("color: white;");
		QFont f = phaseMsg->font();
		f.setPointSize(24);
		f.setBold(true);
		phaseMsg->setFont(f);
		phaseMsg->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

		msgLayout->addWidget(phaseMsg);
		setLayout(msgLayout);

		QPointer<AgeTreeWidget> guard(this);
		QTimer::singleShot(1800, this, [guard, phaseToShowTree]() {
			if (guard) guard->showAgeTree(phaseToShowTree);
			});
		};

	if (age ==2 && !s_phase2Shown) {
		s_phase2Shown = true;
		showPhaseMessage("Phase2 incepe",2);
		return;
	}
	if (age ==3 && !s_phase3Shown) {
		s_phase3Shown = true;
		showPhaseMessage("Phase3 incepe",3);
		return;
	}

	clearWidgetSurface();

	m_currentAge = age;
	auto& board = Core::Board::getInstance();
	const auto* nodesPtr = (age ==1) ? &board.getAge1Nodes() : (age ==2) ? &board.getAge2Nodes() : &board.getAge3Nodes();
	const auto& nodes = *nodesPtr;

	if (age == 2) {
		bool anyAvailable = false;
		for (const auto& n : nodes) {
			if (!n) continue;
			auto c = n->getCard();
			if (!c.has_value()) continue;
			if (n->isAvailable() && c->get().isAvailable()) { anyAvailable = true; break; }
		}
		if (!anyAvailable) {
			QPointer<AgeTreeWidget> guard(this);
			QTimer::singleShot(0, this, [guard]() {
				if (guard) guard->showAgeTree(3);
				});
			return;
		}
	}

	std::vector<int> rows;
	bool flipVertical = false;
	if (age ==1) {
		rows = {2,3,4,5,6 };
	}
	else if (age ==2) {
		rows = {6,5,4,3,2 };
	}
	else {
		rows = { 2,3,4,2,4,3,2 };
		flipVertical = false;
	}

	QColor invisibleBorderColor = QColor("#CCCCCC");
	QColor lineColor = QColor("#4B5563");

	auto paletteForCardColor = [](Models::ColorType c) -> std::tuple<QColor, QColor, QColor, QColor> {
		switch (c) {
		case Models::ColorType::BROWN: return { QColor("#B58860"), QColor("#7C4A1C"), QColor("#7C4A1C"), QColor("#FFFFFF") };
		case Models::ColorType::GREY: return { QColor("#9CA3AF"), QColor("#4B5563"), QColor("#374151"), QColor("#111827") };
		case Models::ColorType::RED: return { QColor("#F87171"), QColor("#B91C1C"), QColor("#7F1D1D"), QColor("#FFFFFF") };
		case Models::ColorType::YELLOW: return { QColor("#FCD34D"), QColor("#D97706"), QColor("#92400E"), QColor("#111827") };
		case Models::ColorType::GREEN: return { QColor("#34D399"), QColor("#047857"), QColor("#065F46"), QColor("#FFFFFF") };
		case Models::ColorType::BLUE: return { QColor("#60A5FA"), QColor("#1E3A8A"), QColor("#0369A1"), QColor("#FFFFFF") };
		case Models::ColorType::PURPLE: return { QColor("#A78BFA"), QColor("#5B21B6"), QColor("#4C1D95"), QColor("#FFFFFF") };
		default: return { QColor("#9CA3AF"), QColor("#4B5563"), QColor("#374151"), QColor("#FFFFFF") };
		}
		};

	if (auto oldLayout = this->layout()) {
		while (auto item = oldLayout->takeAt(0)) {
			if (auto w = item->widget()) {
				w->removeEventFilter(this);
			}
			delete item;
		}
		delete oldLayout;
	}

	if (m_view) {
		m_view->setScene(nullptr);
		m_view->deleteLater();
		m_view = nullptr;
	}
	if (m_scene) {
		qDebug() << "AgeTreeWidget: scheduling deleteLater on old scene";
		m_scene->deleteLater();
		m_scene = nullptr;
	}
	m_proxyMap.clear();

	m_scene = new QGraphicsScene(this);
	m_scene->setBackgroundBrush(Qt::NoBrush);
	m_view = new QGraphicsView(m_scene, this);
	qDebug() << "AgeTreeWidget: created scene=" << static_cast<const void*>(m_scene) << " view=" << static_cast<const void*>(m_view);
	m_view->setRenderHint(QPainter::Antialiasing);
	m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setFrameStyle(QFrame::NoFrame);
	m_view->setAlignment(Qt::AlignCenter);
	m_view->setStyleSheet("background: transparent;");
	m_view->setAttribute(Qt::WA_TranslucentBackground);
	if (m_view->viewport()) {
		m_view->viewport()->setStyleSheet("background: transparent;");
		m_view->viewport()->setAttribute(Qt::WA_TranslucentBackground);
	}

	const int cardW =180;
	const int cardH =110;
	const int hgap =28;
	const int vgap =30;

	if (nodes.empty()) {
		m_scene->setSceneRect(0,0,800,400);
		QGraphicsTextItem* t = m_scene->addText("No cards to display.");
		QFont f = t->font();
		f.setPointSize(18);
		f.setBold(true);
		t->setFont(f);
		t->setDefaultTextColor(QColor("#FFFFFF"));
		QRectF tb = t->boundingRect();
		t->setPos((800 - tb.width()) /2.0, (400 - tb.height()) /2.0);

		auto* vlayout = new QVBoxLayout(this);
		vlayout->setContentsMargins(8,8,8,8);
		vlayout->setSpacing(0);
		vlayout->addWidget(m_view);
		setLayout(vlayout);

		QPointer<AgeTreeWidget> guard(this);
		QTimer::singleShot(0, [guard]() { if (guard) guard->fitAgeTree(); });
		if (m_view && m_view->viewport()) m_view->viewport()->installEventFilter(this);
		return;
	}

	std::vector<QPointF> positions(nodes.size());

	int totalRows = static_cast<int>(rows.size());
	int idx =0;
	int sceneWidth =0;
	for (int r =0; r < totalRows; ++r) {
		int cols = rows[r];
		int rowWidth = cols * cardW + (cols -1) * hgap;
		sceneWidth = std::max(sceneWidth, rowWidth);
	}

	const int totalHeight = totalRows * cardH + (totalRows -1) * vgap;

	int y =0;
	for (int r =0; r < totalRows; ++r) {
		int cols = rows[r];
		int rowWidth = cols * cardW + (cols -1) * hgap;
		int x0 = (sceneWidth - rowWidth) /2;
		int rowY = y;
		if (flipVertical) {
			rowY = (totalHeight - cardH) - y;
		}
		for (int c =0; c < cols; ++c) {
			if (idx >= static_cast<int>(nodes.size())) break;
			int x = x0 + c * (cardW + hgap);
			positions[idx] = QPointF(x, rowY);
			++idx;
		}
		y += cardH + vgap;
	}

	m_scene->setSceneRect(0, 0, std::max(sceneWidth, 800), std::max(totalHeight + vgap, 400));

	std::unordered_map<Core::Node*, int> ptrToIndex;
	for (size_t i =0; i < nodes.size(); ++i) ptrToIndex[nodes[i].get()] = static_cast<int>(i);

	std::vector<QGraphicsRectItem*> rects(nodes.size(), nullptr);
	idx =0;
	for (int r =0; r < totalRows; ++r) {
		int cols = rows[r];
		for (int c =0; c < cols; ++c) {
			if (idx >= static_cast<int>(nodes.size())) break;
			QPointF pos = positions[idx];
			QRectF rrect(pos, QSizeF(cardW, cardH));

			if (!nodes[idx] || !nodes[idx]->getCard()) {
				rects[idx] = nullptr;
				++idx;
				continue;
			}

			auto cardOpt = nodes[idx]->getCard();
			if (!cardOpt.has_value()) continue;
			Models::Card* cardPtr = &cardOpt->get();

			const bool leafAvailable = nodes[idx]->isAvailable();
			const bool isVisible = cardPtr->isVisible();
			const bool isSelectable = leafAvailable && cardPtr->isAvailable();

			QGraphicsRectItem* baseItem = nullptr;

			if (isSelectable) {
				auto* item = new ClickableRect(rrect);
				item->setCard(cardPtr);
				item->setZValue(1);
				m_scene->addItem(item);
				baseItem = item;

				auto [top, bottom, border, text] = paletteForCardColor(cardPtr->getColor());
				item->setGradientColors(top, bottom);
				item->setBorderColor(border);

				if (isVisible) {
					QString name = QString::fromStdString(cardPtr->getName());
					QGraphicsTextItem* t = m_scene->addText(name);
					QFont f = t->font(); f.setBold(true); f.setPointSize(15);
					t->setFont(f);
					t->setDefaultTextColor(text);
					QRectF tb = t->boundingRect();
					t->setPos(pos.x() + (cardW - tb.width()) /2, pos.y() + (cardH - tb.height()) /2);
					t->setZValue(2);
				}

				int nodeIndexToUse = idx;
				item->onClicked = [this, nodeIndexToUse]() {
					this->handleLeafClicked(nodeIndexToUse, m_currentAge);
				};
			}
			else {
				if (isVisible) {
					auto* item = new ClickableRect(rrect);
					item->setCard(cardPtr);
					item->setZValue(1);
					m_scene->addItem(item);
					baseItem = item;

					auto [top, bottom, border, text] = paletteForCardColor(cardPtr->getColor());
					item->setGradientColors(top, bottom);
					item->setBorderColor(border);

					QString name = QString::fromStdString(cardPtr->getName());
					QGraphicsTextItem* t = m_scene->addText(name);
					QFont f = t->font(); f.setBold(true); f.setPointSize(15);
					t->setFont(f);
					t->setDefaultTextColor(text);
					QRectF tb = t->boundingRect();
					t->setPos(pos.x() + (cardW - tb.width()) /2, pos.y() + (cardH - tb.height()) /2);
					t->setZValue(2);
				}
				else {
					QColor bg = QColor("#EDE7E0");
					QGraphicsRectItem* ritem = m_scene->addRect(rrect, QPen(QColor("#374151"),3), QBrush(bg));
					ritem->setZValue(1);
					baseItem = ritem;
				}
			}

			rects[idx] = baseItem;
			++idx;
		}
	}

	auto* vlayout = new QVBoxLayout(this);
	vlayout->setContentsMargins(8,8,8,8);
	vlayout->setSpacing(0);
	vlayout->addWidget(m_view);
	setLayout(vlayout);

	QPointer<AgeTreeWidget> guard(this);
	QTimer::singleShot(0, [guard]() {
		if (guard) guard->fitAgeTree();
	});

	if (m_view && m_view->viewport()) m_view->viewport()->installEventFilter(this);
}

void AgeTreeWidget::fitAgeTree()
{
	if (!m_view || !m_scene) return;
	m_view->resetTransform();
	m_view->fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
	m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

bool AgeTreeWidget::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::Enter) {
		if (auto w = qobject_cast<QWidget*>(obj)) {
			auto it = m_proxyMap.find(w);
			if (it != m_proxyMap.end()) {
				QGraphicsProxyWidget* proxy = it->second;
				proxy->setTransformOriginPoint(proxy->boundingRect().center());
				QPropertyAnimation* anim = new QPropertyAnimation(proxy, "scale", proxy);
				anim->setDuration(140);
				anim->setStartValue(proxy->scale());
				anim->setEndValue(1.08);
				anim->setEasingCurve(QEasingCurve::OutCubic);
				anim->start(QAbstractAnimation::DeleteWhenStopped);
			}
		}
	}
	else if (event->type() == QEvent::Leave) {
		if (auto w = qobject_cast<QWidget*>(obj)) {
			auto it = m_proxyMap.find(w);
			if (it != m_proxyMap.end()) {
				QGraphicsProxyWidget* proxy = it->second;
				QPropertyAnimation* anim = new QPropertyAnimation(proxy, "scale", proxy);
				anim->setDuration(120);
				anim->setStartValue(proxy->scale());
				anim->setEndValue(1.0);
				anim->setEasingCurve(QEasingCurve::OutCubic);
				anim->start(QAbstractAnimation::DeleteWhenStopped);
			}
		}
	}
	else if (m_view && obj == m_view->viewport() && event->type() == QEvent::Resize) {
		fitAgeTree();
	}

	return QWidget::eventFilter(obj, event);
}