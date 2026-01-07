#pragma once
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <vector>
#include <memory>

import Models.Wonder; 
import Core.Player;

class WonderSelectionWidget : public QWidget
{
    Q_OBJECT 

public:
    explicit WonderSelectionWidget(QWidget* parent = nullptr);

    void loadWonders(std::vector<Models::Wonder*> wonders);

    void setOnWonderClicked(std::function<void(int index)> callback);

private:
    QHBoxLayout* m_layout;
    std::vector<QPushButton*> m_buttons; 
    std::function<void(int index)> m_callback; 

private slots:
    void handleButtonClick(); 
};