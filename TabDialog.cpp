
#define _CRT_SECURE_NO_WARNINGS

#include "TabDialog.h"
#include "QtTermTCP.h"
#include <QTcpServer>
#include "QSettings"
#include "QLineEdit"
#include "QTabWidget"
#include "QDialogButtonBox"
#include "QVBoxLayout"
#include "QLabel"
#include "QAction"
#include "QGroupBox"
#include "QPlainTextEdit"
#include "QCheckBox"
#include "QFormLayout"
#include "QScrollArea"

#include "ax25.h"

#ifndef WIN32
#define strtok_s strtok_r
#endif

extern "C" void SaveSettings();

extern int screenHeight;
extern int screenWidth;

extern QList<Ui_ListenSession *> _sessions;
extern QTcpServer * _server;

extern myTcpSocket * AGWSock;
extern myTcpSocket * KISSSock;
extern QLabel * Status1;
extern QFont * menufont;
extern QStatusBar * myStatusBar;

QLineEdit *hostEdit;
QLineEdit *portEdit;
QLineEdit *userEdit;
QLineEdit *passEdit;

extern QAction *actHost[17];
extern QAction *actSetup[16];

extern int ConfigHost;

#define MAXHOSTS 16

extern char Host[MAXHOSTS + 1][100];
extern int Port[MAXHOSTS + 1];
extern char UserName[MAXHOSTS + 1][80];
extern char Password[MAXHOSTS + 1][80];
extern char MYCALL[32];

QLineEdit *TermCall;
QGroupBox *groupBox;
QLineEdit *beaconDest;
QLineEdit *beaconPath;
QLineEdit *beaconInterval;
QLineEdit *beaconPorts;
QLabel *label_2;
QLabel *label_3;
QLabel *label_4;
QLabel *label_5;
QLabel *label_6;
QLabel *label_7;
QLabel *label_11;
QPlainTextEdit *beaconText;
QLabel *label_12;
QGroupBox *groupBox_2;
QLineEdit *AHost;
QLineEdit *APort;
QLineEdit *APath;
QLineEdit *Paclen;
QLabel *label_8;
QLabel *label_9;
QLabel *label_10;
QLabel *label;
QCheckBox *AAGWEnable;
QLabel *label_13;
QCheckBox *AAGWMonEnable;

QLineEdit *AVARATermCall;
QLineEdit *AVARAHost;
QLineEdit *AVARAPort;
QCheckBox *AVARAEnable;


extern int AGWEnable;
extern int VARAEnable;
extern int KISSEnable;
extern int AGWMonEnable;
extern char AGWTermCall[12];
extern char AGWBeaconDest[12];
extern char AGWBeaconPath[80];
extern int AGWBeaconInterval;
extern char AGWBeaconPorts[80];
extern char AGWBeaconMsg[260];

extern char VARATermCall[12];

extern char AGWHost[128];
extern int AGWPortNum;
extern int AGWPaclen;
extern char * AGWPortList;
extern myTcpSocket * AGWSock; 
extern char * AGWPortList;
extern QStringList AGWToCalls;

extern int KISSMode;

extern Ui_ListenSession * ActiveSession;

extern int TermMode;

#define Single 0
#define MDI 1
#define Tabbed 2

extern int listenPort;
extern "C" int listenEnable;
extern char listenCText[4096];

void Send_AGW_C_Frame(Ui_ListenSession * Sess, int Port, char * CallFrom, char * CallTo, char * Data, int DataLen);
extern "C" void SetSessLabel(Ui_ListenSession * Sess, char * label);
extern "C" void SendtoTerm(Ui_ListenSession * Sess, char * Msg, int Len);

QString GetConfPath();

QScrollArea *scrollArea;
QWidget *scrollAreaWidgetContents;

bool myResize::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::Resize)
	{
		QResizeEvent *resizeEvent = static_cast<QResizeEvent *>(event);
		QSize size = resizeEvent->size();
		int h = size.height();
		int w = size.width();

		scrollArea->setGeometry(QRect(5, 5, w - 10, h - 10));
		return true;
	}
	return QObject::eventFilter(obj, event);
}

