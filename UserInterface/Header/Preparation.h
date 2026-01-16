#pragma once

#include <QtWidgets/QDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>

class PreparationDialog : public QDialog {

	Q_OBJECT

public:
	explicit PreparationDialog(QWidget* parent = nullptr) : QDialog(parent)
	{
		setWindowTitle("Pregatire joc");
		auto* layout = new QVBoxLayout(this);

		auto* msg = new QLabel("7Wonders game a inceput, alege-ti username-ul", this);
		msg->setWordWrap(true);
		layout->addWidget(msg);

		auto* p1Row = new QHBoxLayout();
		auto* p1Lbl = new QLabel("Player1:", this);
		m_p1Edit = new QLineEdit(this);
		p1Row->addWidget(p1Lbl);
		p1Row->addWidget(m_p1Edit);
		layout->addLayout(p1Row);

		auto* p2Row = new QHBoxLayout();
		auto* p2Lbl = new QLabel("Player2:", this);
		m_p2Edit = new QLineEdit(this);
		p2Row->addWidget(p2Lbl);
		p2Row->addWidget(m_p2Edit);
		layout->addLayout(p2Row);

		auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
		connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
		connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
		layout->addWidget(buttons);
	}

	QString player1Name() const { return m_p1Edit ? m_p1Edit->text().trimmed() : QString(); }
	QString player2Name() const { return m_p2Edit ? m_p2Edit->text().trimmed() : QString(); }

private:
	QLineEdit* m_p1Edit{ nullptr };
	QLineEdit* m_p2Edit{ nullptr };
};