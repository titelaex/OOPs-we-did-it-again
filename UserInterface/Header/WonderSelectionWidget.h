#pragma once
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout> 
#include <QLabel>      
#include <vector>
#include <memory>
#include <functional>

import Models.Wonder; 
import Core.Player;

class WonderSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WonderSelectionWidget(QWidget* parent = nullptr);

    void loadWonders(std::vector<Models::Wonder*> wonders);
    void setOnWonderClicked(std::function<void(int index)> callback);
    bool eventFilter(QObject* watched, QEvent* event);
    void setTurnMessage(QString message);

private:
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_cardsLayout; 
    QLabel* m_infoLabel;       
    std::vector<Models::Wonder*> m_wonders;

    std::vector<QPushButton*> m_buttons;
    std::function<void(int index)> m_callback;

private slots:
    void handleButtonClick();
};