AGWConnect::AGWConnect(QWidget *parent) : QDialog(parent)
{
	this->setFont(*menufont);

	setWindowTitle(tr("AGW Connection"));

	myResize *resize = new myResize();
	installEventFilter(resize);

	scrollArea = new QScrollArea(this);
	scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
	scrollArea->setGeometry(QRect(5, 5, 562, 681));
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setWidgetResizable(false);
	scrollAreaWidgetContents = new QWidget();
	scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));

	QVBoxLayout *layout = new QVBoxLayout();
	layout->setContentsMargins(10, 10, 10, 10);

	setLayout(layout);

	QFormLayout *formLayout2 = new QFormLayout();
	layout->addLayout(formLayout2);

	wCallFrom = new QLineEdit();
	formLayout2->addRow(new QLabel("Call From"), wCallFrom);

	wCallTo = new QComboBox();
	wCallTo->setEditable(true);
	wCallTo->setInsertPolicy(QComboBox::NoInsert);

	formLayout2->addRow(new QLabel("Call To"), wCallTo);

	Digis = new QLineEdit();
	formLayout2->addRow(new QLabel("Digis"), Digis);

	layout->addSpacing(2);
	layout->addWidget(new QLabel("Radio Ports"));

	RadioPorts = new QListWidget();

	layout->addWidget(RadioPorts);

	QString str;
	int count;

	char * Context;
	char * ptr;

	wCallFrom->setText(AGWTermCall);

	wCallTo->addItems(AGWToCalls);

	if (AGWPortList)
	{
		char * Temp = strdup(AGWPortList);		// Need copy as strtok messes with it

		count = atoi(Temp);

		ptr = strtok_s(Temp, ";", &Context);

		for (int n = 0; n < count; n++)
		{
			ptr = strtok_s(NULL, ";", &Context);
			new QListWidgetItem(ptr, RadioPorts);
		}

		free(Temp);

		// calculate scrollarea height from count

		scrollAreaWidgetContents->setGeometry(QRect(0, 0, 400, 220 + 22 * count));
		this->resize(420, 240 + 22 * count);

	}

	RadioPorts->setFont(*menufont);

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	buttonBox->setFont(*menufont);
	layout->addWidget(buttonBox);

	scrollAreaWidgetContents->setLayout(layout);
	scrollArea->setWidget(scrollAreaWidgetContents);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(myaccept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(myreject()));
}

AGWConnect::~AGWConnect()
{
}

extern QAction *discAction;

void AGWConnect::myaccept()
{
	QVariant Q;

	int port = RadioPorts->currentRow();
	char CallFrom[32];
	char CallTo[32];
	char Via[128];
	char DigiMsg[128] = "";
	int DigiLen = 0;
	char * digiptr = &DigiMsg[1];

	strcpy(CallFrom, wCallFrom->text().toUpper().toUtf8());
	strcpy(CallTo, wCallTo->currentText().toUpper().toUtf8());
	strcpy(Via, Digis->text().toUpper().toUtf8());

	// if digis have to form block with byte count followed by n 10 byte calls

	if (Via[0])
	{
		char * context;
		char * ptr = strtok_s(Via, ", ", &context);

		while (ptr)
		{
			DigiMsg[0]++;
			strcpy(digiptr, ptr);
			digiptr += 10;

			ptr = strtok_s(NULL, ", ", &context);
		}
		DigiLen = digiptr - DigiMsg;
	}

	// Add CallTo if not already in list

	if (AGWToCalls.contains(CallTo))
		AGWToCalls.removeOne(CallTo);

	AGWToCalls.insert(-1, CallTo);

	if (port == -1)
	{
		// Port not set. If connecting to SWITCH use any, otherwise tell user

		if (strcmp(CallTo, "SWITCH") == 0)
		{
			port = 0;
		}
		else
		{
			QMessageBox msgBox;
			msgBox.setText("Select a port to call on");
			msgBox.exec();
			return;
		}
	}

	Send_AGW_C_Frame(ActiveSession, port, CallFrom, CallTo, DigiMsg, DigiLen);

	discAction->setEnabled(true);


	AGWConnect::accept();
}

