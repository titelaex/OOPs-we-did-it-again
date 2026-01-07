#pragma once
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout> // <--- Adăugat
#include <QLabel>      // <--- Adăugat
#include <vector>
#include <memory>
#include <functional>

// Importuri
import Models.Wonder; 
import Core.Player;

class WonderSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WonderSelectionWidget(QWidget* parent = nullptr);

    void loadWonders(std::vector<Models::Wonder*> wonders);
    void setOnWonderClicked(std::function<void(int index)> callback);

    void setTurnMessage(QString message);

private:
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_cardsLayout; 
    QLabel* m_infoLabel;        

    std::vector<QPushButton*> m_buttons;
    std::function<void(int index)> m_callback;

private slots:
    void handleButtonClick();
};