
#ifndef TABDIALOG_H
#define TABDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QListWidget>
#include <QFontComboBox>
#include <QTextEdit>
#include <QCheckBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QRadioButton>

class QDialogButtonBox;
class QFileInfo;
class QTabWidget;

namespace Ui {
class TabDialog;
}

class ListenDialog: public QDialog
{
	Q_OBJECT

public:
	explicit ListenDialog(QWidget *parent = 0);
	~ListenDialog();

private slots:
	void myaccept();
	void myreject();

public:
	QVBoxLayout *verticalLayout;
	QFormLayout *formLayout;
	QLineEdit *portNo;
	QTextEdit *CText;
	QVBoxLayout *verticalLayout_2;
	QDialogButtonBox *buttonBox;
	QCheckBox *Enabled;


};



class TabDialog : public QDialog
{
	Q_OBJECT

public:
	explicit TabDialog(QWidget *parent = 0);
	~TabDialog();

private slots:
	void myaccept();
	void myreject();

private:
	//   Ui::TabDialog *ui;
	QDialogButtonBox *buttonBox;
};

class AGWDialog : public QDialog
{
	Q_OBJECT

public:
	explicit AGWDialog(QWidget *parent = 0);
	~AGWDialog();

public:
	QPushButton *okButton;
	QPushButton *cancelButton;

private slots:
	void myaccept();
	void myreject();

private:
	//   Ui::TabDialog *ui;
	QDialogButtonBox *buttonBox;
};

class AGWConnect : public QDialog
{
	Q_OBJECT

public:
	explicit AGWConnect(QWidget *parent = 0);
	~AGWConnect();

public:
	QLineEdit * wCallFrom;
	QComboBox * wCallTo;
	QLineEdit * Digis;
	QListWidget * RadioPorts;

private slots:
	void myaccept();
	void myreject();

private:
	//   Ui::TabDialog *ui;
	QDialogButtonBox *buttonBox;
};



class VARAConnect : public QDialog
{
	Q_OBJECT

public:
	explicit VARAConnect(QWidget *parent = 0);
	~VARAConnect();

public:
	QLineEdit * wCallFrom;
	QComboBox * wCallTo;
	QLineEdit * Digis;
	QListWidget * RadioPorts;

private slots:
	void myaccept();
	void myreject();

private:
	//   Ui::TabDialog *ui;
	QDialogButtonBox *buttonBox;
};


class KISSConnect : public QDialog
{
	Q_OBJECT

public:
	explicit KISSConnect(QWidget *parent = 0);
	~KISSConnect();

public:
	QLineEdit * wCallFrom;
	QComboBox * wCallTo;
	QComboBox * Chan;
	QLineEdit * Digis;
	QLineEdit * UIDest;
	QListWidget * RadioPorts;
	QHBoxLayout *mylayout;
	QRadioButton * Connected;
	QRadioButton * UIMode;

private slots:
	void myaccept();
	void myreject();

private:
	//   Ui::TabDialog *ui;
	QDialogButtonBox *buttonBox;
};



class fontDialog : public QDialog
{
	Q_OBJECT

public:
	explicit fontDialog(int Menu, QWidget *parent = 0);
	~fontDialog();

public:
	QFontComboBox *font;
	QComboBox *style;
	QComboBox *size;
	QTextEdit * sample;
	QFont workingFont;
	int workingSize;
	int Menuflag;			// Set if menu font

private slots:
	void myaccept();
	void myreject();
	void fontchanged(QFont);
	void stylechanged();
	void sizechanged();

private:
	
	QDialogButtonBox *buttonBox;
};

#endif

class myResize : public QObject
{
	Q_OBJECT

protected:
	bool eventFilter(QObject *obj, QEvent *event) override;
};