void AGWConnect::myreject()
{
	AGWConnect::reject();
}


AGWDialog::AGWDialog(QWidget *parent) : QDialog(parent)
{
	this->setFont(*menufont);

	setWindowTitle(tr("TermTCP AGW Configuration"));

	myResize *resize = new myResize();
	installEventFilter(resize);


	scrollArea = new QScrollArea(this);
	scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
	scrollArea->setGeometry(QRect(5, 5, 562, 681));
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setWidgetResizable(false);
	scrollAreaWidgetContents = new QWidget();
	scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
	scrollAreaWidgetContents->setGeometry(QRect(0, 0, 552, 581));

	this->resize(572, 601);

	QVBoxLayout *layout = new QVBoxLayout;
	QHBoxLayout *hlayout = new QHBoxLayout;

	layout->addLayout(hlayout);
	AAGWEnable = new QCheckBox("Enable AGW Interface");
	AAGWEnable->setLayoutDirection(Qt::LeftToRight);

	AAGWMonEnable = new QCheckBox("Enable Monitor");
	AAGWMonEnable->setGeometry(QRect(255, 18, 216, 21));
	AAGWMonEnable->setLayoutDirection(Qt::RightToLeft);

	hlayout->addWidget(AAGWEnable);
	hlayout->addWidget(AAGWMonEnable);

	QFormLayout *flayout = new QFormLayout;
	layout->addLayout(flayout);

	label = new QLabel("Terminal Call");
	TermCall = new QLineEdit(this);

	flayout->addRow(label, TermCall);
	
	layout->addWidget(new QLabel("Beacon Setup"));

	QFormLayout *flayout1 = new QFormLayout;
	layout->addLayout(flayout1);

	label_2 = new QLabel("Destination");
	beaconDest = new QLineEdit();
	label_3 = new QLabel("Digipeaters");
	beaconPath = new QLineEdit();

	flayout1->addRow(label_2, beaconDest);
	flayout1->addRow(label_3, beaconPath);

	label_4 = new QLabel("Interval");
	beaconInterval = new QLineEdit();
	label_5 = new QLabel("Ports");
	beaconPorts = new QLineEdit();

	flayout1->addRow(label_4, beaconInterval);
	flayout1->addRow(label_5, beaconPorts);

//	label_6 = new QLabel("Minutes", groupBox);

//	label_7 = new QLabel("(Separate with commas)", groupBox);
	label_11 = new QLabel("Message");
	beaconText = new QPlainTextEdit();

	flayout1->addRow(label_11, beaconText);



//	label_12 = new QLabel("(max 256 chars)");
//	label_12->setGeometry(QRect(14, 158, 95, 21));

	layout->addWidget(new QLabel("TNC Setup"));

	QFormLayout *flayout2 = new QFormLayout;
	layout->addLayout(flayout2);

	AHost = new QLineEdit();
	APort = new QLineEdit();
	Paclen = new QLineEdit();
	label_8 = new QLabel("host");
	label_9 = new QLabel("Port");
	label_10 = new QLabel("Paclen  ");

	flayout2->addRow(label_8, AHost);
	flayout2->addRow(label_9, APort);
	flayout2->addRow(label_10, Paclen);

	AAGWEnable->setChecked(AGWEnable);
	AAGWMonEnable->setChecked(AGWMonEnable);
	TermCall->setText(AGWTermCall);
	beaconDest->setText(AGWBeaconDest);
	beaconPath->setText(AGWBeaconPath);
	beaconPorts->setText(AGWBeaconPorts);
	beaconText->setPlainText(AGWBeaconMsg);
	beaconInterval->setText(QString::number(AGWBeaconInterval));
	AHost->setText(AGWHost);
	APort->setText(QString::number(AGWPortNum));
	Paclen->setText(QString::number(AGWPaclen));

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
	buttonBox->setFont(*menufont);
	layout->addWidget(buttonBox);
	scrollAreaWidgetContents->setLayout(layout);
	scrollArea->setWidget(scrollAreaWidgetContents);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(myaccept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(myreject()));

}


