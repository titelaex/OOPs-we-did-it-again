#pragma once

#include <QStyledItemDelegate>

class CardSpineDelegate final : public QStyledItemDelegate
{
public:
 explicit CardSpineDelegate(QObject* parent = nullptr);

 QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
 void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};
