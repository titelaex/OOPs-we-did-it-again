#include "WonderSelectionWidget.h"
#include <QDebug>
#include <QToolTip>
#include <QCursor>
#include <QString>
#include <QEvent>
import Models.ResourceType;

WonderSelectionWidget::WonderSelectionWidget(QWidget* parent): QWidget(parent)
{
	m_mainLayout = new QVBoxLayout(this);
	m_mainLayout->setAlignment(Qt::AlignCenter);
	m_mainLayout->setSpacing(20);

	m_infoLabel = new QLabel("Se pregateste jocul...", this);
	m_infoLabel->setAlignment(Qt::AlignCenter);
	m_infoLabel->setStyleSheet("font-size:24px; font-weight: bold; color: white; margin-bottom:10px;");
	m_mainLayout->addWidget(m_infoLabel);

	m_cardsLayout = new QHBoxLayout();
	m_cardsLayout->setSpacing(15);
	m_cardsLayout->setAlignment(Qt::AlignCenter);

	m_mainLayout->addLayout(m_cardsLayout);
}

void WonderSelectionWidget::setTurnMessage(QString message)
{
	if (m_infoLabel)
		m_infoLabel->setText(message);
}
static QString resourceToString(Models::ResourceType r)
{
	switch (r) {
	case Models::ResourceType::WOOD:    return "Wood";
	case Models::ResourceType::STONE:   return "Stone";
	case Models::ResourceType::CLAY:    return "Clay";
	case Models::ResourceType::PAPYRUS: return "Papyrus";
	case Models::ResourceType::GLASS:   return "Glass";
	default: return QString("Type %1").arg(static_cast<int>(r));
	}
}
static QString buildWonderText(Models::Wonder* w)
{
	if (!w) return QString();
	QString name = QString::fromStdString(w->getName());
	QString caption = QString::fromStdString(w->getCaption());
	int shields = static_cast<int>(w->getShieldPoints());
	int victoryPoints = static_cast<int>(w->getVictoryPoints());

	QString text;
	text += name + "\n";
	if (!caption.isEmpty()) text += caption + "\n";
	text += QString("Shield points: %1\n").arg(shields);
	text += QString("Victory points: %1\n").arg(victoryPoints);
	
	text += "Resource Cost:";
	const auto& cost = w->getResourceCost();
	if (cost.empty()) {
		text += " No resource cost\n";
	}
	else {
		for (const auto& kv : cost) {
			const auto res = kv.first;
			const auto amount = kv.second;
			text += QString(" • %1 x %2\n").arg(static_cast<int>(amount)).arg(resourceToString(res));
		}
	}
	return text;
}

void WonderSelectionWidget::loadWonders(std::vector<Models::Wonder*> wonders)
{
	qDeleteAll(m_buttons);
	m_buttons.clear();
	m_wonders.clear();
	QLayoutItem* item;
	while ((item = m_cardsLayout->takeAt(0)) != nullptr) {
		delete item->widget();
		delete item;
	}

	for (size_t i = 0; i < wonders.size(); ++i) {
		auto w = wonders[i];
		m_wonders.push_back(w);

		QPushButton* btn = new QPushButton(this);
		btn->setFixedSize(180, 110);

		if (w) {
			btn->setText(QString::fromStdString(w->getName()));
			btn->setStyleSheet(
				"QPushButton {"
				" background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4B5563, stop:1 #1F2937);"
				" border:2px solid #9CA3AF;"
				" border-radius:8px;"
				" color: white;"
				" font-weight: bold;"
				" font-size:15px;"
				"}"
				"QPushButton:hover { border:2px solid #F59E0B; }"
			);

			btn->setProperty("index", static_cast<int>(i));
			connect(btn, &QPushButton::clicked, this, &WonderSelectionWidget::handleButtonClick);

			btn->setAttribute(Qt::WA_Hover, true);
			btn->installEventFilter(this);
		}

		m_cardsLayout->addWidget(btn);
		m_buttons.push_back(btn);
	}
}

void WonderSelectionWidget::setOnWonderClicked(std::function<void(int index)> callback)
{
	m_callback = callback;
}

void WonderSelectionWidget::handleButtonClick()
{
	QPushButton* senderBtn = qobject_cast<QPushButton*>(sender());
	if (senderBtn && m_callback) {
		int idx = senderBtn->property("index").toInt();
		m_callback(idx);
	}
}


bool WonderSelectionWidget::eventFilter(QObject* watched, QEvent* event)
{
	auto* btn = qobject_cast<QPushButton*>(watched);
	if (!btn) return QWidget::eventFilter(watched, event);

	switch (event->type()) 
	{
	case QEvent::Enter:
	case QEvent::HoverMove:
	{
		int idx = btn->property("index").toInt();
		Models::Wonder* w = (idx >= 0 && idx < static_cast<int>(m_wonders.size())) ? m_wonders[idx] : nullptr;
		QString text = buildWonderText(w);
		if (!text.isEmpty()) {
			QToolTip::showText(QCursor::pos(), text, btn);
		}
		return true;
	}
	case QEvent::Leave:
		QToolTip::hideText();
		return true;
	default:
		break;
	}
	return QWidget::eventFilter(watched, event);
}