AGWDialog::~AGWDialog()
{
}


KISSConnect::KISSConnect(QWidget *parent) : QDialog(parent)
{
	this->setFont(*menufont);

	setWindowTitle(tr("KISS Connection"));

	myResize *resize = new myResize();
	installEventFilter(resize);

	scrollArea = new QScrollArea(this);
	scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
	scrollArea->setGeometry(QRect(5, 5, 250, 200));
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setWidgetResizable(false);
	scrollAreaWidgetContents = new QWidget();
	scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));

	// layout is top level.
	// Add a Horizontal layout for Mode and Verical Layout for Call and Digis

	QVBoxLayout *layout = new QVBoxLayout();
	layout->setContentsMargins(10, 10, 10, 10);
	setLayout(layout);

	QHBoxLayout *mylayout = new QHBoxLayout();

	Connected = new QRadioButton("Sesion");
	UIMode = new QRadioButton("UI");
	
	mylayout->addWidget(new QLabel("Connection Mode"));
	mylayout->addWidget(Connected);
	mylayout->addWidget(UIMode);

	if (KISSMode)			// UI = 1
		UIMode->setChecked(1);
	else
		Connected->setChecked(1);

	QFormLayout *formLayout2 = new QFormLayout();
	layout->addLayout(mylayout);
	layout->addSpacing(10);
	layout->addLayout(formLayout2);

	wCallTo = new QComboBox();
	wCallTo->setEditable(true);
	wCallTo->setInsertPolicy(QComboBox::NoInsert);

	formLayout2->addRow(new QLabel("Call To"), wCallTo);

	Digis = new QLineEdit();
	formLayout2->addRow(new QLabel("Digis"), Digis);

	layout->addSpacing(2);

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	buttonBox->setFont(*menufont);
	layout->addWidget(buttonBox);

	scrollAreaWidgetContents->setLayout(layout);
	scrollArea->setWidget(scrollAreaWidgetContents);

	wCallTo->addItems(AGWToCalls);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(myaccept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(myreject()));
}

extern "C" void * KISSConnectOut(void * Sess, char * CallFrom, char * CallTo, char * Digis, int Chan, void * Socket);

KISSConnect::~KISSConnect()
{
}

extern QAction * YAPPSend;
extern QMenu * connectMenu;
extern QMenu * disconnectMenu;

TAX25Port DummyPort;
extern "C" TAX25Port * get_user_port_by_calls(int snd_ch, char *  CallFrom, char *  CallTo);

