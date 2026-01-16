#pragma once
#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <memory>

namespace Core { class IGameListener; }

class DiscardedCardsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DiscardedCardsWidget(QWidget* parent = nullptr);
    ~DiscardedCardsWidget();

    void addDiscardedCard(const QString& cardName, const QString& cardColor);

    std::shared_ptr<Core::IGameListener> getListener();

private:
    void setupUi();

    QListWidget* m_cardsList{ nullptr };
    QLabel* m_titleLabel{ nullptr };
    std::shared_ptr<Core::IGameListener> m_listener;
};