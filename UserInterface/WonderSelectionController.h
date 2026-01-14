#pragma once

#include <QObject>
#include <vector>
#include <memory>
#include <functional>

class PlayerPanelWidget;
class WonderSelectionWidget;
namespace Models { class Wonder; }
namespace Core { class Player; }

class WonderSelectionController : public QObject
{
    Q_OBJECT

public:
    explicit WonderSelectionController(
        WonderSelectionWidget* widget,
        PlayerPanelWidget* leftPanel,
        PlayerPanelWidget* rightPanel,
        QObject* parent = nullptr);

    void start();
    std::function<void()> onSelectionComplete;

private:
    void loadNextBatch();
    void onWonderSelected(int index);
    void updateTurnLabel();

    WonderSelectionWidget* m_widget{ nullptr };
    PlayerPanelWidget* m_leftPanel{ nullptr };
    PlayerPanelWidget* m_rightPanel{ nullptr };

    int m_selectionPhase{ 0 };
    int m_cardsPickedInPhase{ 0 };
    std::vector<Models::Wonder*> m_currentBatch;
};