void KISSConnect::myaccept()
{
	QVariant Q;

	char CallTo[32];
	char Via[128];
	strcpy(CallTo, wCallTo->currentText().toUpper().toUtf8());
	strcpy(Via, Digis->text().toUpper().toUtf8());

	TAX25Port * AX25Sess = 0;

	// Check for duplicate session

	AX25Sess = get_user_port_by_calls(0, MYCALL, CallTo);

	if (AX25Sess)
	{
		// Duplicate

		char Msg[256];

		int Len = sprintf(Msg, "You already have a session between %s and %s so can't connect\r", MYCALL, CallTo);

		WritetoOutputWindow(ActiveSession, (unsigned char *)Msg, Len);
//		KISSConnect::accept();
		return;
	}

	KISSMode = UIMode->isChecked();

	// Add CallTo if not already in list

	if (AGWToCalls.contains(CallTo))
		AGWToCalls.removeOne(CallTo);

	AGWToCalls.insert(-1, CallTo);

	ActiveSession->KISSMode = KISSMode;

	if (KISSMode == 0)
	{
		ActiveSession->KISSSession = KISSConnectOut(ActiveSession, MYCALL, CallTo, Via, 0, (void *)KISSSock);
		WritetoOutputWindow(ActiveSession, (unsigned char *)"Connecting...\r", 14);
		discAction->setEnabled(true);
	}
	else
	{
		// UI

		char Msg[128];
		int Len = 0;

		memset(&DummyPort, 0, sizeof(DummyPort));

		ActiveSession->KISSSession = (void *)&DummyPort;		// Dummy marker to show session in use

		strcpy(ActiveSession->UIDEST, CallTo);
		strcpy(ActiveSession->UIPATH, Via);

		if (TermMode == Tabbed)
			Len = sprintf(Msg, "UI %s", CallTo);
		else
			Len = sprintf(Msg, "UI Session with %s\r", CallTo);

		SetSessLabel(ActiveSession, Msg);

		Len = sprintf(Msg, "UI Session with %s\r", CallTo);
		SendtoTerm(ActiveSession, Msg, Len);
		connectMenu->setEnabled(false);
		discAction->setEnabled(true);
		YAPPSend->setEnabled(false);
	}

	KISSConnect::accept();
}

void KISSConnect::myreject()
{
	KISSConnect::reject();
}



VARAConnect::VARAConnect(QWidget *parent) : QDialog(parent)
{
	this->setFont(*menufont);

	setWindowTitle(tr("VARA Connection"));

	myResize *resize = new myResize();
	installEventFilter(resize);

	scrollArea = new QScrollArea(this);
	scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
	scrollArea->setGeometry(QRect(5, 5, 250, 150));
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setWidgetResizable(false);
	scrollAreaWidgetContents = new QWidget();
	scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));

	QVBoxLayout *layout = new QVBoxLayout();
	layout->setContentsMargins(10, 10, 10, 10);

	setLayout(layout);

	QFormLayout *formLayout2 = new QFormLayout();
	layout->addLayout(formLayout2);

	wCallFrom = new QLineEdit();
	formLayout2->addRow(new QLabel("Call From"), wCallFrom);

	wCallTo = new QComboBox();
	wCallTo->setEditable(true);
	wCallTo->setInsertPolicy(QComboBox::NoInsert);

	formLayout2->addRow(new QLabel("Call To"), wCallTo);

	Digis = new QLineEdit();
	formLayout2->addRow(new QLabel("Digis"), Digis);

	layout->addSpacing(2);

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	buttonBox->setFont(*menufont);
	layout->addWidget(buttonBox);

	scrollAreaWidgetContents->setLayout(layout);
	scrollArea->setWidget(scrollAreaWidgetContents);

	wCallFrom->setText(VARATermCall);
	wCallTo->addItems(AGWToCalls);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(myaccept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(myreject()));
}

VARAConnect::~VARAConnect()
{
}

extern myTcpSocket * VARASock;
extern myTcpSocket * VARADataSock;

void VARAConnect::myaccept()
{
	QVariant Q;

	char CallFrom[32];
	char CallTo[32];
	char Via[128];
	char Msg[256];
	int Len;

	strcpy(CallFrom, wCallFrom->text().toUpper().toUtf8());
	strcpy(CallTo, wCallTo->currentText().toUpper().toUtf8());
	strcpy(Via, Digis->text().toUpper().toUtf8());

// if digis have to form block with byte count followed by n 10 byte calls


// Add CallTo if not already in list

	if (AGWToCalls.contains(CallTo))
		AGWToCalls.removeOne(CallTo);

	AGWToCalls.insert(-1, CallTo);


	if (Via[0])
		Len = sprintf(Msg, "CONNECT %s %s VIA %s\r", CallFrom, CallTo, Via);
	else 
		Len = sprintf(Msg, "CONNECT %s %s\r", CallFrom, CallTo);

	VARASock->write(Msg, Len);

	discAction->setEnabled(true);
	VARAConnect::accept();
}

