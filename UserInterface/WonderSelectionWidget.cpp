#include "WonderSelectionWidget.h"
#include <QDebug>

WonderSelectionWidget::WonderSelectionWidget(QWidget* parent)
    : QWidget(parent)
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setAlignment(Qt::AlignCenter);
    m_mainLayout->setSpacing(20); 

    m_infoLabel = new QLabel("Se pregateste jocul...", this);
    m_infoLabel->setAlignment(Qt::AlignCenter);
    m_infoLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: white; margin-bottom: 10px;");
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

void WonderSelectionWidget::loadWonders(std::vector<Models::Wonder*> wonders)
{
    qDeleteAll(m_buttons);
    m_buttons.clear();

    QLayoutItem* item;
    while ((item = m_cardsLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    for (size_t i = 0; i < wonders.size(); ++i) {
        auto w = wonders[i];

        QPushButton* btn = new QPushButton(this);

        btn->setFixedSize(180, 110);

        if (w) {
            btn->setText(QString::fromStdString(w->getName()));

            btn->setStyleSheet(
                "QPushButton {"
                "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4B5563, stop:1 #1F2937);"
                "  border: 2px solid #9CA3AF;"
                "  border-radius: 8px;"
                "  color: white;"
                "  font-weight: bold;"
                "  font-size: 15px;" 
                "}"
                "QPushButton:hover { border: 2px solid #F59E0B; }"
            );

            btn->setProperty("index", static_cast<int>(i));
            connect(btn, &QPushButton::clicked, this, &WonderSelectionWidget::handleButtonClick);
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