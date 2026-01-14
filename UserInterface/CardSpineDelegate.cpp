#include "CardSpineDelegate.h"

#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionViewItem>
#include <QAbstractItemView>
#include <QApplication>

CardSpineDelegate::CardSpineDelegate(QObject* parent)
	: QStyledItemDelegate(parent)
{
}

QSize CardSpineDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	Q_UNUSED(option);
	Q_UNUSED(index);
	// A thin vertical "book spine"
	return QSize(24,120);
}

void CardSpineDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	const QRect r = option.rect.adjusted(2,2, -2, -2);

	QColor bg = index.data(Qt::BackgroundRole).value<QColor>();
	if (!bg.isValid()) bg = QColor("#374151");

	QColor border = QColor("#111827");
	if (option.state & QStyle::State_Selected) border = QColor("#F59E0B");

	QPainterPath path;
	path.addRoundedRect(r,6,6);
	painter->fillPath(path, bg);
	painter->setPen(QPen(border,2));
	painter->drawPath(path);

	painter->setPen(QPen(QColor(255,255,255,35),2));
	painter->drawLine(r.topLeft() + QPoint(2,2), r.bottomLeft() + QPoint(2, -2));

	const QString text = index.data(Qt::DisplayRole).toString();
	if (text.isEmpty()) { painter->restore(); return; }

	painter->setClipPath(path);

	// We draw "vertical" text by rotating -90 degrees. In rotated coordinates:
	// - X grows along original height (this is where the text lays out horizontally)
	// - Y grows along original width (this is the limited dimension)
	const int padX =6; // padding along text direction (in rotated coords)
	const int padY =2; // padding perpendicular to text
	const int availableX = r.height() -2 * padX; // available width in rotated coords
	const int availableY = r.width() -2 * padY; // available height in rotated coords

	QFont f = option.font;
	f.setBold(true);
	// Make the font as large as possible while still allowing at least1 line.
	int pt = f.pointSize();
	if (pt <=0) pt =10;
	f.setPointSize(pt);

	// Helper to wrap into N lines without breaking words unless needed.
	auto wrapIntoLines = [](const QString& s, int maxWidth, const QFontMetrics& fm) {
		QString t = s;
		// normalize whitespace
		t = t.simplified();
		QStringList words = t.split(' ', Qt::SkipEmptyParts);
		QStringList lines;

		auto pushWordWrapped = [&](const QString& w) {
			// If a single word is too long, hard-wrap by characters.
			QString cur;
			for (int i =0; i < w.size(); ++i) {
				QString next = cur + w.at(i);
				if (!cur.isEmpty() && fm.horizontalAdvance(next) > maxWidth) {
					lines.push_back(cur);
					cur.clear();
				}
				cur += w.at(i);
			}
			if (!cur.isEmpty()) lines.push_back(cur);
		};

		QString current;
		for (const auto& w : words) {
			if (current.isEmpty()) {
				if (fm.horizontalAdvance(w) <= maxWidth) current = w;
				else pushWordWrapped(w);
				continue;
			}
			QString candidate = current + " " + w;
			if (fm.horizontalAdvance(candidate) <= maxWidth) {
				current = candidate;
			}
			else {
				lines.push_back(current);
				current.clear();
				if (fm.horizontalAdvance(w) <= maxWidth) current = w;
				else pushWordWrapped(w);
			}
		}
		if (!current.isEmpty()) lines.push_back(current);
		if (lines.isEmpty()) lines.push_back(QString());
		return lines;
	};

	// Find a font size that allows the wrapped lines to fit into availableY.
	QStringList bestLines;
	for (int attempt =0; attempt <20; ++attempt) {
		QFontMetrics fm(f);
		QStringList lines = wrapIntoLines(text, availableX, fm);
		int lineH = fm.height();
		int totalH = lineH * lines.size();
		if (totalH <= availableY || f.pointSize() <=6) {
			bestLines = lines;
			break;
		}
		f.setPointSize(std::max(6, f.pointSize() -1));
	}
	if (bestLines.isEmpty()) bestLines = QStringList{ text };

	painter->setFont(f);
	painter->setPen(QColor("#F9FAFB"));
	QFontMetrics fm(painter->font());
	const int lineH = fm.height();
	const int totalH = lineH * static_cast<int>(bestLines.size());

	QPoint center = r.center();
	painter->translate(center);
	painter->rotate(-90);

	// Center block of lines in rotated coordinate space
	QRect rotRect(-r.height() /2 + padX, -r.width() /2 + padY, availableX, availableY);
	int yStart = rotRect.top() + (rotRect.height() - totalH) /2;

	for (int i =0; i < bestLines.size(); ++i) {
		QRect lineRect(rotRect.left(), yStart + i * lineH, rotRect.width(), lineH);
		painter->drawText(lineRect, Qt::AlignCenter, bestLines[i]);
	}

	painter->restore();
}