void VARAConnect::myreject()
{
	VARAConnect::reject();
}


extern QProcess *process;



TabDialog::TabDialog(QWidget *parent) : QDialog(parent)
{
	char portnum[10];

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(myaccept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(myreject()));

	QVBoxLayout *layout = new QVBoxLayout;
	QLabel *hostLabel = new QLabel(tr("Host Name:"));
	hostEdit = new QLineEdit(Host[ConfigHost]);

	QLabel *portLabel = new QLabel(tr("Port:"));
	sprintf(portnum, "%d", Port[ConfigHost]);
	portEdit = new QLineEdit(portnum);

	QLabel *userLabel = new QLabel(tr("User:"));
	userEdit = new QLineEdit(UserName[ConfigHost]);

	QLabel *passLabel = new QLabel(tr("Password:"));
	passEdit = new QLineEdit(Password[ConfigHost]);

	layout->addWidget(hostLabel);
	layout->addWidget(hostEdit);
	layout->addWidget(portLabel);
	layout->addWidget(portEdit);
	layout->addWidget(userLabel);
	layout->addWidget(userEdit);
	layout->addWidget(passLabel);
	layout->addWidget(passEdit);

	layout->addStretch(1);
	layout->addWidget(buttonBox);
	setLayout(layout);

	setWindowTitle(tr("TermTCP Host Configuration"));
}

void AGWDialog::myaccept()
{
	QVariant Q;

	int OldEnable = AGWEnable;
	int OldPort = AGWPortNum;
	char oldHost[128];
	strcpy(oldHost, AGWHost);

	//	QString val = Sess->portNo->text();A
	//	QByteArray qb = val.toLatin1();
	//	char * ptr = qb.data();

	AGWEnable = AAGWEnable->isChecked();
	AGWMonEnable = AAGWMonEnable->isChecked();

	strcpy(AGWTermCall, TermCall->text().toUtf8().toUpper());
	strcpy(AGWBeaconDest, beaconDest->text().toUtf8().toUpper());
	strcpy(AGWBeaconPath, beaconPath->text().toUtf8().toUpper());
	strcpy(AGWBeaconPorts, beaconPorts->text().toUtf8().toUpper());

	if (beaconText->toPlainText().length() > 256)
	{
		QMessageBox msgBox;
		msgBox.setText("Beacon Text Too Long");
		msgBox.exec();
	}
	else
		strcpy(AGWBeaconMsg, beaconText->toPlainText().toUtf8().toUpper());

	Q = beaconInterval->text();
	AGWBeaconInterval = Q.toInt();

	strcpy(AGWHost, AHost->text().toUtf8());

	Q = APort->text();
	AGWPortNum = Q.toInt();

	Q = Paclen->text();
	AGWPaclen = Q.toInt();
		
	SaveSettings();

	if (AGWEnable != OldEnable || AGWPortNum != OldPort || strcmp(oldHost, AGWHost) != 0)
	{
		// (re)start connection

		if (OldEnable && AGWSock && AGWSock->ConnectedState == QAbstractSocket::ConnectedState)
		{
			AGWSock->disconnectFromHost();
			Status1->setText("AGW Disconnected");
		}
		// AGWTimer will reopen connection
	}

	myStatusBar->setVisible(AGWEnable | VARAEnable | KISSEnable);

	AGWDialog::accept();

}

void AGWDialog::myreject()
{
	AGWDialog::reject();
}

void TabDialog::myaccept()
{
	QString val = hostEdit->text();
	QByteArray qb = val.toLatin1();
	char * ptr = qb.data();
	strcpy(Host[ConfigHost], ptr);
		
	val = portEdit->text();
	qb = val.toLatin1();
	ptr = qb.data();
	Port[ConfigHost] = atoi(ptr);

	val = userEdit->text();
	qb = val.toLatin1();
	ptr = qb.data();
	strcpy(UserName[ConfigHost], ptr);

	val = passEdit->text();
	qb = val.toLatin1();
	ptr = qb.data();
	strcpy(Password[ConfigHost], ptr);

	actHost[ConfigHost]->setText(Host[ConfigHost]);
	actSetup[ConfigHost]->setText(Host[ConfigHost]);

	SaveSettings();


	TabDialog::accept();

}

void TabDialog::myreject()
{
	TabDialog::reject();
}

TabDialog::~TabDialog()
{
}

// Menu dialog


fontDialog::fontDialog(int Menu, QWidget *parent) : QDialog(parent)
{
	// Menu is set if setting Menufont, zero for setting terminal font.

	int i;
	char valChar[16];

	QString family;
	int csize;
    QFont::Weight weight;

#ifdef ANDROID
	this->resize((screenWidth * 7) / 8, 200);
	this->setMaximumWidth((screenWidth * 7) / 8);
#endif
	this->setFont(*menufont);

	Menuflag = Menu;

	if (Menu)
	{
		workingFont = *menufont;

		QFontInfo info(*menufont);
		family = info.family();
		csize = info.pointSize();

		setWindowTitle("Menu Font Dialog");
	}
	else
	{
		// get current term font

		QSettings settings(GetConfPath(), QSettings::IniFormat);

#ifdef ANDROID
		family = settings.value("FontFamily", "Driod Sans Mono").toString();
		csize = settings.value("PointSize", 12).toInt();
        weight = (QFont::Weight)settings.value("Weight", 50).toInt();
#else
		family = settings.value("FontFamily", "Courier New").toString();
		csize = settings.value("PointSize", 10).toInt();
		weight = (QFont::Weight)settings.value("Weight", 50).toInt();
#endif

		workingFont = QFont(family);
		workingFont.setPointSize(csize);
		workingFont.setWeight(weight);

		setWindowTitle("Terminal Font Dialog");
	}

	QVBoxLayout *layout = new QVBoxLayout();
	layout->setContentsMargins(10, 10, 10, 10);

	setLayout(layout);

	QHBoxLayout *hlayout = new QHBoxLayout();
	layout->addLayout(hlayout);

	font = new QFontComboBox();

	if (Menu == 0)
		font->setFontFilters(QFontComboBox::MonospacedFonts);

	font->setMaximumWidth((screenWidth * 5) / 8);
	font->view()->setMaximumWidth((7 * screenWidth) / 8);

	style = new QComboBox();
	style->setMaximumWidth(screenWidth / 4);
	size = new QComboBox();
	sample = new QTextEdit();
	sample->setText("ABCDabcd1234");
	sample->setFont(workingFont);

	hlayout->addWidget(font);
	hlayout->addWidget(style);
	hlayout->addWidget(size);
	layout->addWidget(sample);

	QFontDatabase database;

	const QStringList styles = database.styles(family);

	const QList<int> smoothSizes = database.smoothSizes(family, styles[0]);

	for (int points : smoothSizes)
		size->addItem(QString::number(points));

	for (QString xstyle : styles)
		style->addItem(xstyle);

	i = font->findText(family, Qt::MatchExactly);
	font->setCurrentIndex(i);

	sprintf(valChar, "%d", csize);
	i = size->findText(valChar, Qt::MatchExactly);
	size->setCurrentIndex(i);

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	buttonBox->setFont(*menufont);
	layout->addWidget(buttonBox);
	setLayout(layout);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(myaccept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(myreject()));
	connect(font, SIGNAL(currentFontChanged(QFont)), this, SLOT(fontchanged(QFont)));
	connect(style, SIGNAL(currentIndexChanged(int)), this, SLOT(stylechanged()));
	connect(size, SIGNAL(currentIndexChanged(int)), this, SLOT(sizechanged()));
}

void fontDialog::fontchanged(QFont newfont)
{
	QFontDatabase database;
	QString family = newfont.family();

	workingFont = newfont;

	const QStringList styles = database.styles(family);
	const QList<int> smoothSizes = database.smoothSizes(family, styles[0]);

	size->clear();
	style->clear();

	for (int points : smoothSizes)
		size->addItem(QString::number(points));

	for (QString xstyle : styles)
		style->addItem(xstyle);

	sample->setFont(workingFont);
}

void fontDialog::stylechanged()
{
	QFontDatabase database;

	QString family = font->currentFont().family();

	bool italic = database.italic(family, style->currentText());
	int weight = database.weight(family, style->currentText());

	if (weight < 0)
		weight = 50;				// Normal

	workingFont.setItalic(italic);
    workingFont.setWeight((QFont::Weight) weight);

	sample->setFont(workingFont);
}

void fontDialog::sizechanged()
{
	int newsize = size->currentText().toInt();
	workingFont.setPointSize(newsize);
	sample->setFont(workingFont);
}
fontDialog::~fontDialog()
{
}

void fontDialog::myaccept()
{
	QSettings settings(GetConfPath(), QSettings::IniFormat);

	if (Menuflag)
	{
		delete menufont;
		menufont = new QFont(workingFont);

		QtTermTCP::setFonts();

		settings.setValue("MFontFamily", workingFont.family());
		settings.setValue("MPointSize", workingFont.pointSize());
		settings.setValue("MWeight", workingFont.weight());
	}
	else
	{
		Ui_ListenSession * Sess;

		for (int i = 0; i < _sessions.size(); ++i)
		{
			Sess = _sessions.at(i);

			if (Sess->termWindow)
				Sess->termWindow->setFont(workingFont);

			if (Sess->inputWindow)
				Sess->inputWindow->setFont(workingFont);

			if (Sess->monWindow)
				Sess->monWindow->setFont(workingFont);
		}

		settings.setValue("FontFamily", workingFont.family());
		settings.setValue("PointSize", workingFont.pointSize());
		settings.setValue("Weight", workingFont.weight());
	}

	fontDialog::accept();
}

void fontDialog::myreject()
{
	fontDialog::reject();
}



ListenDialog::ListenDialog(QWidget *parent) : QDialog(parent)
{
#ifdef ANDROID
	this->resize((screenWidth * 3) / 4 , 500);
#endif
	verticalLayout = new QVBoxLayout();
	verticalLayout->setContentsMargins(10, 10, 10, 10);

	setLayout(verticalLayout);

	Enabled = new QCheckBox();
	Enabled->setText(QString::fromUtf8("Enable Listen"));
	Enabled->setLayoutDirection(Qt::LeftToRight);

	verticalLayout->addWidget(Enabled);

	formLayout = new QFormLayout();

	portNo = new QLineEdit();
//	portNo->setMaximumSize(QSize(100, 30));

	formLayout->addRow(new QLabel("Port"), portNo);

	CText = new QTextEdit();
	CText->setMinimumSize(QSize(0, 150));
	CText->setMaximumSize(QSize(401, 150));

	formLayout->addRow(new QLabel("CText"), CText);
	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	buttonBox->setFont(*menufont);
	verticalLayout->addWidget(buttonBox);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(myaccept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(myreject()));

	verticalLayout->addLayout(formLayout);
	verticalLayout->addWidget(buttonBox);

	portNo->setText(QString::number(listenPort));
	Enabled->setChecked(listenEnable);
	CText->setText(listenCText);

}

ListenDialog::~ListenDialog()
{
}

void ListenDialog::myaccept()
{
	QString val = portNo->text();
	QByteArray qb = val.toLatin1();
	char * ptr = qb.data();

	listenPort = atoi(ptr);
	listenEnable = Enabled->isChecked();
	strcpy(listenCText, CText->toPlainText().toUtf8());

	while ((ptr = strchr(listenCText, '\n')))
		*ptr = '\r';

	if (_server->isListening())
		_server->close();

	SaveSettings();

	if (listenEnable)
		_server->listen(QHostAddress::Any, listenPort);

	ListenDialog::accept();
}

void ListenDialog::myreject()
{
	ListenDialog::reject();
}
