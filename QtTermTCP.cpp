// Qt Version of BPQTermTCP

#define VersionString "0.0.0.73"


// .12 Save font weight
// .13 Display incomplete lines (ie without CR)
// .14 Add YAPP and Listen Mode
// .15 Reuse windows in Listen Mode 
// .17 MDI Version 7/1/20
// .18 Fix input window losing focus when data arrives on other window
// .19 Fix Scrollback
// .20 WinXP compatibility changes
// .21 Save Window Positions
// .22 Open a window on first start
// .23 Add Tabbed display option
// .24 Fix crash when setting Monitor flags in Tabbed mode
// .25 Add AGW mode
// .26 Add sending CTRL/C CTRL/D and CTRL/Z)
// .27 Limit size of windows to 10000 lines 
//	   Fix YAPP in Tabbed Mode
// .28 Add AGW Beacon
// .29 Fix allocating Listem sessions to tabs connected using AGW
// .30 Fix allocationd AGW Monitor Sessions
// .31 Fix output being written to the wrong place when window is scrolled back
// .32 Fix connect with digis
// .33 Add StripLF option
// .34 Improvements for Andriod
//	   Option to change Menu Fonts
//	   Fix receiving part lines when scrolled back
// .35 Fix PE if not in Tabbed Mode
// .36 Improved dialogs mainly for Android
//	   Try to make sure sessions are closed before exiting
// .37 Replace VT with LF (for pasting from BPQ32 Terminal Window)
//	   Dont show AGW status line if AGW interface is disabled
//	   Don't show Window Menu in Single Mode
// .38 Protect against connect to normal Telnet Port.
//	   Send CTEXT and Beep for inward AGW Connects
//	   Make sending Idle and Connect beeps configurable
//	   Change displayed Monitor flags when active window changed.
//	   Fix duplicate text on long lines
// .39 Add option to Convert non-utf8 charaters
// .40 Prevent crash if AGW monitor Window closed
// .41 Allow AGW Config and Connect dialogs to be resized with scrollbars
//	   Fix disabling connect flag on current session when AGW connects
// .42 Fix some bugs in AGW session handling
// .43 Include Ross KD5LPB's fixes for MAC
// .44 Ctrl/[ sends ESC (0x1b)
// .45 Add VARA support							Nov 2021
// .46 Fix dialog size on VARA setup
// .47 Move VARA Status indicator. 
//	   Add FLRIG PTT for VARA
// .48 Check that YAPP Receive Directory has been set
//	   Save separate configs for VARA, VARAFM and VARASAT
// .49 Add BBS Monitor							Dec 2021
// .50 Make Serial port support optional for Android Version
// .51 Add KISS TNC Support						May 2022
// .52 Save partially typed lines on cursor up	May 2022
// .53 Add UI Mode for KISS Sessions			May 2022
//     Fix saving KISS Paclen (b)
//	   Fix Keepalive  (c)
// .54 Add option to change screen colours		June 2022
// .55 Build without optimisation
// .56 Add Teletext mode						Nov 2022
// .57 Fix KISS mode incoming call handling
// .58 Add method to toggle Normal/Teletext Mode
//	   Fix KISS multiple session protection
// .59 Add Teletext double height mode
// .60 Add option to name sessions				Jan 2023
// .61 Add VARA Init Script						Feb 2023
// .62 Fix running AGW in single session mode	Feb 2023
// .63 Fix handling split monitor frame (no fe in buffer) Apr 2023
// .64 Add Clear Screen command to context menu Apr 2023
// .65 Fixes for 63 port version of BPQ			May 2023
// .66 Colour Tab of incoming calls				June 2023
// .67 Add config Yapp RX Size dialog			July 2023
//	   Fix 63 port montoring

// .68	Sept 2023

//	Remember last used host on restart	
//	Add AutoConnect in Tabbed Mode
//	Fix auto copy when QtTerm not active window

// .69	October 2023

//	Options related to sound alerts moved to a separte menu in Setup
//	Allow use of WAV files instead of Beep sound for sound alerts
//	Enable an alarm to be sounded when one of a list of words or phrases is received.

//	.70 October 2023

//	Include some .wav files in resources
//	Add Test button to Alert setup dialog

//	.71 October 2023

//	Add option to use local time
//	Fixes for Mac OS

//	.72 November 2023
//	Don't display "Enable Monitor" on startup

//	.73 November 2023
//	Raise RTS on KISS serial port




#define _CRT_SECURE_NO_WARNINGS

#define UNUSED(x) (void)(x)

#define USESERIAL

#include "QtTermTCP.h"
#include "TabDialog.h"
#include <time.h>
#include <QVBoxLayout>
#include <QListWidgetItem>
#include <QToolButton>
#include <QInputMethod>
#include "QTreeWidget"
#include <QToolBar>
#include <QLabel>
#include <QActionGroup>
#include <QtWidgets>
#include <QColor>
#ifdef USESERIAL
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#endif
#ifndef WIN32
#define strtok_s strtok_r
#endif
#include <fcntl.h>
#include <errno.h>
#ifndef WIN32
#include <unistd.h>
#endif

#include "ax25.h"

#define UNREFERENCED_PARAMETER(P)          (P)

void DecodeTeleText(Ui_ListenSession * Sess, char * text);

char Host[MAXHOSTS + 1][100] = { "" };
int Port[MAXHOSTS + 1] = { 0 };
char UserName[MAXHOSTS + 1][80] = { "" };
char Password[MAXHOSTS + 1][80] = { "" };
char MonParams[MAXHOSTS + 1][80] = { "" };
char SessName[MAXHOSTS + 1][80] = { "" };
int ListenPort = 8015;

// Session Type Equates

#define Term 1
#define Mon 2
#define Listen 4

// Presentation - Single Window, MDI or Tabbed

int TermMode = 0;

#define Single 0
#define MDI 1
#define Tabbed 2

int singlemodeFormat = Mon + Term;

char monStyleSheet[128] = "background-color: rgb(0,255,255)";
char termStyleSheet[128] = "background-color: rgb(255,0,255);";
char inputStyleSheet[128] = "color: rgb(255, 0, 0); background-color: rgb(255,255,0);";

QColor monBackground = qRgb(0, 255, 255);
QColor monRxText = qRgb(0, 0, 255);
QColor monTxText = qRgb(255, 0, 0);
QColor monOtherText = qRgb(0, 0, 0);

QColor termBackground = qRgb(255, 0, 255);
QColor outputText = qRgb(0, 0, 0);
QColor EchoText = qRgb(0, 0, 255);
QColor WarningText = qRgb(255, 0, 0);
QColor inputBackground = qRgb(255, 255, 0);
QColor inputText = qRgb(0, 0, 255);

QColor newTabText = qRgb(255, 0, 0);		// Red
QColor oldTabText = qRgb(0, 0, 0);			// Black


// There is something odd about this. It doesn't match BPQTERMTCP though it looks the same

// Chat uses these (+ 10)
//{ 0, 4, 9, 11, 13, 16, 17, 42, 45, 50, 61, 64, 66, 72, 81, 84, 85, 86, 87, 89 };

// As we have a white background we need dark colours

// 0 is black. 23 is normal output (blue) 81 is red (used by AGW Mon)
// for monitor in colour, host sends 17 for RX Text, 81 for TX Text but code converts to monRx/TxText qrgb values

QRgb Colours[256] = { 0,
		qRgb(0,0,0), qRgb(0,0,128), qRgb(0,0,192), qRgb(0,0,255),				// 1 - 4
		qRgb(0,64,0), qRgb(0,64,128), qRgb(0,64,192), qRgb(0,64,255),			// 5 - 8
		qRgb(0,128,0), qRgb(0,128,128), qRgb(0,128,192), qRgb(0,128,255),		// 9 - 12
		qRgb(0,192,0), qRgb(0,192,128), qRgb(0,192,192), qRgb(0,192,255),		// 13 - 16
		qRgb(0,255,0), qRgb(0,255,128), qRgb(0,255,192), qRgb(0,255,255),		// 17 - 20

		qRgb(64,0,0), qRgb(64,0,128), qRgb(64,0,192), qRgb(0,0,255),				// 21 
		qRgb(64,64,0), qRgb(64,64,128), qRgb(64,64,192), qRgb(64,64,255),
		qRgb(64,128,0), qRgb(64,128,128), qRgb(64,128,192), qRgb(64,128,255),
		qRgb(64,192,0), qRgb(64,192,128), qRgb(64,192,192), qRgb(64,192,255),
		qRgb(64,255,0), qRgb(64,255,128), qRgb(64,255,192), qRgb(64,255,255),

		qRgb(128,0,0), qRgb(128,0,128), qRgb(128,0,192), qRgb(128,0,255),				// 41
		qRgb(128,64,0), qRgb(128,64,128), qRgb(128,64,192), qRgb(128,64,255),
		qRgb(128,128,0), qRgb(128,128,128), qRgb(128,128,192), qRgb(128,128,255),
		qRgb(128,192,0), qRgb(128,192,128), qRgb(128,192,192), qRgb(128,192,255),
		qRgb(128,255,0), qRgb(128,255,128), qRgb(128,255,192), qRgb(128,255,255),

		qRgb(192,0,0), qRgb(192,0,128), qRgb(192,0,192), qRgb(192,0,255),				// 61 - 80
		qRgb(192,64,0), qRgb(192,64,128), qRgb(192,64,192), qRgb(192,64,255),
		qRgb(192,128,0), qRgb(192,128,128), qRgb(192,128,192), qRgb(192,128,255),
		qRgb(192,192,0), qRgb(192,192,128), qRgb(192,192,192), qRgb(192,192,255),
		qRgb(192,255,0), qRgb(192,255,128), qRgb(192,255,192), qRgb(192,255,255),

		qRgb(255,0,0), qRgb(255,0,128), qRgb(255,0,192), qRgb(255,0,255),				// 81 - 100
		qRgb(255,64,0), qRgb(255,64,128), qRgb(255,64,192), qRgb(255,64,255),
		qRgb(255,128,0), qRgb(255,128,128), qRgb(255,128,192), qRgb(255,128,255),
		qRgb(255,192,0), qRgb(255,192,128), qRgb(255,192,192), qRgb(255,192,255),
		qRgb(255,255,0), qRgb(255,255,128), qRgb(255,255,192), qRgb(255,255,255)
};



int SavedHost = 0;				// from config

char * sessionList = NULL;		// Saved sessions

extern int ChatMode;
extern int Bells;
extern int StripLF;
extern int convUTF8;

extern time_t LastWrite;
extern int AlertInterval;
extern int AlertBeep;
extern int AlertFreq;
extern int AlertDuration;
extern int ConnectBeep;

bool useBeep;			// use wav files if not set

extern int UseKeywords;
extern QString KeyWordsFile;

QString ConnectWAV("");
QString AlertWAV("");
QString BellWAV("");
QString IntervalWAV("");

extern int MaxRXSize;

extern int AutoTeletext;

// AGW Host Interface stuff

int AGWEnable = 0;
int AGWMonEnable = 0;
int AGWLocalTime = 0;
int AGWMonNodes = 0;
char AGWTermCall[12] = "";
char AGWBeaconDest[12] = "";
char AGWBeaconPath[80] = "";
int AGWBeaconInterval = 0;
char AGWBeaconPorts[80];
char AGWBeaconMsg[260] = "";


char AGWHost[128] = "127.0.0.1";
int AGWPortNum = 8000;
int AGWPaclen = 80;
extern char * AGWPortList;
extern myTcpSocket * AGWSock;

typedef struct AGWUser_t
{
	QTcpSocket *socket;
	unsigned char data_in[8192];
	int data_in_len;
	unsigned char  AGW_frame_buf[512];
	int	Monitor;
	int	Monitor_raw;
	Ui_ListenSession * MonSess;			// Window for Monitor info

} AGWUser;

extern AGWUser *AGWUsers;				// List of currently connected clients
void Send_AGW_m_Frame(QTcpSocket* socket);


QStringList AGWToCalls;

// KISS Interface

int KISSEnable = 0;
extern "C" int KISSMonEnable;
extern "C" int KISSLocalTime;
extern "C" int KISSMonNodes;
extern "C" int KISSListen;

char SerialPort[80] = "";
char KISSHost[128] = "127.0.0.1";
int KISSPortNum = 1000;
int KISSBAUD = 19200;
char MYCALL[32];
extern "C" UCHAR axMYCALL[7];			// Mycall in ax.25

int KISSMode = 0;			// Connected (0) or UI (1)

myTcpSocket * KISSSock;

extern "C" void * KISSSockCopy[4];

int KISSConnected = 0;
int KISSConnecting = 0;

Ui_ListenSession * KISSMonSess = nullptr;

QSerialPort * m_serial = nullptr;

// VARA Interface

int VARAEnable = 0;
char VARAHost[128] = "127.0.0.1";
char VARAHostHF[128] = "127.0.0.1";
char VARAHostFM[128] = "127.0.0.1";
char VARAHostSAT[128] = "127.0.0.1";
int VARAPortNum = 8000;
int VARAPortHF = 8000;
int VARAPortFM = 8000;
int VARAPortSAT = 8000;
char VARATermCall[12] = "";
char VARAPath[256] = "";
char VARAInit[256] = "";
char VARAPathHF[256] = "";
char VARAPathFM[256] = "";
char VARAPathSAT[256] = "";

int VARA500 = 0;
int VARA2300 = 1;
int VARA2750 = 0;

int VARAHF = 1;
int VARAFM = 0;
int VARASAT = 0;

myTcpSocket * VARASock;
myTcpSocket * VARADataSock;

int VARAConnected = 0;
int VARAConnecting = 0;


extern char YAPPPath[256];

void menuChecked(QAction * Act);

// PTT Stuff

#define PTTRTS		1
#define PTTDTR		2
#define PTTCAT		4
#define PTTCM108	8
#define PTTHAMLIB	16
#define PTTFLRIG	32

#ifdef USESERIAL

QSerialPort * hPTTDevice = 0;
#else


#endif
char PTTPort[80] = "";			// Port for Hardware PTT - may be same as control port.
int PTTBAUD = 19200;
int PTTMode = PTTRTS;			// PTT Control Flags.

char PTTOnString[128] = "";
char PTTOffString[128] = "";

int CATHex = 1;

unsigned char PTTOnCmd[64];
int PTTOnCmdLen = 0;

unsigned char  PTTOffCmd[64];
int PTTOffCmdLen = 0;

int pttGPIOPin = 17;			// Default
int pttGPIOPinR = 17;
bool pttGPIOInvert = false;
bool useGPIO = false;
bool gotGPIO = false;

int HamLibPort = 4532;
char HamLibHost[32] = "192.168.1.14";

int FLRigPort = 12345;
char FLRigHost[32] = "127.0.0.1";


char CM108Addr[80] = "";

int VID = 0;
int PID = 0;

// CM108 Code

char * CM108Device = NULL;

QProcess *process = NULL;

void GetSettings();

// These widgets defined here as they are accessed from outside the framework

QLabel * Status1;
QLabel * Status2;
QLabel * Status3;
QLabel * Status4;

QAction *actHost[19];
QAction *actSetup[16];

QAction * TabSingle = NULL;
QAction * TabBoth = NULL;
QAction * TabMon = NULL;

QMenu *monitorMenu;
QMenu * YAPPMenu;
QMenu *connectMenu;
QMenu *disconnectMenu;

QAction *EnableMonitor;
QAction *EnableMonLog;
QAction *MonLocalTime;
QAction *MonTX;
QAction *MonSup;
QAction *MonNodes;
QAction *MonUI;
QAction *MonColour;
QAction *MonPort[65];
QAction *actChatMode;
QAction *actAutoTeletext;
QAction *actBells;
QAction *actStripLF;
QAction *actIntervalBeep;
QAction *actConnectBeep;
QAction *actAuto;
QAction *actnoConv;
QAction *actCP1251;
QAction *actCP1252;
QAction *actCP437;

QAction *actFonts;
QAction *actmenuFont;
QAction *actColours;
QAction *actmyFonts;
QAction *YAPPSend;
QAction *YAPPSetRX;
QAction *YAPPSetSize;
QAction *singleAct;
QAction *singleAct2;
QAction *MDIAct;
QAction *tabbedAct;
QAction *discAction;
QFont * menufont;
QMenu *windowMenu;
QMenu *setupMenu;
QAction *ListenAction;

QTabWidget *tabWidget;
QMdiArea *mdiArea;
QWidget * mythis;
QStatusBar * myStatusBar;

QTcpServer * _server;

QMenuBar *mymenuBar;
QToolBar *toolBar;
QToolButton * but1;
QToolButton * but2;
QToolButton * but3;
QToolButton * but4;
QToolButton * but5;

QList<Ui_ListenSession *> _sessions;

// Session Type Equates

#define Term 1
#define Mon 2
#define Listen 4

int TabType[10] = { Term, Term + Mon, Term, Term, Term, Term, Mon, Mon, Mon };

int AutoConnect[10] = {0, 0 ,0, 0, 0, 0, 0, 0, 0, 0};

int currentHost[10] = {0, 0 ,0, 0, 0, 0, 0, 0, 0, 0};

int listenPort = 8015;
extern "C" int listenEnable;
char listenCText[4096] = "";

int ConfigHost = 0;

int Split = 50;				// Mon/Term size split

int termX;

bool Cascading = false;		// Set to stop size being saved when cascading

QMdiSubWindow * ActiveSubWindow = NULL;
Ui_ListenSession * ActiveSession = NULL;

Ui_ListenSession * newWindow(QObject * parent, int Type, const char * Label = nullptr);
void Send_AGW_C_Frame(Ui_ListenSession * Sess, int Port, char * CallFrom, char * CallTo, char * Data, int DataLen);
void AGW_AX25_data_in(void * AX25Sess, unsigned char * data, int Len);
void AGWMonWindowClosing(Ui_ListenSession *Sess);
void AGWWindowClosing(Ui_ListenSession *Sess);
extern "C" void KISSDataReceived(void * socket, unsigned char * data, int length);
void closeSerialPort();

extern void initUTF8();
int checkUTF8(unsigned char * Msg, int Len, unsigned char * out);

QDialog * deviceUI;

#include <QStandardPaths>
#include <sys/stat.h>
#include <sys/types.h>




void EncodeSettingsLine(int n, char * String)
{
	sprintf(String, "%s|%d|%s|%s|%s|%s", Host[n], Port[n], UserName[n], Password[n], MonParams[n], SessName[n]);
	return;
}

// This is used for placing discAction in the preference menu
#ifdef __APPLE__
bool is_mac = true;
#else
bool is_mac = false;
#endif

QString GetConfPath()
{
	std::string conf_path = "QtTermTCP.ini";  // Default conf file stored alongside application.

#ifdef __APPLE__

	// Get configuration path for MacOS.

	// This will usually be placed in /Users/USER/Library/Application Support/QtTermTCP

	std::string directory = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).at(0).toStdString();

	conf_path = directory + "/QtTermTCP.ini";

	mkdir(directory.c_str(), 0775);

#endif

	return QString::fromUtf8(conf_path.c_str());

}



void DecodeSettingsLine(int n, char * String)
{
	char * Param = strdup(String);
	char * Rest;
	char * Save = Param;			// for Free

	Rest = strlop(Param, '|');

	if (Rest == NULL)
		return;

	strcpy(Host[n], Param);
	Param = Rest;

	Rest = strlop(Param, '|');
	Port[n] = atoi(Param);
	Param = Rest;

	Rest = strlop(Param, '|');
	strcpy(UserName[n], Param);
	Param = Rest;

	Rest = strlop(Param, '|');
	strcpy(Password[n], Param);
	Param = Rest;

	Rest = strlop(Param, '|');
	strcpy(MonParams[n], Param);

	if (Rest)
		strcpy(SessName[n], Rest);

	free(Save);
	return;
}

//#ifdef ANDROID
//#define inputheight 60
//#else
#define inputheight 25
//#endif

extern "C" void SendtoAX25(void * conn, unsigned char * Msg, int Len);

void DoTermResize(Ui_ListenSession * Sess)
{

	QRect r = Sess->rect();

	int H, Mid, monHeight, termHeight, Width, Border = 3;

	Width = r.width();
	H = r.height();

	if (TermMode == Tabbed)
	{
		//					H -= 20;
		Width -= 7;
	}
	else if (TermMode == Single)
	{
		Width -= 7;
		//					H += 20;
	}

	if (Sess->SessionType == Listen || Sess->SessionType == Term)
	{
		// Term and Input

		// Calc Positions of window

		termHeight = H - (inputheight + 3 * Border);

		Sess->termWindow->setGeometry(QRect(Border, Border, Width, termHeight));

		if (Sess->TTActive)
		{
			Sess->TTLabel->setGeometry(QRect(Border, Border, 600, 475));
//			Sess->termWindow->setVisible(false);
			Sess->TTLabel->setVisible(true);
		}
		else
		{
//			Sess->termWindow->setVisible(true);
			Sess->TTLabel->setVisible(false);
		}

		Sess->inputWindow->setGeometry(QRect(Border, H - (inputheight + Border), Width, inputheight));
	}
	else if (Sess->SessionType == (Term | Mon))
	{
		// All 3

		// Calc Positions of window

		Mid = (H * Split) / 100;

		monHeight = Mid - Border;
		termX = Mid;
		termHeight = H - Mid - (inputheight + 3 * Border);

		Sess->monWindow->setGeometry(QRect(Border, Border, Width, monHeight));
		Sess->termWindow->setGeometry(QRect(Border, Mid + Border, Width, termHeight));

		if (Sess->TTActive)
		{
			Sess->TTLabel->setGeometry(QRect(3, Mid + Border, 600, 475));
//			Sess->termWindow->setVisible(false);
			Sess->TTLabel->setVisible(true);
		}
		else
		{
//			Sess->termWindow->setVisible(true);
			Sess->TTLabel->setVisible(false);
		}

		Sess->inputWindow->setGeometry(QRect(Border, H - (inputheight + Border), Width, inputheight));

	}
	else
	{
		// Should just be Mon only

		Sess->monWindow->setGeometry(QRect(Border, Border, Width, H - 2 * Border));
	}
}

bool QtTermTCP::eventFilter(QObject* obj, QEvent *event)
{
	// See if from a Listening Session

	Ui_ListenSession * Sess;

	for (int i = 0; i < _sessions.size(); ++i)
	{
		Sess = _sessions.at(i);

		//	Sess = _sessions.at(i);	for (Ui_ListenSession * Sess : _sessions)
		//	{
		if (Sess == NULL)
			continue;

		if (Sess == obj)
		{
			if (event->type() == QEvent::Close)
			{
				// Window closing

				// This only closed mon sessinos

				if (Sess->AGWMonSession)
					//					AGWMonWindowClosing(Sess);
					AGWWindowClosing(Sess);

				if (Sess->AGWSession)
					AGWWindowClosing(Sess);

				if (Sess->clientSocket)
				{
					int loops = 100;
					Sess->clientSocket->disconnectFromHost();
					while (Sess->clientSocket && loops-- && Sess->clientSocket->state() != QAbstractSocket::UnconnectedState)
						QThread::msleep(10);
				}
				_sessions.removeOne(Sess);
				return QMainWindow::eventFilter(obj, event);
			}

			if (event->type() == QEvent::Resize)
			{
				DoTermResize(Sess);
			}
		}

		if (Sess->inputWindow == obj)
		{
			if (event->type() == QEvent::KeyPress)
			{
				QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

				int key = keyEvent->key();
				Qt::KeyboardModifiers modifier = keyEvent->modifiers();

				if (modifier == Qt::ControlModifier)
				{
					char Msg[] = "\0\r";

					if (key == Qt::Key_BracketLeft)
						Msg[0] = 0x1b;
					if (key == Qt::Key_Z)
						Msg[0] = 0x1a;
					else if (key == Qt::Key_C)
						Msg[0] = 3;
					else if (key == Qt::Key_D)
						Msg[0] = 4;

					if (Msg[0])
					{
						if (Sess->KISSSession)
						{
							// Send to ax.25 code

							SendtoAX25(Sess->KISSSession, (unsigned char *)Msg, (int)strlen(Msg));
						}
						else if (Sess->AGWSession)
						{
							// Terminal is in AGWPE mode - send as AGW frame

							AGW_AX25_data_in(Sess->AGWSession, (unsigned char *)Msg, (int)strlen(Msg));

						}
						else if (Sess->clientSocket && Sess->clientSocket->state() == QAbstractSocket::ConnectedState)
						{
							Sess->clientSocket->write(Msg);
						}

						return true;
					}
				}

				if (key == Qt::Key_Up)
				{
					// Scroll up

					if (Sess->KbdStack[Sess->StackIndex] == NULL)
						return true;

					// If anything typed, stack part command

					QByteArray stringData = Sess->inputWindow->text().toUtf8();

					if (stringData.length() && Sess->StackIndex == 0)
					{
						if (Sess->KbdStack[49])
							free(Sess->KbdStack[49]);

						for (int i = 48; i >= 0; i--)
						{
							Sess->KbdStack[i + 1] = Sess->KbdStack[i];
						}

						Sess->KbdStack[0] = qstrdup(stringData.data());
						Sess->StackIndex++;
					}

					Sess->inputWindow->setText(Sess->KbdStack[Sess->StackIndex]);
					Sess->inputWindow->cursorForward(strlen(Sess->KbdStack[Sess->StackIndex]));

					Sess->StackIndex++;
					if (Sess->StackIndex == 50)
						Sess->StackIndex = 49;

					return true;
				}
				else if (key == Qt::Key_Down)
				{
					// Scroll down

					if (Sess->StackIndex == 0)
					{
						Sess->inputWindow->setText("");
						return true;
					}

					Sess->StackIndex--;

					if (Sess->StackIndex && Sess->KbdStack[Sess->StackIndex - 1])
					{
						Sess->inputWindow->setText(Sess->KbdStack[Sess->StackIndex - 1]);
						Sess->inputWindow->cursorForward(strlen(Sess->KbdStack[Sess->StackIndex - 1]));
					}
					else
						Sess->inputWindow->setText("");

					return true;
				}
				else if (key == Qt::Key_Return || key == Qt::Key_Enter)
				{
					LreturnPressed(Sess);
					return true;
				}

				return false;
			}

			if (event->type() == QEvent::MouseButtonPress)
			{
				QMouseEvent *k = static_cast<QMouseEvent *> (event);

				// Paste on Right Click

				if (k->button() == Qt::RightButton)
				{
					Sess->inputWindow->paste();
					return true;
				}
				return QMainWindow::eventFilter(obj, event);
			}
		}
	}
	return QMainWindow::eventFilter(obj, event);
}

QAction * setupMenuLine(QMenu * Menu, char * Label, QObject * parent, int State)
{
	QAction * Act = new QAction(Label, parent);
	if (Menu)
		Menu->addAction(Act);

	Act->setCheckable(true);
	if (State)
		Act->setChecked(true);

	parent->connect(Act, SIGNAL(triggered()), parent, SLOT(menuChecked()));

	Act->setFont(*menufont);

	return Act;
}

// This now creates all window types - Term, Mon, Combined, Listen

int xCount = 0;
int sessNo = 0;

Ui_ListenSession * newWindow(QObject * parent, int Type, const char * Label)
{
	Ui_ListenSession * Sess = new(Ui_ListenSession);

	// Need to explicity initialise on Qt4

	Sess->termWindow = NULL;
	Sess->monWindow = NULL;
	Sess->inputWindow = NULL;

	for (int i = 0; i < 50; i++)
		Sess->KbdStack[i] = NULL;

	Sess->StackIndex = 0;
	Sess->InputMode = 0;
	Sess->SlowTimer = 0;
	Sess->MonData = 0;
	Sess->OutputSaveLen = 0;
	Sess->MonSaveLen = 0;
	Sess->PortMonString[0] = 0;
	Sess->portmask = 0;
	Sess->portmask = 1;
	Sess->mtxparam = 1;
	Sess->mlocaltime = 0;
	Sess->mcomparam = 1;
	Sess->monUI = 0;
	Sess->MonitorNODES = 0;
	Sess->MonitorColour = 1;
	Sess->CurrentHost = 0;

	Sess->SessionType = Type;
	Sess->clientSocket = NULL;
	Sess->AGWSession = NULL;
	Sess->AGWMonSession = NULL;
	Sess->KISSSession = NULL;
	Sess->KISSMode = 0;
	Sess->TTActive = 0;
	Sess->TTFlashToggle = 0;
	Sess->pageBuffer[0] = 0;
	Sess->Tab = 0;

	Sess->LogMonitor = false;
	Sess->monSpan = (char *) "<span style=white-space:pre>";
	Sess->monLogfile = nullptr;
	Sess->sessNo = sessNo++;

	_sessions.append(Sess);

//	Sess->TT = new Ui_TeleTextDialog();

//	Sess->TT->setupUi(&Sess->TTUI);

//	Sess->TTUI.show();
//	Sess->TTUI.raise();
//	Sess->TTUI.activateWindow();

	//	Sess->setObjectName(QString::fromUtf8("Sess"));

	if (TermMode == MDI)
	{
		Sess->sw = mdiArea->addSubWindow(Sess);
		//		Sess->installEventFilter(parent);

		Sess->sw->resize(800, 600);
	}
	else if (TermMode == Tabbed)
	{
		Sess->Tab = xCount++;

		if (Type == Mon)
			tabWidget->addTab(Sess, "Monitor");
		else
			tabWidget->addTab(Sess, Label);
	}

	Sess->installEventFilter(parent);

	QSettings settings(GetConfPath(), QSettings::IniFormat);

#ifdef ANDROID
	QFont font = QFont(settings->value("FontFamily", "Driod Sans Mono").value<QString>(),
		settings->value("PointSize", 12).toInt(),
		settings->value("Weight", 50).toInt());
#else
	QFont font = QFont(settings.value("FontFamily", "Courier New").value<QString>(),
		settings.value("PointSize", 10).toInt(),
		settings.value("Weight", 50).toInt());
#endif

	if ((Type & Mon))
	{
		Sess->monWindow = new QTextEdit(Sess);
		Sess->monWindow->setReadOnly(1);
		Sess->monWindow->document()->setMaximumBlockCount(10000);
		Sess->monWindow->setFont(font);
		Sess->monWindow->setStyleSheet(monStyleSheet);
		mythis->connect(Sess->monWindow, SIGNAL(selectionChanged()), parent, SLOT(onTEselectionChanged()));
	}

	if ((Type & (Listen | Term)))
	{
		Sess->termWindow = new QTextEdit(Sess);
		Sess->termWindow->setReadOnly(1);
		Sess->termWindow->document()->setMaximumBlockCount(10000);
		Sess->termWindow->setFont(font);
		Sess->termWindow->setStyleSheet(termStyleSheet);

		Sess->TTLabel = new QLabel(Sess);
		Sess->TTLabel->setGeometry(QRect(0,0 ,500, 500));
		Sess->TTLabel->setVisible(false);

		Sess->TTBitmap = new QImage(40 * 15, 25 * 19, QImage::Format_RGB32);
		Sess->TTBitmap->fill(Qt::black);

/*

		char Page[4096];

		QFile file("/savepage.txt");
		file.open(QIODevice::ReadOnly);
		file.read(Page, 4096);
		file.close();

		Sess->TTActive = 1;
		strcpy(Sess->pageBuffer, Page);
		DecodeTeleText(Sess, Sess->pageBuffer);

*/

		Sess->TTLabel->setPixmap(QPixmap::fromImage(*Sess->TTBitmap));

		Sess->TTLabel->setContextMenuPolicy(Qt::CustomContextMenu);
		mythis->connect(Sess->TTLabel, SIGNAL(customContextMenuRequested(const QPoint&)),
			parent, SLOT(showContextMenuL()));

		mythis->connect(Sess->termWindow, SIGNAL(selectionChanged()), parent, SLOT(onTEselectionChanged()));

		Sess->inputWindow = new QLineEdit(Sess);
		Sess->inputWindow->installEventFilter(parent);
		Sess->inputWindow->setFont(font);
		Sess->inputWindow->setStyleSheet(inputStyleSheet);
		Sess->inputWindow->setContextMenuPolicy(Qt::PreventContextMenu);

		mythis->connect(Sess->inputWindow, SIGNAL(selectionChanged()), parent, SLOT(onLEselectionChanged()));
	}

	if (Type == Term || Type == Listen)
	{
		// Term Only or Listen Only

		Sess->termWindow->setContextMenuPolicy(Qt::CustomContextMenu);

		mythis->connect(Sess->termWindow, SIGNAL(customContextMenuRequested(const QPoint&)),
			parent, SLOT(showContextMenuT(const QPoint &)));

	}

	if (Type == Mon)
	{
		// Monitor Only

		Sess->monWindow->setContextMenuPolicy(Qt::CustomContextMenu);

		mythis->connect(Sess->monWindow, SIGNAL(customContextMenuRequested(const QPoint&)),
			parent, SLOT(showContextMenuMOnly(const QPoint &)));

	}

	if (Type == (Term | Mon))
	{
		// Combined Term and Mon. Add Custom Menu to set Mon/Term Split with Right Click

		Sess->monWindow->setContextMenuPolicy(Qt::CustomContextMenu);
		Sess->termWindow->setContextMenuPolicy(Qt::CustomContextMenu);


		mythis->connect(Sess->monWindow, SIGNAL(customContextMenuRequested(const QPoint&)),
			parent, SLOT(showContextMenuM(const QPoint &)));

		mythis->connect(Sess->termWindow, SIGNAL(customContextMenuRequested(const QPoint&)),
			parent, SLOT(showContextMenuMT(const QPoint &)));
	}

	if (Sess->SessionType == Mon)			// Mon Only
		Sess->setWindowTitle("Monitor Session Disconnected");
	else if (Sess->SessionType == Listen)			// Mon Only
		Sess->setWindowTitle("Listen Window Disconnected");
	else
		Sess->setWindowTitle("Disconnected");

	Sess->installEventFilter(mythis);

	Sess->show();

	int pos = (_sessions.size() - 1) * 20;

	if (TermMode == MDI)
	{
		Sess->sw->move(pos, pos);
	}

	QSize Size(800, 602);						// Not actually used, but Event constructor needs it

	QResizeEvent event(Size, Size);

	QApplication::sendEvent(Sess, &event);		// Resize Widgets to fix Window

	return Sess;
}

QByteArray timeLoaded = QDateTime::currentDateTime().toString("yymmdd_hhmmss").toUtf8();

QtTermTCP::QtTermTCP(QWidget *parent) : QMainWindow(parent)
{
	int i;
	char Title[80];

	// Get configuration path for MacOS.
	std::string directory = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).at(0).toStdString();
	std::string conf_path = directory + "/QtTermTCP.ini";

	//mkdir(directory.c_str(), 0775);

	_server = new QTcpServer();

	mythis = this;

	ui.setupUi(this);

	initUTF8();

	mymenuBar = new QMenuBar(this);
	mymenuBar->setGeometry(QRect(0, 0, 781, 26));
	setMenuBar(mymenuBar);

	toolBar = new QToolBar(this);
	toolBar->setObjectName("mainToolbar");
	addToolBar(Qt::BottomToolBarArea, toolBar);

	QSettings mysettings(GetConfPath(), QSettings::IniFormat);

	restoreGeometry(mysettings.value("geometry").toByteArray());
	restoreState(mysettings.value("windowState").toByteArray());

	GetSettings();

	GetKeyWordFile();

	// Set background colours

	sprintf(monStyleSheet, "background-color: rgb(%d, %d, %d);",
		monBackground.red(), monBackground.green(), monBackground.blue());

	sprintf(termStyleSheet, "background-color: rgb(%d, %d, %d);",
		termBackground.red(), termBackground.green(), termBackground.blue());

	sprintf(inputStyleSheet, "color: rgb(%d, %d, %d); background-color: rgb(%d, %d, %d);",
		inputText.red(), inputText.green(), inputText.blue(),
		inputBackground.red(), inputBackground.green(), inputBackground.blue());


#ifdef ANDROID
	menufont = new QFont(mysettings.value("MFontFamily", "Driod Sans").value<QString>(),
		mysettings.value("MPointSize", 12).toInt(),
		mysettings.value("MWeight", 50).toInt());
#else
	menufont = new QFont(mysettings.value("MFontFamily", "Aerial").value<QString>(),
		mysettings.value("MPointSize", 10).toInt(),
		mysettings.value("MWeight", 50).toInt());

#endif
	if (TermMode == MDI)
	{
		mdiArea = new QMdiArea(ui.centralWidget);
		mdiArea->setGeometry(QRect(0, 0, 771, 571));

		mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

		connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(xon_mdiArea_changed()));

		setCentralWidget(mdiArea);
	}
	else if (TermMode == Tabbed)
	{
		Ui_ListenSession * Sess;
		int index = 0;
	
		tabWidget = new QTabWidget(this);
		QTabBar* tabBar = tabWidget->tabBar();

//		QString style1 = "QTabWidget::tab-bar{left:0;}";  //  for Mac
//		tabWidget->setStyleSheet(style1);

		QString style = "QTabBar::tab:selected{background-color: #d0d0d0;} QTabBar::tab:!selected{background-color: #f0f0f0;}";

		tabBar->setStyleSheet(style);

		tabWidget->setContextMenuPolicy(Qt::CustomContextMenu);

		connect(tabWidget, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(showContextMenu(const QPoint &)));

		ui.verticalLayout->addWidget(tabWidget);
		tabWidget->setTabPosition(QTabWidget::South);

		Sess = newWindow(this, TabType[0], "Sess 1");
		Sess->CurrentHost = currentHost[index++];
		Sess = newWindow(this, TabType[1], "Sess 2");
		Sess->CurrentHost = currentHost[index++];
		Sess = newWindow(this, TabType[2], "Sess 3");
		Sess->CurrentHost = currentHost[index++];
		Sess = newWindow(this, TabType[3], "Sess 4");
		Sess->CurrentHost = currentHost[index++];
		Sess = newWindow(this, TabType[4], "Sess 5");
		Sess->CurrentHost = currentHost[index++];
		Sess = newWindow(this, TabType[5], "Sess 6");
		Sess->CurrentHost = currentHost[index++];
		Sess = newWindow(this, TabType[6], "Sess 7");
		Sess->CurrentHost = currentHost[index++];
		Sess = newWindow(this, TabType[7], "Monitor");
		Sess->CurrentHost = currentHost[index++];
		Sess = newWindow(this, TabType[8], "Monitor");
		Sess->CurrentHost = currentHost[index++];

		ActiveSession = _sessions.at(0);

		connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabSelected(int)));

		tabWidget->setFont(*menufont);

	}

	sprintf(Title, "QtTermTCP Version %s", VersionString);

	this->setWindowTitle(Title);

	//#ifdef ANDROID
	//	mymymenuBar->setVisible(false);
	//#endif

	if (TermMode == Single)
		windowMenu = mymenuBar->addMenu("");
	else
		windowMenu = mymenuBar->addMenu(tr("&Window"));

	connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(updateWindowMenu()));
	windowMenu->setFont(*menufont);

	newTermAct = new QAction(tr("New Terminal Window"), this);
	connect(newTermAct, SIGNAL(triggered()), this, SLOT(doNewTerm()));

	newMonAct = new QAction(tr("New Monitor Window"), this);
	connect(newMonAct, SIGNAL(triggered()), this, SLOT(doNewMon()));

	newCombinedAct = new QAction(tr("New Combined Term/Mon Window"), this);
	connect(newCombinedAct, SIGNAL(triggered()), this, SLOT(doNewCombined()));

	if (TermMode == MDI)
	{
		closeAct = new QAction(tr("Cl&ose"), this);
		closeAct->setStatusTip(tr("Close the active window"));
		connect(closeAct, SIGNAL(triggered()), mdiArea, SLOT(closeActiveSubWindow()));

		closeAllAct = new QAction(tr("Close &All"), this);
		closeAllAct->setStatusTip(tr("Close all the windows"));
		connect(closeAllAct, SIGNAL(triggered()), mdiArea, SLOT(closeAllSubWindows()));

		tileAct = new QAction(tr("&Tile"), this);
		tileAct->setStatusTip(tr("Tile the windows"));
		connect(tileAct, SIGNAL(triggered()), mdiArea, SLOT(tileSubWindows()));

		cascadeAct = new QAction(tr("&Cascade"), this);
		cascadeAct->setStatusTip(tr("Cascade the windows"));
		connect(cascadeAct, SIGNAL(triggered()), this, SLOT(doCascade()));

		nextAct = new QAction(tr("Ne&xt"), this);
		nextAct->setShortcuts(QKeySequence::NextChild);
		nextAct->setStatusTip(tr("Move the focus to the next window"));
		connect(nextAct, SIGNAL(triggered()), mdiArea, SLOT(activateNextSubWindow()));

		previousAct = new QAction(tr("Pre&vious"), this);
		previousAct->setShortcuts(QKeySequence::PreviousChild);
		previousAct->setStatusTip(tr("Move the focus to the previous window"));
		connect(previousAct, SIGNAL(triggered()), mdiArea, SLOT(activatePreviousSubWindow()));
	}

	quitAction = new QAction(tr("&Quit"), this);
	connect(quitAction, SIGNAL(triggered()), this, SLOT(doQuit()));

	windowMenuSeparatorAct = new QAction(this);
	windowMenuSeparatorAct->setSeparator(true);

	updateWindowMenu();

	connectMenu = mymenuBar->addMenu(tr("&Connect"));

	actHost[16] = new QAction("AGW Connect", this);
	actHost[16]->setFont(*menufont);
	actHost[16]->setVisible(0);
	connectMenu->addAction(actHost[16]);

	connect(actHost[16], SIGNAL(triggered()), this, SLOT(Connect()));

	actHost[17] = new QAction("VARA Connect", this);
	actHost[17]->setFont(*menufont);
	actHost[17]->setVisible(0);
	connectMenu->addAction(actHost[17]);

	connect(actHost[17], SIGNAL(triggered()), this, SLOT(Connect()));

	actHost[18] = new QAction("KISS Connect", this);
	actHost[18]->setFont(*menufont);
	actHost[18]->setVisible(KISSEnable);
	actHost[18]->setEnabled(0);
	connectMenu->addAction(actHost[18]);

	connect(actHost[18], SIGNAL(triggered()), this, SLOT(Connect()));
	
	for (i = 0; i < MAXHOSTS; i++)
	{
		if (SessName[i][0])
		{
			char Lable[256];
			sprintf(Lable, "%s(%s)", Host[i], SessName[i]);
			actHost[i] = new QAction(Lable, this);
		}
		else
			actHost[i] = new QAction(Host[i], this);

		actHost[i]->setFont(*menufont);
		connectMenu->addAction(actHost[i]);
		connect(actHost[i], SIGNAL(triggered()), this, SLOT(Connect()));
	}

	discAction = mymenuBar->addAction("&Disconnect");

	// Place discAction in mac app menu, otherwise it doesn't appear
	if (is_mac == true) {
		QMenu * app_menu = mymenuBar->addMenu("App Menu");
		discAction->setMenuRole(QAction::ApplicationSpecificRole);
		app_menu->addAction(discAction);

	}

	connect(discAction, SIGNAL(triggered()), this, SLOT(Disconnect()));
	discAction->setEnabled(false);

	toolBar->setFont(*menufont);

#ifndef ANDROID
	toolBar->setVisible(false);
#endif
	but4 = new QToolButton();
	but4->setPopupMode(QToolButton::InstantPopup);
	but4->setText("Window");
	but4->addAction(windowMenu->menuAction());
	but4->setFont(*menufont);
	toolBar->addWidget(but4);

	but5 = new QToolButton();
	but5->setPopupMode(QToolButton::InstantPopup);
	but5->setText("Connect");
	but5->addAction(connectMenu->menuAction());
	but5->setFont(*menufont);

	toolBar->addWidget(but5);

	toolBar->addAction(discAction);

	setupMenu = mymenuBar->addMenu(tr("&Setup"));
	hostsubMenu = setupMenu->addMenu("Hosts");
	hostsubMenu->setFont(*menufont);

	for (i = 0; i < MAXHOSTS; i++)
	{
		if (Host[i][0])
		{
			char Label[256];

			if (SessName[i][0])
				sprintf(Label, "%s(%s)", Host[i], SessName[i]);
			else
				strcpy(Label, Host[i]);

			actSetup[i] = new QAction(Label, this);
		}
		else
			actSetup[i] = new QAction("New Host", this);

		hostsubMenu->addAction(actSetup[i]);
		connect(actSetup[i], SIGNAL(triggered()), this, SLOT(SetupHosts()));

		actSetup[i]->setFont(*menufont);
	}


	// Setup Presentation Options

	setupMenu->addSeparator()->setText(tr("Presentation"));

	QActionGroup * termGroup = new QActionGroup(this);

	singleAct = setupMenuLine(nullptr, (char *)"Single Window (Term + Mon)", this, (TermMode == Single) && (singlemodeFormat == Term + Mon));
	singleAct2 = setupMenuLine(nullptr, (char *)"Single Window (Term only)", this, (TermMode == Single) && (singlemodeFormat == Term));
	MDIAct = setupMenuLine(nullptr, (char *)"MDI", this, TermMode == MDI);
	tabbedAct = setupMenuLine(nullptr, (char *)"Tabbed", this, TermMode == Tabbed);

	termGroup->addAction(singleAct);
	termGroup->addAction(singleAct2);
	termGroup->addAction(MDIAct);
	termGroup->addAction(tabbedAct);

	setupMenu->addAction(singleAct);
	setupMenu->addAction(singleAct2);
	setupMenu->addAction(MDIAct);
	setupMenu->addAction(tabbedAct);
	setupMenu->addSeparator();
	AlertAction = new QAction("Sound Alerts Setup", this);

	setupMenu->addAction(AlertAction);
	connect(AlertAction, SIGNAL(triggered()), this, SLOT(AlertSlot()));
	AlertAction->setFont(*menufont);
	setupMenu->addSeparator();

	actFonts = new QAction("Terminal Font", this);
	setupMenu->addAction(actFonts);
	connect(actFonts, SIGNAL(triggered()), this, SLOT(doFonts()));
	actFonts->setFont(*menufont);

	actmenuFont = new QAction("Menu Font", this);
	setupMenu->addAction(actmenuFont);
	connect(actmenuFont, SIGNAL(triggered()), this, SLOT(doMFonts()));
	actmenuFont->setFont(*menufont);

	actColours = new QAction("Choose Colours", this);
	setupMenu->addAction(actColours);
	connect(actColours, SIGNAL(triggered()), this, SLOT(doColours()));
	actColours->setFont(*menufont);

	setupMenu->addSeparator();

	AGWAction = new QAction("AGW Setup", this);
	setupMenu->addAction(AGWAction);
	connect(AGWAction, SIGNAL(triggered()), this, SLOT(AGWSlot()));
	AGWAction->setFont(*menufont);

	VARAAction = new QAction("VARA Setup", this);
	setupMenu->addAction(VARAAction);
	connect(VARAAction, SIGNAL(triggered()), this, SLOT(VARASlot()));
	VARAAction->setFont(*menufont);

	KISSAction = new QAction("KISS Setup", this);
	setupMenu->addAction(KISSAction);
	connect(KISSAction, SIGNAL(triggered()), this, SLOT(KISSSlot()));
	KISSAction->setFont(*menufont);

	setupMenu->addSeparator();

	actChatMode = setupMenuLine(setupMenu, (char *)"Chat Terminal Mode (Send Keepalives)", this, ChatMode);
	actAutoTeletext = setupMenuLine(setupMenu, (char *)"Auto switch to Teletext", this, AutoTeletext);
	actStripLF = setupMenuLine(setupMenu, (char *)"Strip Line Feeds", this, StripLF);

	setupMenu->addSeparator();

	setupMenu->addAction(new QAction("Interpret non-UTF8 input as:", this));
	setupMenu->setFont(*menufont);

	QActionGroup * cpGroup = new QActionGroup(this);

	actnoConv = setupMenuLine(nullptr, (char *)"   Don't Convert (assume is UTF-8)", this, convUTF8 == -1);
	actAuto = setupMenuLine(nullptr, (char *)"   Auto", this, convUTF8 == 0);
	actCP1251 = setupMenuLine(nullptr, (char *)"   CP1251 (Cyrillic)", this, convUTF8 == 1251);
	actCP1252 = setupMenuLine(nullptr, (char *)"   CP1252 (Western Europe)", this, convUTF8 == 1252);
	actCP437 = setupMenuLine(nullptr, (char *)"   CP437 (Windows Line Draw)", this, convUTF8 == 437);

	cpGroup->addAction(actnoConv);
	cpGroup->addAction(actAuto);
	cpGroup->addAction(actCP1251);
	cpGroup->addAction(actCP1252);
	cpGroup->addAction(actCP437);

	setupMenu->addAction(actnoConv);
	setupMenu->addAction(actAuto);
	setupMenu->addAction(actCP1251);
	setupMenu->addAction(actCP1252);
	setupMenu->addAction(actCP437);

	monitorMenu = mymenuBar->addMenu(tr("&Monitor"));

	EnableMonitor = setupMenuLine(monitorMenu, (char *)"Enable Monitoring", this, 0);
	EnableMonitor->setVisible(0);
	EnableMonLog = setupMenuLine(monitorMenu, (char *)"Log to File", this, 0);
	MonLocalTime = setupMenuLine(monitorMenu, (char *)"Use local time", this, 0);
	MonTX = setupMenuLine(monitorMenu, (char *)"Monitor TX", this, 1);
	MonSup = setupMenuLine(monitorMenu, (char *)"Monitor Supervisory", this, 1);
	MonUI = setupMenuLine(monitorMenu, (char *)"Only Monitor UI Frames", this, 0);
	MonNodes = setupMenuLine(monitorMenu, (char *)"Monitor NODES Broadcasts", this, 0);
	MonColour = setupMenuLine(monitorMenu, (char *)"Enable Colour", this, 1);

	for (i = 0; i < MAXPORTS + 1; i++)
	{
		MonPort[i] = setupMenuLine(monitorMenu, (char *)"Port", this, 0);
		MonPort[i]->setVisible(false);
	}

	but1 = new QToolButton();
	but1->setPopupMode(QToolButton::InstantPopup);
	but1->setText("Monitor");
	but1->addAction(monitorMenu->menuAction());
	toolBar->addWidget(but1);

	but2 = new QToolButton();
	but2->setPopupMode(QToolButton::InstantPopup);
	but2->setText("Setup");
	but2->addAction(setupMenu->menuAction());
	toolBar->addWidget(but2);

	ListenAction = mymenuBar->addAction("&Listen");
	connect(ListenAction, SIGNAL(triggered()), this, SLOT(ListenSlot()));


	// Place Listen Action in mac app menu, otherwise it doesn't appear
	if (is_mac == true)
	{
		QMenu * app_menu = mymenuBar->addMenu("App Menu");
		ListenAction->setMenuRole(QAction::ApplicationSpecificRole);
		app_menu->addAction(ListenAction);
	}

	toolBar->addAction(ListenAction);

	YAPPMenu = mymenuBar->addMenu(tr("&YAPP"));

	YAPPSend = new QAction("Send File", this);
	YAPPSetRX = new QAction("Set Receive Directory", this);
	YAPPSetSize = new QAction("Set Max Size", this);
	YAPPSend->setFont(*menufont);
	YAPPSetRX->setFont(*menufont);
	YAPPSetSize->setFont(*menufont);

	YAPPMenu->addAction(YAPPSend);
	YAPPMenu->addAction(YAPPSetRX);
	YAPPMenu->addAction(YAPPSetSize);
	YAPPSend->setEnabled(false);

	connect(YAPPSend, SIGNAL(triggered()), this, SLOT(doYAPPSend()));
	connect(YAPPSetRX, SIGNAL(triggered()), this, SLOT(doYAPPSetRX()));
	connect(YAPPSetSize, SIGNAL(triggered()), this, SLOT(doYAPPSetSize()));

	but3 = new QToolButton();
	but3->setPopupMode(QToolButton::InstantPopup);
	but3->setText("YAPP");
	but3->addAction(YAPPMenu->menuAction());
	but3->setFont(*menufont);
	toolBar->addWidget(but3);

	toolBar->setFont(*menufont);

	// Set up Status Bar

	setStyleSheet("QStatusBar{border-top: 1px outset black;} QStatusBar::item { border: 1px solid black; border-radius: 3px;}");
	//	setStyleSheet("QStatusBar{border-top: 1px outset black;}");


	Status4 = new QLabel("");
	Status3 = new QLabel("         ");
	Status2 = new QLabel("    ");
	Status1 = new QLabel("  Disconnected  ");

	Status1->setMinimumWidth(100);
	Status2->setMinimumWidth(100);
	Status3->setMinimumWidth(100);
	Status4->setMinimumWidth(100);

	myStatusBar = statusBar();

	statusBar()->addPermanentWidget(Status4);
	statusBar()->addPermanentWidget(Status3);
	statusBar()->addPermanentWidget(Status2);
	statusBar()->addPermanentWidget(Status1);

	statusBar()->setVisible(AGWEnable | VARAEnable | KISSEnable);
	// Restore saved sessions

	if (TermMode == Single)
	{
		Ui_ListenSession * Sess = newWindow(this, singlemodeFormat);

		ActiveSession = Sess;
		Sess->CurrentHost = currentHost[0];

		ui.verticalLayout->addWidget(Sess);

		connectMenu->setEnabled(true);
		discAction->setEnabled(false);
		YAPPSend->setEnabled(false);
	}

	if (TermMode == MDI)
	{
		int n = atoi(sessionList);
		int index = 0;

		char *p, *Context;
		char delim[] = "|";

		int type = 0, host = 0, topx, leftx, bottomx, rightx;

		p = strtok_s((char *)sessionList, delim, &Context);

		while (n--)
		{
			p = strtok_s(NULL, delim, &Context);

			sscanf(p, "%d,%d,%d,%d,%d,%d", &type, &host, &topx, &leftx, &bottomx, &rightx);

			Ui_ListenSession * Sess = newWindow(this, type);

			Sess->CurrentHost = currentHost[index++];;

			QRect r(leftx, topx, rightx - leftx, bottomx - topx);

			Sess->sw->setGeometry(r);
			Sess->sw->move(leftx, topx);

		}
	}

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(MyTimerSlot()));
	timer->start(10000);

	QTimer *timer2 = new QTimer(this);
	connect(timer2, SIGNAL(timeout()), this, SLOT(KISSTimerSlot()));
	timer2->start(100);

	// Run timer now to connect to AGW if configured

	MyTimerSlot();

	if (listenEnable)
		_server->listen(QHostAddress::Any, listenPort);

	connect(_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));

	setFonts();

	if (VARAEnable)
		OpenPTTPort();

	memset(axMYCALL, 0, 7);
	ConvToAX25(MYCALL, axMYCALL);

	// Do any autoconnects

	for (int i = 0; i < _sessions.size(); ++i)
	{
		if (AutoConnect[i] > 0)
		{
			Ui_ListenSession * Sess = _sessions.at(i);
			
			Sess->clientSocket = new myTcpSocket();
			Sess->clientSocket->Sess = Sess;

			connect(Sess->clientSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
			connect(Sess->clientSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
			connect(Sess->clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));

			Sess->clientSocket->connectToHost(&Host[Sess->CurrentHost][0], Port[Sess->CurrentHost]);
		}
	}

}

void QtTermTCP::showContextMenu(const QPoint &point)
{
	if (point.isNull())
		return;

	QTabBar* tabBar = tabWidget->tabBar();
	QRect Wrect = tabWidget->rect();
	QRect Brect = tabBar->rect();
	QPoint myPoint = point;

	// Get x coordinate of first tab (on Mac tabs are centre aligned)

	QRect rect = tabWidget->tabBar()->geometry();
	int left = rect.left();


	int n = myPoint.y() - (Wrect.height() - Brect.height());
	myPoint.setY(n); 
	n = myPoint.x() - left;

	myPoint.setX(n);

	int tabIndex =  tabBar->tabAt(myPoint);

	QMenu menu(this);

	QAction * Act = new QAction("AutoConnect on load", this);
	Act->setObjectName(QString::number(tabIndex));

	menu.addAction(Act);

	Act->setCheckable(true);

	if (AutoConnect[tabIndex] <= 0)
		Act->setChecked(false);
	else
		Act->setChecked(true);

	connect(Act, SIGNAL(triggered()), this, SLOT(autoConnectChecked()));

	menu.exec(tabWidget->mapToGlobal(point));

}

void QtTermTCP::autoConnectChecked()
{
	QAction * Act = static_cast<QAction*>(QObject::sender());
	QString Tab = Act->objectName();
	int tabNo = Tab.toInt();

	tabWidget->setCurrentIndex(tabNo);
	
	Ui_ListenSession *Sess = (Ui_ListenSession *)tabWidget->currentWidget();
	int state = Act->isChecked();

	if (state == 0)
		AutoConnect[tabNo] = 0;
	else
	{
		if (Sess->clientSocket)		// Connected
			AutoConnect[tabNo] = 1;
		else
			AutoConnect[tabNo] = 0;
	}
}


void QtTermTCP::setFonts()
{
	windowMenu->menuAction()->setFont(*menufont);
	connectMenu->menuAction()->setFont(*menufont);
	setupMenu->menuAction()->setFont(*menufont);
	monitorMenu->menuAction()->setFont(*menufont);
	YAPPMenu->menuAction()->setFont(*menufont);

	if (tabWidget)
		tabWidget->setFont(*menufont);
	mymenuBar->setFont(*menufont);
	toolBar->setFont(*menufont);
	but1->setFont(*menufont);
	but2->setFont(*menufont);
	but3->setFont(*menufont);
	but4->setFont(*menufont);
	but5->setFont(*menufont);
	discAction->setFont(*menufont);
	ListenAction->setFont(*menufont);

	for (int i = 0; i < MAXHOSTS; i++)
	{
		actHost[i]->setFont(*menufont);
		actSetup[i]->setFont(*menufont);
	}

	actHost[16]->setFont(*menufont);			// AGW  Host
}

void QtTermTCP::doQuit()
{
	if (_server->isListening())
		_server->close();

	SaveSettings();

	QCoreApplication::quit();
}

void FlashifNotActive()
{
	if (!mythis->isActiveWindow())
		QApplication::alert(mythis, 0);
}


// "Copy on select" Code


void QtTermTCP::onTEselectionChanged()
{
	QTextEdit * x = static_cast<QTextEdit*>(QObject::sender());

	if (isActiveWindow())
		x->copy();
}

void QtTermTCP::onLEselectionChanged()
{
	QLineEdit * x = static_cast<QLineEdit*>(QObject::sender());
	if (isActiveWindow())
		x->copy();
}

void QtTermTCP::setVDMode()
{
	QAction * sender = static_cast<QAction*>(QObject::sender());

	QTextEdit * window = static_cast<QTextEdit*>(sender->parentWidget());

	// Need to find session with this window as owner

	Ui_ListenSession * Sess = NULL;


	for (int i = 0; i < _sessions.size(); ++i)
	{
		Sess = _sessions.at(i);

		if (Sess->termWindow == window)
		{
			Sess->TTActive ^= 1;
			DoTermResize(Sess);
			break;
		}
	}
}

int splitY;					// Value when menu added

void QtTermTCP::setSplit()
{
	QAction * sender = static_cast<QAction*>(QObject::sender());

	QWidget * Parent = sender->parentWidget();

	if (Parent)
		Parent = Parent->parentWidget();

	int y = Parent->rect().height() - 50;

	// y is height of whole windom. splitX is new split position
	// So split = 100 * splitx/x

	Split = (splitY * 100) / y;

	if (Split < 10)
		Split = 10;
	else if (Split > 90)
		Split = 90;

	QSize size(800, 602);
	QResizeEvent event(size, size);

	eventFilter(Parent, &event);
}

void QtTermTCP::ClearScreen()
{
	QAction * sender = static_cast<QAction*>(QObject::sender());
	QTextEdit * window = static_cast<QTextEdit*>(sender->parentWidget());
	window->clear();
}


void QtTermTCP::showContextMenuMT(const QPoint &pt)				// Term Window
{
	// Monitor and Terminal (Term Half)

	QTextEdit* sender = static_cast<QTextEdit*>(QObject::sender());

	QMenu *menu = sender->createStandardContextMenu();

	QString style = "QMenu {border-radius:15px; background-color: white;margin: 2px; border: 1px solid rgb(58, 80, 116); color:  rgb(58, 80, 116);}QMenu::separator {height: 2px;background: rgb(58, 80, 116);margin-left: 10px;margin-right: 5px;}";
	menu->setStyleSheet(style);

	QAction * actSplit = new QAction("Set Monitor/Output Split", sender);
	QAction * actVDMode = new QAction("Toggle Viewdata Mode", sender);
	QAction * actClear = new QAction("Clear Screen Buffer", sender);

	menu->addAction(actSplit);
	menu->addAction(actVDMode);
	menu->addAction(actClear);

	splitY = pt.y() + termX;

	connect(actSplit, SIGNAL(triggered()), this, SLOT(setSplit()));
	connect(actVDMode, SIGNAL(triggered()), this, SLOT(setVDMode()));
	connect(actClear, SIGNAL(triggered()), this, SLOT(ClearScreen()));

	menu->exec(sender->mapToGlobal(pt));
	delete menu;
}

void QtTermTCP::showContextMenuMOnly(const QPoint &pt)
{
	// Monitor only

	QTextEdit* sender = static_cast<QTextEdit*>(QObject::sender());

	QMenu *menu = sender->createStandardContextMenu();

	QString style = "QMenu {border-radius:15px; background-color: white;margin: 2px; border: 1px solid rgb(58, 80, 116); color:  rgb(58, 80, 116);}QMenu::separator {height: 2px;background: rgb(58, 80, 116);margin-left: 10px;margin-right: 5px;}";
	menu->setStyleSheet(style);

	QAction * actClear = new QAction("Clear Screen Buffer", sender);

	menu->addAction(actClear);
	connect(actClear, SIGNAL(triggered()), this, SLOT(ClearScreen()));

	menu->exec(sender->mapToGlobal(pt));
	delete menu;
}


void QtTermTCP::showContextMenuT(const QPoint &pt)				// Term Window
{
	// Just Terminal 

	QTextEdit* sender = static_cast<QTextEdit*>(QObject::sender());

	QMenu *menu = sender->createStandardContextMenu();

	QString style = "QMenu {border-radius:15px; background-color: white;margin: 2px; border: 1px solid rgb(58, 80, 116); color:  rgb(58, 80, 116);}";
	menu->setStyleSheet(style);


	QAction * actVDMode = new QAction("Toggle Viewdata Mode", sender);
	QAction * actClear = new QAction("Clear Screen Buffer", sender);

	menu->addAction(actVDMode);
	menu->addAction(actClear);

	connect(actVDMode, SIGNAL(triggered()), this, SLOT(setVDMode()));
	connect(actClear, SIGNAL(triggered()), this, SLOT(ClearScreen()));

	menu->exec(sender->mapToGlobal(pt));
	delete menu;
}


void QtTermTCP::showContextMenuL()				// Term Window
{
	// Teletext Label Right Clicked - cancel TT Mode

	QLabel * sender = static_cast<QLabel*>(QObject::sender());

	// Need to find session with this label as owner

	Ui_ListenSession * Sess = NULL;

	for (int i = 0; i < _sessions.size(); ++i)
	{
		Sess = _sessions.at(i);

		if (Sess->TTLabel == sender)
		{
			Sess->TTActive ^= 1;

			DoTermResize(Sess);
			break;
		}
	}
}


void QtTermTCP::showContextMenuM(const QPoint &pt)				// Mon Window
{
	QTextEdit* sender = static_cast<QTextEdit*>(QObject::sender());

	QMenu *menu = sender->createStandardContextMenu();
	QString style = "QMenu {border-radius:15px; background-color: white;margin: 2px; border: 1px solid rgb(58, 80, 116); color:  rgb(58, 80, 116);}";
	menu->setStyleSheet(style);

	QAction * actSplit = new QAction("Set Monitor/Output Split", sender);
	QAction * actClear = new QAction("Clear Screen Buffer", sender);

	menu->addAction(actSplit);
	menu->addAction(actClear);

	splitY = pt.y();

	connect(actSplit, SIGNAL(triggered()), this, SLOT(setSplit()));
	connect(actClear, SIGNAL(triggered()), this, SLOT(ClearScreen()));

	menu->exec(sender->mapToGlobal(pt));
	delete menu;
}

extern "C" void setMenus(int State)
{
	// Sets Connect, Disconnect and YAPP Send  enable flags to match connection state

	if (State)
	{
		connectMenu->setEnabled(false);
		discAction->setEnabled(true);
		YAPPSend->setEnabled(true);
	}
	else
	{
		connectMenu->setEnabled(true);
		discAction->setEnabled(false);
		YAPPSend->setEnabled(false);
	}
}

extern "C" void SetSessLabel(Ui_ListenSession * Sess, char * label)
{
	if (TermMode == MDI)
		Sess->setWindowTitle(label);
	else if (TermMode == Tabbed)
		tabWidget->setTabText(Sess->Tab, label);
	else if (TermMode == Single)
		mythis->setWindowTitle(label);
}


extern "C" void ClearSessLabel(Ui_ListenSession * Sess)
{
	if (TermMode == MDI)
	{
		if (Sess->SessionType == Mon)			// Mon Only
			Sess->setWindowTitle("Monitor Session Disconnected");
		else if (Sess->SessionType == Listen)			// Mon Only
			Sess->setWindowTitle("Listen Window Disconnected");
		else
			Sess->setWindowTitle("Disconnected");
	}
	else if (TermMode == Tabbed)
	{
		if (Sess->SessionType == Mon)			// Mon Only
			tabWidget->setTabText(Sess->Tab, "Monitor");
		else
		{
			char Label[16];
			sprintf(Label, "Sess %d", Sess->Tab + 1);
			tabWidget->setTabText(Sess->Tab, Label);
		}
	}
	else if (TermMode == Single)
	{
		if (Sess->SessionType == Mon)			// Mon Only
			mythis->setWindowTitle("Monitor Session Disconnected");
		else
			mythis->setWindowTitle("Disconnected");
	}
}

void QtTermTCP::tabSelected(int Current)
{
	Ui_ListenSession * Sess = NULL;

	if (Current < _sessions.size())
	{
		Sess = _sessions.at(Current);

		if (Sess == nullptr)
			return;

		ActiveSession = Sess;

		tabWidget->tabBar()->setTabTextColor(tabWidget->currentIndex(), oldTabText);

		if (Sess->clientSocket || Sess->AGWSession || Sess->KISSSession || Sess->KISSMode)
		{
			connectMenu->setEnabled(false);
			discAction->setEnabled(true);
			YAPPSend->setEnabled(true);
		}
		else
		{
			connectMenu->setEnabled(true);
			discAction->setEnabled(false);
			YAPPSend->setEnabled(false);
		}

		// If a monitor Window, change Monitor config settings

		EnableMonLog->setChecked(Sess->LogMonitor);

		if (AGWUsers && Sess == AGWUsers->MonSess)		// AGW Monitor
		{
			for (int i = 0; i < 64; i++)
				SetPortMonLine(i, (char *)"", 0, 0);			// Set all hidden

			connectMenu->setEnabled(false);
			MonTX->setVisible(0);
			MonSup->setVisible(0);
			MonUI->setVisible(0);
			MonColour->setVisible(0);

			EnableMonitor->setVisible(1);
			EnableMonitor->setChecked(AGWMonEnable);
			MonLocalTime->setChecked(Sess->mlocaltime);
			MonNodes->setChecked(Sess->MonitorNODES);
		}
		else if (Sess == KISSMonSess)				// KISS Monitor
		{
			for (int i = 0; i < 64; i++)
				SetPortMonLine(i, (char *)"", 0, 0);			// Set all hidden

			connectMenu->setEnabled(false);
			MonTX->setVisible(0);
			MonSup->setVisible(0);
			MonUI->setVisible(0);
			MonColour->setVisible(0);

			EnableMonitor->setVisible(1);
			EnableMonitor->setChecked(KISSMonEnable);
			MonLocalTime->setChecked(Sess->mlocaltime);
			MonNodes->setChecked(Sess->MonitorNODES);
		}
		else
		{
			EnableMonitor->setVisible(0);
			MonTX->setVisible(1);
			MonSup->setVisible(1);
			MonUI->setVisible(1);
			MonColour->setVisible(1);
		}

		if (Sess->PortMonString[0])
		{
			char * ptr = (char *)malloc(2048);
			memcpy(ptr, Sess->PortMonString, 2048);

			int NumberofPorts = atoi((char *)&ptr[2]);
			char *p, *Context;
			char msg[80];
			int portnum, m;
			char delim[] = "|";

			// Remove old Monitor menu

			for (int i = 0; i < 64; i++)
			{
				SetPortMonLine(i, (char *)"", 0, 0);			// Set all hidden
			}

			p = strtok_s((char *)&ptr[2], delim, &Context);

			while (NumberofPorts--)
			{
				p = strtok_s(NULL, delim, &Context);
				if (p == NULL)
					break;

				m = portnum = atoi(p);

				sprintf(msg, "Port %s", p);

				if (m == 0)
					m = 64;

				if (Sess->portmask & ((uint64_t)1 << (m - 1)))
					SetPortMonLine(portnum, msg, 1, 1);
				else
					SetPortMonLine(portnum, msg, 1, 0);
			}
			free(ptr);

			MonLocalTime->setChecked(Sess->mlocaltime);
			MonTX->setChecked(Sess->mtxparam);
			MonSup->setChecked(Sess->mcomparam);
			MonUI->setChecked(Sess->monUI);
			MonNodes->setChecked(Sess->MonitorNODES);
			MonColour->setChecked(Sess->MonitorColour);
		}
		return;
	}
}

void QtTermTCP::SetupHosts()
{
	QAction * Act = static_cast<QAction*>(QObject::sender());
	int i;

	for (i = 0; i < MAXHOSTS; i++)
	{
		if (Act == actSetup[i])
			break;
	}

	if (i > 15)
		return;

	ConfigHost = i;

	TabDialog tabdialog(0);
	tabdialog.exec();
}


void QtTermTCP::Connect()
{
	QMdiSubWindow * UI;
	Ui_ListenSession * Sess = nullptr;
	QAction * Act = static_cast<QAction*>(QObject::sender());

	int i;

	for (i = 0; i < MAXHOSTS; i++)
	{
		if (Act == actHost[i])
			break;
	}

	SavedHost = i;				// Last used

	if (TermMode == MDI)
	{
		UI = mdiArea->activeSubWindow();

		for (i = 0; i < _sessions.size(); ++i)
		{
			Sess = _sessions.at(i);

			if (Sess->sw == UI)
				break;
		}

		if (i == _sessions.size())
			return;

		currentHost[i] = SavedHost;
	}

	else if (TermMode == Tabbed)
	{
		Sess = (Ui_ListenSession *)tabWidget->currentWidget();
	}

	else if (TermMode == Single)
		Sess = _sessions.at(0);

	if ((Sess == nullptr || Sess->SessionType & Listen))
		return;

	Sess->CurrentHost = SavedHost;

	currentHost[Sess->Tab] = SavedHost;

	if (Act == actHost[16])
	{
		// This runs the AGW Connection dialog

		Sess->CurrentHost = 16;				// Iast used
		AGWConnect dialog(0);
		dialog.exec();
		return;
	}


	if (Act == actHost[17])
	{
		// This runs the VARA Connection dialog

		Sess->CurrentHost = 17;				// Iast used
		VARADataSock->Sess = Sess;
		VARASock->Sess = Sess;

		VARAConnect dialog(0);
		dialog.exec();
		WritetoOutputWindow(Sess, (unsigned char *)"Connecting...\r", 14);

		return;
	}

	if (Act == actHost[18])
	{
		// This runs the KISS Connection dialog

		Sess->CurrentHost = 18;				// Iast used

		KISSConnect dialog(0);
		dialog.exec();
		return;
	}

	// Set Monitor Params for this host

	sscanf(MonParams[Sess->CurrentHost], "%llx %x %x %x %x %x",
		&Sess->portmask, &Sess->mtxparam, &Sess->mcomparam,
		&Sess->MonitorNODES, &Sess->MonitorColour, &Sess->monUI);

	Sess->mlocaltime = (Sess->mtxparam >> 7);
	Sess->mtxparam &= 1;

	MonLocalTime->setChecked(Sess->mlocaltime);
	MonTX->setChecked(Sess->mtxparam);
	MonSup->setChecked(Sess->mcomparam);
	MonUI->setChecked(Sess->monUI);
	MonNodes->setChecked(Sess->MonitorNODES);
	MonColour->setChecked(Sess->MonitorColour);

	// Remove old Monitor menu

	for (i = 0; i < 64; i++)
	{
		SetPortMonLine(i, (char *)"", 0, 0);			// Set all hidden
	}

	delete(Sess->clientSocket);

	Sess->clientSocket = new myTcpSocket();
	Sess->clientSocket->Sess = Sess;

	connect(Sess->clientSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
	connect(Sess->clientSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
	connect(Sess->clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));

	Sess->clientSocket->connectToHost(&Host[Sess->CurrentHost][0], Port[Sess->CurrentHost]);
	return;
}

extern "C" void rst_timer(TAX25Port * AX25Sess);
extern "C" void set_unlink(TAX25Port * AX25Sess, Byte * path);

void QtTermTCP::Disconnect()
{
	QMdiSubWindow * UI;
	Ui_ListenSession * Sess = nullptr;

	if (TermMode == MDI)
	{
		int i;

		UI = mdiArea->activeSubWindow();

		for (i = 0; i < _sessions.size(); ++i)
		{
			Sess = _sessions.at(i);

			if (Sess->sw == UI)
				break;
		}

		if (i == _sessions.size())
			return;
	}

	else if (TermMode == Tabbed)
	{
		Sess = (Ui_ListenSession *)tabWidget->currentWidget();
	}

	else if (TermMode == Single)
		Sess = _sessions.at(0);

	// if AGW send a d frame else disconnect TCP session

	if (Sess->AGWSession)
		Send_AGW_Ds_Frame(Sess->AGWSession);
	else if (VARASock && VARASock->Sess == Sess)
		VARASock->write("DISCONNECT\r");
	else if (Sess->KISSSession)
	{
		if (Sess->KISSMode == 0)
		{
			TAX25Port * Port = (TAX25Port *)Sess->KISSSession;

			rst_timer(Port);
			set_unlink(Port, Port->Path);
		}
		else
		{
			// KISS UI Mode

			char Msg[128];
			int Len = sprintf(Msg, "Disconnected\r");

			Sess->KISSMode = 0;
			SendtoTerm(Sess, Msg, Len);
			ClearSessLabel(Sess);
			setMenus(0);
			Sess->KISSSession = NULL;
		}
	}
	else

	{
		if (Sess && Sess->clientSocket)
			Sess->clientSocket->disconnectFromHost();
	}
	return;
}


void QtTermTCP::doYAPPSend()
{
	QFileDialog dialog(this);
	QStringList fileNames;
	dialog.setFileMode(QFileDialog::AnyFile);

	QMdiSubWindow * UI;
	Ui_ListenSession * Sess = nullptr;
	int i;

	if (TermMode == MDI)
	{
		UI = mdiArea->activeSubWindow();

		for (i = 0; i < _sessions.size(); ++i)
		{
			Sess = _sessions.at(i);

			if (Sess->sw == UI)
				break;
		}

		if (i == _sessions.size())
			return;
	}

	else if (TermMode == Tabbed)
	{
		Sess = (Ui_ListenSession *)tabWidget->currentWidget();
	}

	else if (TermMode == Single)
		Sess = _sessions.at(0);

	if (Sess == nullptr)
		return;

	if ((Sess->SessionType & Mon))
	{
		// Turn off monitoring

		setTraceOff(Sess);
	}

	if (dialog.exec())
	{
		char FN[256];

		fileNames = dialog.selectedFiles();
		if (fileNames[0].length() < 256)
		{
			strcpy(FN, fileNames[0].toUtf8());
			YAPPSendFile(Sess, FN);
		}
	}
}

void QtTermTCP::doYAPPSetRX()
{
	QFileDialog dialog(this);
	QStringList fileNames;
	dialog.setFileMode(QFileDialog::Directory);
	dialog.setDirectory(YAPPPath);

	if (dialog.exec())
	{
		fileNames = dialog.selectedFiles();
		if (fileNames[0].length() < 256)
			strcpy(YAPPPath, fileNames[0].toUtf8());

		SaveSettings();
	}
}

Ui_SizeDialog * YappSize;

void QtTermTCP::doYAPPSetSize()
{
	// This runs the VARA Configuration dialog

	char valChar[80];

	YappSize = new(Ui_SizeDialog);

	QDialog UI;

	YappSize->setupUi(&UI);

	UI.setFont(*menufont);
	deviceUI = &UI;

	sprintf(valChar, "%d", MaxRXSize);

	YappSize->maxSize->setText(valChar);

	QObject::connect(YappSize->okButton, SIGNAL(clicked()), this, SLOT(sizeaccept()));
	QObject::connect(YappSize->cancelButton, SIGNAL(clicked()), this, SLOT(sizereject()));

	UI.exec();
}

void QtTermTCP::sizeaccept()
{
	QVariant Q;
	Q = YappSize->maxSize->text();

	MaxRXSize = Q.toInt();
	SaveSettings();
	delete(YappSize);
}


void QtTermTCP::sizereject()
{
	delete(YappSize);
	deviceUI->reject();
}


void QtTermTCP::menuChecked()
{
	QAction * Act = static_cast<QAction*>(QObject::sender());

	int state = Act->isChecked();
	int newTermMode = TermMode;
	int newSingleMode = singlemodeFormat;

	if (Act == TabSingle || Act == TabBoth || Act == TabMon)
	{
		// Tabbed Window format had changed

		int i = tabWidget->currentIndex();

		if (Act == TabSingle)
			TabType[i] = Term;
		else if (Act == TabBoth)
			TabType[i] = Term + Mon;
		else
			TabType[i] = Mon;

		QMessageBox msgBox;
		msgBox.setText("Tab Types will change next time program is started.");
		msgBox.exec();

	}

	if (Act == singleAct || Act == singleAct2 || Act == MDIAct || Act == tabbedAct)
	{
		// Term Mode had changed

		if (singleAct->isChecked())
		{
			newTermMode = Single;
			newSingleMode = Mon + Term;
		}
		else if (singleAct2->isChecked())
		{
			newTermMode = Single;
			newSingleMode = Term;
		}
		else if (MDIAct->isChecked())
			newTermMode = MDI;
		else if (tabbedAct->isChecked())
			newTermMode = Tabbed;

		if (newTermMode != TermMode || newSingleMode != singlemodeFormat)
		{
			QSettings settings(GetConfPath(), QSettings::IniFormat);
			settings.setValue("TermMode", newTermMode);
			settings.setValue("singlemodeFormat", newSingleMode);
			QMessageBox msgBox;
			msgBox.setText("Presentation Mode will change next time program is started.");
			msgBox.exec();
		}

		return;
	}

	if (Act == EnableMonitor)
		ActiveSession->EnableMonitor = state;
	else if (Act == EnableMonLog)
		ActiveSession->LogMonitor = state;
	else if (Act == MonLocalTime)
		ActiveSession->mlocaltime = state;
	else if (Act == MonTX)
		ActiveSession->mtxparam = state;
	else if (Act == MonSup)
		ActiveSession->mcomparam = state;
	else if (Act == MonUI)
		ActiveSession->monUI = state;
	else if (Act == MonNodes)
		ActiveSession->MonitorNODES = state;
	else if (Act == MonColour)
		ActiveSession->MonitorColour = state;
	else if (Act == actChatMode)
		ChatMode = state;
	else if (Act == actAutoTeletext)
		AutoTeletext = state;
	else if (Act == actBells)
		Bells = state;
	else if (Act == actStripLF)
		StripLF = state;
	else if (Act == actIntervalBeep)
		AlertBeep = state;
	else if (Act == actConnectBeep)
		ConnectBeep = state;
	else if (Act == actnoConv)
		convUTF8 = -1;
	else if (Act == actAuto)
		convUTF8 = 0;
	else if (Act == actCP1251)
		convUTF8 = 1251;
	else if (Act == actCP1252)
		convUTF8 = 1252;
	else if (Act == actCP437)
		convUTF8 = 437;
	else
	{
		// look for port entry
		for (int i = 0; i < MAXPORTS + 1; i++)
		{
			if (Act == MonPort[i])
			{
				uint64_t mmask;

				if (i == 0)							// BBS Mon - use bit 63 (Port 64)
					mmask = (uint64_t)1 << 63;
				else
					mmask = (uint64_t)1 << (i - 1);

				if (state)
					ActiveSession->portmask |= mmask;
				else
					ActiveSession->portmask &= ~mmask;
				break;
			}
		}
	}


	if (ActiveSession->clientSocket && ActiveSession->SessionType & Mon)
		SendTraceOptions(ActiveSession);

	else if (AGWUsers && ActiveSession == AGWUsers->MonSess)
	{
		AGWMonEnable = ActiveSession->EnableMonitor;
		AGWLocalTime = ActiveSession->mlocaltime;
		AGWMonNodes = ActiveSession->MonitorNODES;
		Send_AGW_m_Frame((QTcpSocket*)ActiveSession->AGWSession);
	}

	else if (ActiveSession == KISSMonSess)
	{
		KISSLocalTime = ActiveSession->mlocaltime; 
		KISSMonEnable = ActiveSession->EnableMonitor;
		KISSMonNodes = ActiveSession->MonitorNODES;
	}
	return;
}




void QtTermTCP::LDisconnect(Ui_ListenSession * LUI)
{
	if (LUI->clientSocket)
		LUI->clientSocket->disconnectFromHost();
}

extern QApplication * a;

void QtTermTCP::LreturnPressed(Ui_ListenSession * Sess)
{
	QByteArray stringData = Sess->inputWindow->text().toUtf8();

	// if multiline input (eg from copy/paste) replace LF with CR

	char * ptr;
	char * Msgptr;
	char * Msg;

	QScrollBar *scrollbar = Sess->termWindow->verticalScrollBar();
	bool scrollbarAtBottom = (scrollbar->value() >= (scrollbar->maximum() - 4));

	if (scrollbarAtBottom)
		Sess->termWindow->moveCursor(QTextCursor::End);					// So we don't get blank lines

	// Stack it

	Sess->StackIndex = 0;

	if (Sess->KbdStack[49])
		free(Sess->KbdStack[49]);

	for (int i = 48; i >= 0; i--)
	{
		Sess->KbdStack[i + 1] = Sess->KbdStack[i];
	}

	Sess->KbdStack[0] = qstrdup(stringData.data());

	stringData.append('\n');

	Msgptr = stringData.data();

	if (Sess->TTActive)					// Teletext uses 0x5F for #
		while ((ptr = strchr(Msgptr, 0x23)))	// Replace VT with LF
			*ptr++ = 0x5f;


	while ((ptr = strchr(Msgptr, 11)))	// Replace VT with LF
		*ptr++ = 10;

	LastWrite = time(NULL);				// Stop initial beep
	Sess->SlowTimer = 0;

	Sess->pageBuffer[0] = 0;			// Reset Teletext mode screen buffer

	if (Sess->KISSMode == 1)
	{
		// UI session. Send as UI Message

		while ((ptr = strchr(Msgptr, '\n')))
		{
			*ptr++ = 0;

			Msg = (char *)malloc(strlen(Msgptr) + 10);
			strcpy(Msg, Msgptr);
			strcat(Msg, "\r");

			Send_UI(0, 0xF0, MYCALL, Sess->UIDEST, (unsigned char *)Msg, (int)strlen(Msg));

			WritetoOutputWindowEx(Sess, (unsigned char *)Msg, (int)strlen(Msg),
				Sess->termWindow, &Sess->OutputSaveLen, Sess->OutputSave, EchoText);		// Black

			free(Msg);

			Msgptr = ptr;
		}
	}
	else if (Sess->KISSSession)
	{
		// Send to ax.25 code

		while ((ptr = strchr(Msgptr, '\n')))
		{
			*ptr++ = 0;

			Msg = (char *)malloc(strlen(Msgptr) + 10);
			strcpy(Msg, Msgptr);
			strcat(Msg, "\r");

			SendtoAX25(Sess->KISSSession, (unsigned char *)Msg, (int)strlen(Msg));

			WritetoOutputWindowEx(Sess, (unsigned char *)Msg, (int)strlen(Msg),
				Sess->termWindow, &Sess->OutputSaveLen, Sess->OutputSave, EchoText);		// Black

			free(Msg);

			Msgptr = ptr;
		}
	}

	else if (Sess->AGWSession)
	{
		// Terminal is in AGWPE mode - send as AGW frame

		while ((ptr = strchr(Msgptr, '\n')))
		{
			*ptr++ = 0;

			Msg = (char *)malloc(strlen(Msgptr) + 10);
			strcpy(Msg, Msgptr);
			strcat(Msg, "\r");

			AGW_AX25_data_in(Sess->AGWSession, (unsigned char *)Msg, (int)strlen(Msg));

			WritetoOutputWindowEx(Sess, (unsigned char *)Msg, (int)strlen(Msg),
				Sess->termWindow, &Sess->OutputSaveLen, Sess->OutputSave, EchoText);		// Black

//			Sess->termWindow->insertPlainText(Msg);

			free(Msg);

			Msgptr = ptr;
		}
	}

	else if (VARASock && VARASock->Sess == Sess)
	{
		while ((ptr = strchr(Msgptr, '\n')))
		{
			*ptr++ = 0;

			Msg = (char *)malloc(strlen(Msgptr) + 10);
			strcpy(Msg, Msgptr);
			strcat(Msg, "\r");

			VARADataSock->write(Msg);

			WritetoOutputWindowEx(Sess, (unsigned char *)Msg, (int)strlen(Msg),
				Sess->termWindow, &Sess->OutputSaveLen, Sess->OutputSave, EchoText);		// Black

//			Sess->termWindow->insertPlainText(Msg);

			free(Msg);

			Msgptr = ptr;
		}
	}

	else if (Sess->clientSocket && Sess->clientSocket->state() == QAbstractSocket::ConnectedState)
	{
		while ((ptr = strchr(Msgptr, '\n')))
		{
			*ptr++ = 0;

			Msg = (char *)malloc(strlen(Msgptr) + 10);
			strcpy(Msg, Msgptr);
			strcat(Msg, "\r");

			Sess->clientSocket->write(Msg);

			WritetoOutputWindowEx(Sess, (unsigned char *)Msg, (int)strlen(Msg),
				Sess->termWindow, &Sess->OutputSaveLen, Sess->OutputSave, EchoText);		// Black

//			Sess->termWindow->insertPlainText(Msg);

			free(Msg);

			Msgptr = ptr;
		}
	}
	else
	{
		// Not Connected 

		if (Sess->SessionType != Listen)
		{
			if (Sess->CurrentHost < MAXHOSTS)
			{
				// Last connect was TCP

				char Msg[] = "Connecting....\r";

				WritetoOutputWindowEx(Sess, (unsigned char *)Msg, (int)strlen(Msg),
					Sess->termWindow, &Sess->OutputSaveLen, Sess->OutputSave, WarningText);		// Red

				delete(Sess->clientSocket);

				Sess->clientSocket = new myTcpSocket();
				Sess->clientSocket->Sess = Sess;

				connect(Sess->clientSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
				connect(Sess->clientSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
				connect(Sess->clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));

				Sess->clientSocket->connectToHost(&Host[Sess->CurrentHost][0], Port[Sess->CurrentHost]);
			}

			else if (Sess->CurrentHost == 16)		// AGW
			{
				// Invoke AGW connect dialog 

				AGWConnect dialog(0);
				dialog.exec();

				WritetoOutputWindow(Sess, (unsigned char *)"Connecting...\r", 14);
				return;
			}

			else if (Sess->CurrentHost == 17)		// VARA
			{
				// Invoke VARA connect dialog 

				VARAConnect dialog(0);
				dialog.exec();

				WritetoOutputWindow(Sess, (unsigned char *)"Connecting...\r", 14);
				return;
			}

			else if (Sess->CurrentHost == 18)		// KISS
			{
				// Do we send as UI or invoke kiss connect dialog ??

				// Try Connect Dialog for now - may make a menu option

				KISSConnect dialog(0);
				dialog.exec();

				WritetoOutputWindow(Sess, (unsigned char *)"Connecting...\r", 14);
				return;
			}
		}
		else
		{
			char Msg[] = "Incoming Session - Can't Connect\r";

			WritetoOutputWindowEx(Sess, (unsigned char *)Msg, (int)strlen(Msg),
				Sess->termWindow, &Sess->OutputSaveLen, Sess->OutputSave, 81);		// Red

		}
	}

	if (scrollbarAtBottom)
		Sess->termWindow->moveCursor(QTextCursor::End);

	Sess->inputWindow->setText("");
	a->inputMethod()->hide();
}


void QtTermTCP::displayError(QAbstractSocket::SocketError socketError)
{
	myTcpSocket* sender = static_cast<myTcpSocket*>(QObject::sender());

	switch (socketError)
	{
	case QAbstractSocket::RemoteHostClosedError:
		break;

	case QAbstractSocket::HostNotFoundError:
		QMessageBox::information(this, tr("QtTermTCP"),
			tr("The host was not found. Please check the "
				"host name and portsettings->"));
		break;

	case QAbstractSocket::ConnectionRefusedError:
		QMessageBox::information(this, tr("QtTermTCP"),
			tr("The connection was refused by the peer."));
		break;

	default:
		QMessageBox::information(this, tr("QtTermTCP"),
			tr("The following error occurred: %1.")
			.arg(sender->errorString()));
	}

	connectMenu->setEnabled(true);
}

void QtTermTCP::readyRead()
{
	int Read;
	unsigned char Buffer[8192];
	myTcpSocket* Socket = static_cast<myTcpSocket*>(QObject::sender());

	Ui_ListenSession * Sess = (Ui_ListenSession *)Socket->Sess;

	// read the data from the socket

	Read = Socket->read((char *)Buffer, 2047);

	while (Read > 0)
	{
		//		if (InputMode == 'Y')			// Yapp
		//		{
		//			QString myString = QString::fromUtf8((char*)Buffer, Read);
		//			QByteArray ptr = myString.toLocal8Bit();
		//			memcpy(Buffer, ptr.data(), ptr.length());
		//			Read = ptr.length();
		//		}

			
		Buffer[Read] = 0;

		ProcessReceivedData(Sess, Buffer, Read);

		QString myString = QString::fromUtf8((char*)Buffer);
		//		qDebug() << myString;
		Read = Socket->read((char *)Buffer, 2047);
	}
}

extern "C" int SocketSend(Ui_ListenSession * Sess, char * Buffer, int len)
{
	myTcpSocket *Socket = Sess->clientSocket;

	if (Socket && Socket->state() == QAbstractSocket::ConnectedState)
		return Socket->write(Buffer, len);

	return 0;
}

extern "C" int SocketFlush(Ui_ListenSession * Sess)
{
	if (Sess->AGWSession || Sess->KISSSession || (VARASock && VARASock->Sess == Sess))
		return 0;

	myTcpSocket* Socket = Sess->clientSocket;

	if (Socket && Socket->state() == QAbstractSocket::ConnectedState)
		return Socket->flush();

	return 0;
}

extern "C" void mySleep(int ms)
{
	QThread::msleep(ms);
}

extern "C" void SetPortMonLine(int i, char * Text, int visible, int enabled)
{
	MonPort[i]->setText(Text);
	MonPort[i]->setVisible(visible);
	MonPort[i]->setChecked(enabled);
}

bool scrollbarAtBottom = 0;

extern "C" void WritetoOutputWindowEx(Ui_ListenSession * Sess, unsigned char * Buffer, int len, QTextEdit * termWindow, int * OutputSaveLen, char * OutputSave, QColor Colour);

extern "C" void WritetoOutputWindow(Ui_ListenSession * Sess, unsigned char * Buffer, int len)
{
	WritetoOutputWindowEx(Sess, Buffer, len, Sess->termWindow, &Sess->OutputSaveLen, Sess->OutputSave, outputText);
}


extern "C" void WritetoOutputWindowEx(Ui_ListenSession * Sess, unsigned char * Buffer, int len, QTextEdit * termWindow, int * OutputSaveLen, char * OutputSave, QColor Colour)
{
	unsigned char Copy[8192];
	unsigned char * ptr1, *ptr2;
	unsigned char Line[8192];
	unsigned char out[8192];
	int outlen;

	int num;

	if (termWindow == NULL)
		return;

	time_t NOW = time(NULL);

	// Beep if no output for a while

	if (AlertInterval && (NOW - LastWrite) > AlertInterval)
	{
		if (AlertBeep)
			myBeep(&IntervalWAV);

	}

	// Alert if QtTerm not active window (unless Mon window)

	if((Sess->SessionType & Mon) == 0)
		FlashifNotActive();

	// if tabbed and not active tab set tab label red

	if (TermMode == Tabbed)
	{
		if (Sess->monWindow != termWindow)		// Not if Monitor
		{
			if (ActiveSession != Sess)
			{
				tabWidget->tabBar()->setTabTextColor(Sess->Tab, newTabText);
			}
		}
	}

	LastWrite = NOW;

	// Mustn't mess with original buffer

	memcpy(Copy, Buffer, len);

	Copy[len] = 0;

	ptr1 = Copy;

	const QTextCursor old_cursor = termWindow->textCursor();
	const int old_scrollbar_value = termWindow->verticalScrollBar()->value();
	const bool is_scrolled_down = old_scrollbar_value == termWindow->verticalScrollBar()->maximum();

	if (*OutputSaveLen)
	{
		// Have part line - append to it
		memcpy(&OutputSave[*OutputSaveLen], Copy, len);
		*OutputSaveLen += len;
		ptr1 = (unsigned char *)OutputSave;
		len = *OutputSaveLen;
		*OutputSaveLen = 0;

		// part line was written to screen so remove it

//		termWindow->setFocus();
		termWindow->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
		termWindow->moveCursor(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
		termWindow->moveCursor(QTextCursor::End, QTextCursor::KeepAnchor);
		termWindow->textCursor().removeSelectedText();
		//		termWindow->textCursor().deletePreviousChar();
	}

	// Move the cursor to the end of the document.

	termWindow->moveCursor(QTextCursor::End);

	// Insert the text at the position of the cursor (which is the end of the document).

lineloop:

	if (len <= 0)
	{
		if (old_cursor.hasSelection() || !is_scrolled_down)
		{
			// The user has selected text or scrolled away from the bottom: maintain position.
			termWindow->setTextCursor(old_cursor);
			termWindow->verticalScrollBar()->setValue(old_scrollbar_value);
		}
		else
		{
			// The user hasn't selected any text and the scrollbar is at the bottom: scroll to the bottom.
			termWindow->moveCursor(QTextCursor::End);
			termWindow->verticalScrollBar()->setValue(termWindow->verticalScrollBar()->maximum());
		}

		return;
	}


	//	copy text to control a line at a time	

	ptr2 = (unsigned char *)memchr(ptr1, 13, len);

	if (ptr2 == 0)	// No CR
	{
		if (len > 8000)
			len = 8000;			// Should never get lines this long

		memmove(OutputSave, ptr1, len);
		*OutputSaveLen = len;

		// Write part line to screen

		memcpy(Line, ptr1, len);
		Line[len] = 0;

		// I don't think I need to worry if UTF8 as will be rewritten when rest arrives

		if (Line[0] == 0x1b)			// Colour Escape
		{
			if (Sess->MonitorColour)
				termWindow->setTextColor(Colours[Line[1] - 10]);

			termWindow->textCursor().insertText(QString::fromUtf8((char*)&Line[2]));
		}
		else
		{
			termWindow->setTextColor(Colour);
			termWindow->textCursor().insertText(QString::fromUtf8((char*)Line));
		}

		//		*OutputSaveLen = 0;		// Test if we need to delete part line

		//		if (scrollbarAtBottom)
		//			termWindow->moveCursor(QTextCursor::End);

		if (old_cursor.hasSelection() || !is_scrolled_down)
		{
			// The user has selected text or scrolled away from the bottom: maintain position.
			termWindow->setTextCursor(old_cursor);
			termWindow->verticalScrollBar()->setValue(old_scrollbar_value);
		}
		else
		{
			// The user hasn't selected any text and the scrollbar is at the bottom: scroll to the bottom.
			termWindow->moveCursor(QTextCursor::End);
			termWindow->verticalScrollBar()->setValue(termWindow->verticalScrollBar()->maximum());
		}
		return;
	}

	*(ptr2++) = 0;

	if (Bells)
	{
		char * ptr;

		do {
			ptr = (char *)memchr(ptr1, 7, len);

			if (ptr)
			{
				*(ptr) = 32;
				myBeep(&BellWAV);
			}
		} while (ptr);
	}

	num = ptr2 - ptr1 - 1;

	//		if (LogMonitor) WriteMonitorLine(ptr1, ptr2 - ptr1);

	memcpy(Line, ptr1, num);
	Line[num++] = 13;
	Line[num] = 0;

	if (Line[0] == 0x1b)			// Colour Escape
	{
		if (Sess->MonitorColour)
			termWindow->setTextColor(Colours[Line[1] - 10]);

		outlen = checkUTF8(&Line[2], num - 2, out);
		out[outlen] = 0;
		termWindow->textCursor().insertText(QString::fromUtf8((char*)out));
		//		termWindow->textCursor().insertText(QString::fromUtf8((char*)&Line[2]));
	}
	else
	{
		termWindow->setTextColor(Colour);

		outlen = checkUTF8(Line, num, out);
		out[outlen] = 0;
		termWindow->textCursor().insertText(QString::fromUtf8((char*)out));
		//		termWindow->textCursor().insertText(QString::fromUtf8((char*)Line));
	}

	len -= (ptr2 - ptr1);

	ptr1 = ptr2;

	if ((len > 0) && StripLF)
	{
		if (*ptr1 == 0x0a)					// Line Feed
		{
			ptr1++;
			len--;
		}
	}



	goto lineloop;

}

void WriteMonitorLog(Ui_ListenSession * Sess, char * Msg);

extern "C" void WritetoMonWindow(Ui_ListenSession * Sess, unsigned char * Msg, int len)
{
	char * ptr1 = (char *)Msg, *ptr2;
	char Line[512];
	int num;

	//	QScrollBar *scrollbar = Sess->monWindow->verticalScrollBar();
	//	bool scrollbarAtBottom = (scrollbar->value() >= (scrollbar->maximum() - 4));

	if (Sess->monWindow == nullptr)
		return;

	Sess->monWindow->setStyleSheet(monStyleSheet);

	const QTextCursor old_cursor = Sess->monWindow->textCursor();
	const int old_scrollbar_value = Sess->monWindow->verticalScrollBar()->value();
	const bool is_scrolled_down = old_scrollbar_value == Sess->monWindow->verticalScrollBar()->maximum();

	// Move the cursor to the end of the document.
	Sess->monWindow->moveCursor(QTextCursor::End);

	// Insert the text at the position of the cursor (which is the end of the document).


	// Write a line at a time so we can action colour chars

//		Buffer[Len] = 0;

//	if (scrollbarAtBottom)
//		Sess->monWindow->moveCursor(QTextCursor::End);

	if (Sess->MonSaveLen)
	{
		// Have part line - append to it
		memcpy(&Sess->MonSave[Sess->MonSaveLen], Msg, len);
		Sess->MonSaveLen += len;
		ptr1 = Sess->MonSave;
		len = Sess->MonSaveLen;
		Sess->MonSaveLen = 0;
	}

lineloop:

	if (len <= 0)
	{
		//		if (scrollbarAtBottom)
		//			Sess->monWindow->moveCursor(QTextCursor::End);


		if (old_cursor.hasSelection() || !is_scrolled_down)
		{
			// The user has selected text or scrolled away from the bottom: maintain position.
			Sess->monWindow->setTextCursor(old_cursor);
			Sess->monWindow->verticalScrollBar()->setValue(old_scrollbar_value);
		}
		else
		{
			// The user hasn't selected any text and the scrollbar is at the bottom: scroll to the bottom.
			Sess->monWindow->moveCursor(QTextCursor::End);
			Sess->monWindow->verticalScrollBar()->setValue(Sess->monWindow->verticalScrollBar()->maximum());
		}

		return;
	}

	//	copy text to control a line at a time	

	ptr2 = (char *)memchr(ptr1, 13, len);

	if (ptr2 == 0)	// No CR
	{
		memmove(Sess->MonSave, ptr1, len);
		Sess->MonSaveLen = len;
		return;
	}

	*(ptr2++) = 0;

	
	if (Sess->LogMonitor)
		WriteMonitorLog(Sess, ptr1);

	num = ptr2 - ptr1 - 1;

	memcpy(Line, ptr1, num);
	Line[num++] = 13;
	Line[num] = 0;

	if (Line[0] == 0x1b)			// Colour Escape
	{
		if (Sess->MonitorColour)
		{
			if (Line[1] == 17)
				Sess->monWindow->setTextColor(monRxText);
			else
				Sess->monWindow->setTextColor(monTxText);
		}
		else
			Sess->monWindow->setTextColor(monOtherText);

		Sess->monWindow->textCursor().insertText(QString::fromUtf8((char*)&Line[2]));
	}
	else
	{
		Sess->monWindow->textCursor().insertText(QString::fromUtf8((char*)Line));
	}
	len -= (ptr2 - ptr1);

	ptr1 = ptr2;

	goto lineloop;

}

int ProcessYAPPMessage(Ui_ListenSession * Sess, UCHAR * Msg, int Len);
void QueueMsg(Ui_ListenSession * Sess, char * Msg, int Len);

// YAPP stuff

#define SOH 1
#define STX 2
#define ETX 3
#define EOT 4
#define ENQ 5
#define ACK 6
#define DLE	0x10
#define NAK 0x15
#define CAN 0x18

extern "C" void SendtoTerm(Ui_ListenSession * Sess, char * Msg, int Len)
{
	if (Sess == 0)
		return;
	
	Sess->SlowTimer = 0;
	Msg[Len] = 0;

	if (Sess->InputMode == 'Y')			// Yapp
	{
		ProcessYAPPMessage(Sess, (unsigned char *)Msg, Len);
		return;
	}

	// Could be a YAPP Header

	if (Len == 2 && Msg[0] == ENQ && Msg[1] == 1)		// YAPP Send_Init
	{
		char YAPPRR[2];

		// Turn off monitoring

		Sess->InputMode = 'Y';

		YAPPRR[0] = ACK;
		YAPPRR[1] = 1;
		QueueMsg(Sess, YAPPRR, 2);

		return;
	}

	if (AutoTeletext && (Msg[0] == 0x1e || Msg[0] == 0x0c))
	{
		if (Sess->TTActive == 0)
		{
			Sess->TTActive = 1;
			DoTermResize(Sess);
		}
	}

	if (Sess->TTActive)
	{
		// Feed to Teletext code

		// We need to decode a whole page. There is no obvious delimiter so process till data stops.
		// Buffer is cleared when next input is sent

		if (strlen(&Sess->pageBuffer[0] + Len) > 4090)
			Sess->pageBuffer[0] = 0;							// Protect buffer

		strcat(Sess->pageBuffer, (char *)Msg);

		DecodeTeleText(Sess, (char *)Sess->pageBuffer);			// Re-decode same data until we get the end
		return;
	}

	WritetoOutputWindow(Sess, (unsigned char *)Msg, Len);

}

QSettings* settings;

// This makes geting settings for more channels easier

char Prefix[16] = "AX25_A";

void GetPortSettings(int Chan);

QVariant getAX25Param(const char * key, QVariant Default)
{
	char fullKey[64];

	sprintf(fullKey, "%s/%s", Prefix, key);
	return settings->value(fullKey, Default);
}

void getAX25Params(int chan)
{
	Prefix[5] = chan + 'A';
	GetPortSettings(chan);
}


void GetPortSettings(int Chan)
{
	maxframe[Chan] = getAX25Param("Maxframe", 4).toInt();
	fracks[Chan] = getAX25Param("Retries", 10).toInt();
	frack_time[Chan] = getAX25Param("FrackTime", 8).toInt();
	persist[Chan] = getAX25Param("Persist", 128).toInt();
	kisspaclen[Chan] = getAX25Param("Paclen", 128).toInt();
	idletime[Chan] = getAX25Param("IdleTime", 180).toInt();
	slottime[Chan] = getAX25Param("SlotTime", 100).toInt();
	resptime[Chan] = getAX25Param("RespTime", 1500).toInt();
	TXFrmMode[Chan] = getAX25Param("TXFrmMode", 1).toInt();
	max_frame_collector[Chan] = getAX25Param("FrameCollector", 6).toInt();
	//exclude_callsigns[Chan]= getAX25Param("ExcludeCallsigns/");
	//exclude_APRS_frm[Chan]= getAX25Param("ExcludeAPRSFrmType/");
	KISS_opt[Chan] = getAX25Param("KISSOptimization", false).toInt();;
	dyn_frack[Chan] = getAX25Param("DynamicFrack", false).toInt();;
	IPOLL[Chan] = getAX25Param("IPOLL", 80).toInt();

}


void GetSettings()
{
	QByteArray qb;
	int i;
	char Key[16];
	char Param[256];

	settings = new QSettings(GetConfPath(), QSettings::IniFormat);

	// Get saved session definitions

	sessionList = strdup(settings->value("Sessions", "1|3, 0, 5, 5, 600, 800|").toString().toUtf8());

	for (i = 0; i < MAXHOSTS; i++)
	{
		sprintf(Key, "HostParams%d", i);

		qb = settings->value(Key).toByteArray();

		DecodeSettingsLine(i, qb.data());
	}

	Split = settings->value("Split", 50).toInt();
	ChatMode = settings->value("ChatMode", 1).toInt();
	AutoTeletext = settings->value("AutoTeletext", 0).toInt();
	Bells = settings->value("Bells", 1).toInt();
	StripLF = settings->value("StripLF", 1).toInt();
	useBeep = settings->value("useBeep", true).toBool();
	AlertBeep = settings->value("AlertBeep", 1).toInt();
	AlertInterval = settings->value("AlertInterval", 300).toInt();
	ConnectBeep = settings->value("ConnectBeep", 1).toInt();
	ConnectWAV = settings->value("ConnectWAV", ":/PCBeep").toString().toUtf8();
	AlertWAV = settings->value("AlertWAV", ":/PCBeep").toString().toUtf8();
	BellWAV = settings->value("BellWAV", ":/PCBeep").toString().toUtf8();
	IntervalWAV = settings->value("IntervalWAV", ":/PCBeep").toString().toUtf8();

	UseKeywords = settings->value("UseKeywords", 0).toInt();
	KeyWordsFile = settings->value("KeyWordsFile", "keywords.sys").toString().toUtf8();

	SavedHost = settings->value("CurrentHost", 0).toInt();
	strcpy(YAPPPath, settings->value("YAPPPath", "").toString().toUtf8());
	MaxRXSize = settings->value("MaxRXSize", 100000).toInt();

	listenPort = settings->value("listenPort", 8015).toInt();
	listenEnable = settings->value("listenEnable", false).toBool();
	strcpy(listenCText, settings->value("listenCText", "").toString().toUtf8());

	TermMode = settings->value("TermMode", 0).toInt();
	singlemodeFormat = settings->value("singlemodeFormat", Term + Mon).toInt();

	AGWEnable = settings->value("AGWEnable", 0).toInt();
	AGWMonEnable = settings->value("AGWMonEnable", 0).toInt();
	AGWLocalTime = settings->value("AGWLocalTime", 0).toInt();
	AGWMonNodes = settings->value("AGWMonNodes", 0).toInt();
	strcpy(AGWTermCall, settings->value("AGWTermCall", "").toString().toUtf8());
	strcpy(AGWBeaconDest, settings->value("AGWBeaconDest", "").toString().toUtf8());
	strcpy(AGWBeaconPath, settings->value("AGWBeaconPath", "").toString().toUtf8());
	AGWBeaconInterval = settings->value("AGWBeaconInterval", 0).toInt();
	strcpy(AGWBeaconPorts, settings->value("AGWBeaconPorts", "").toString().toUtf8());
	strcpy(AGWBeaconMsg, settings->value("AGWBeaconText", "").toString().toUtf8());
	strcpy(AGWHost, settings->value("AGWHost", "127.0.0.1").toString().toUtf8());
	AGWPortNum = settings->value("AGWPort", 8000).toInt();
	AGWPaclen = settings->value("AGWPaclen", 80).toInt();
	AGWToCalls = settings->value("AGWToCalls", "").toStringList();
	convUTF8 = settings->value("convUTF8", 0).toInt();

	KISSEnable = settings->value("KISSEnable", 0).toInt();
	KISSMonEnable = settings->value("KISSMonEnable", 1).toInt();
	KISSLocalTime = settings->value("KISSLocalTime", 0).toInt();
	KISSMonNodes = settings->value("KISSMonNodes", 0).toInt();

	KISSListen = settings->value("KISSListen", 1).toInt();
	strcpy(MYCALL, settings->value("MYCALL", "").toString().toUtf8());
	strcpy(KISSHost, settings->value("KISSHost", "127.0.0.1").toString().toUtf8());
	KISSPortNum = settings->value("KISSPort", 8100).toInt();
	KISSMode = settings->value("KISSMode", 0).toInt();

	strcpy(SerialPort, settings->value("KISSSerialPort", "None").toString().toUtf8());
	KISSBAUD = settings->value("KISSBAUD", 19200).toInt();
	getAX25Params(0);

	VARAEnable = settings->value("VARAEnable", 0).toInt();
	strcpy(VARAHost, settings->value("VARAHost", "127.0.0.1").toString().toUtf8());
	strcpy(VARAHostFM, settings->value("VARAHostFM", "127.0.0.1").toString().toUtf8());
	strcpy(VARAHostHF, settings->value("VARAHostHF", "127.0.0.1").toString().toUtf8());
	strcpy(VARAHostSAT, settings->value("VARAHostSAT", "127.0.0.1").toString().toUtf8());
	VARAPortNum = settings->value("VARAPort", 8300).toInt();
	VARAPortHF = settings->value("VARAPortHF", 8300).toInt();
	VARAPortFM = settings->value("VARAPortFM", 8300).toInt();
	VARAPortSAT = settings->value("VARAPortSAT", 8300).toInt();
	strcpy(VARAPath, settings->value("VARAPath", "C:\\VARA\\VARA.exe").toString().toUtf8());
	strcpy(VARAPathHF, settings->value("VARAPathHF", "C:\\VARA\\VARA.exe").toString().toUtf8());
	strcpy(VARAPathFM, settings->value("VARAPathFM", "C:\\VARA\\VARAFM.exe").toString().toUtf8());
	strcpy(VARAPathSAT, settings->value("VARAPathSAT", "C:\\VARA\\VARASAT.exe").toString().toUtf8());
	strcpy(VARATermCall, settings->value("VARATermCall", "").toString().toUtf8());
	VARA500 = settings->value("VARA500", 0).toInt();
	VARA2300 = settings->value("VARA2300", 1).toInt();
	VARA2750 = settings->value("VARA2750", 0).toInt();
	VARAHF = settings->value("VARAHF", 1).toInt();
	VARAFM = settings->value("VARAFM", 0).toInt();
	VARASAT = settings->value("VARASAT", 0).toInt();
	strcpy(VARAInit, settings->value("VARAInit", "").toString().toUtf8());

	strcpy(PTTPort, settings->value("PTT", "None").toString().toUtf8());
	PTTMode = settings->value("PTTMode", 19200).toInt();
	PTTBAUD = settings->value("PTTBAUD", 19200).toInt();
	CATHex = settings->value("CATHex", 1).toInt();

	strcpy(PTTOnString, settings->value("PTTOnString", "").toString().toUtf8());
	strcpy(PTTOffString, settings->value("PTTOffString", "").toString().toUtf8());

	pttGPIOPin = settings->value("pttGPIOPin", 17).toInt();
	pttGPIOPinR = settings->value("pttGPIOPinR", 17).toInt();

#ifdef WIN32
	strcpy(CM108Addr, settings->value("CM108Addr", "0xD8C:0x08").toString().toUtf8());
#else
	strcpy(CM108Addr, settings->value("CM108Addr", "/dev/hidraw0").toString().toUtf8());
#endif

	HamLibPort = settings->value("HamLibPort", 4532).toInt();
	strcpy(HamLibHost, settings->value("HamLibHost", "127.0.0.1").toString().toUtf8());

	FLRigPort = settings->value("FLRigPort", 12345).toInt();
	strcpy(FLRigHost, settings->value("FLRigHost", "127.0.0.1").toString().toUtf8());


	strcpy(Param, settings->value("TabType", "1 1 1 1 1 1 1 2 2").toString().toUtf8());
	sscanf(Param, "%d %d %d %d %d %d %d %d %d %d",
		&TabType[0], &TabType[1], &TabType[2], &TabType[3], &TabType[4],
		&TabType[5], &TabType[6], &TabType[7], &TabType[8], &TabType[9]);

	strcpy(Param, settings->value("AutoConnect", "0, 0 ,0, 0, 0, 0, 0, 0, 0, 0").toString().toUtf8());
	sscanf(Param, "%d %d %d %d %d %d %d %d %d %d",
		&AutoConnect[0], &AutoConnect[1], &AutoConnect[2], &AutoConnect[3], &AutoConnect[4],
		&AutoConnect[5], &AutoConnect[6], &AutoConnect[7], &AutoConnect[8], &AutoConnect[9]);

	strcpy(Param, settings->value("currentHost", "0, 0 ,0, 0, 0, 0, 0, 0, 0, 0").toString().toUtf8());
	sscanf(Param, "%d %d %d %d %d %d %d %d %d %d",
		&currentHost[0], &currentHost[1], &currentHost[2], &currentHost[3], &currentHost[4],
		&currentHost[5], &currentHost[6], &currentHost[7], &currentHost[8], &currentHost[9]);


	monBackground = settings->value("monBackground", QColor(255, 255, 255)).value<QColor>();
	monRxText = settings->value("monRxText", QColor(0, 0, 255)).value<QColor>();
	monTxText = settings->value("monTxText", QColor(255, 0, 0)).value<QColor>();
	monOtherText = settings->value("monOtherText", QColor(0, 0, 0)).value<QColor>();

	termBackground = settings->value("termBackground", QColor(255, 255, 255)).value<QColor>();
	outputText = settings->value("outputText", QColor(0, 0, 255)).value<QColor>();
	EchoText = settings->value("EchoText", QColor(0, 0, 0)).value<QColor>();
	WarningText = settings->value("WarningText", QColor(255, 0, 0)).value<QColor>();

	inputBackground = settings->value("inputBackground", QColor(255, 255, 255)).value<QColor>();
	inputText = settings->value("inputText", QColor(0, 0, 0)).value<QColor>();

	delete(settings);
}

void SavePortSettings(int Chan);

void saveAX25Param(const char * key, QVariant Value)
{
	char fullKey[64];

	sprintf(fullKey, "%s/%s", Prefix, key);

	settings->setValue(fullKey, Value);
}

void saveAX25Params(int chan)
{
	Prefix[5] = chan + 'A';
	SavePortSettings(chan);
}

void SavePortSettings(int Chan)
{
	saveAX25Param("Retries", fracks[Chan]);
	saveAX25Param("Maxframe", maxframe[Chan]);
	saveAX25Param("Paclen", kisspaclen[Chan]);
	saveAX25Param("FrackTime", frack_time[Chan]);
	saveAX25Param("IdleTime", idletime[Chan]);
	saveAX25Param("SlotTime", slottime[Chan]);
	saveAX25Param("Persist", persist[Chan]);
	saveAX25Param("RespTime", resptime[Chan]);
	saveAX25Param("TXFrmMode", TXFrmMode[Chan]);
	saveAX25Param("FrameCollector", max_frame_collector[Chan]);
	saveAX25Param("ExcludeCallsigns", exclude_callsigns[Chan]);
	saveAX25Param("ExcludeAPRSFrmType", exclude_APRS_frm[Chan]);
	saveAX25Param("KISSOptimization", KISS_opt[Chan]);
	saveAX25Param("DynamicFrack", dyn_frack[Chan]);
	saveAX25Param("BitRecovery", recovery[Chan]);
	saveAX25Param("IPOLL", IPOLL[Chan]);
	saveAX25Param("MyDigiCall", MyDigiCall[Chan]);
}

extern "C" void SaveSettings()
{
	int i;
	char Param[512];
	char Key[16];

	settings = new QSettings(GetConfPath(), QSettings::IniFormat);

	for (i = 0; i < MAXHOSTS; i++)
	{
		sprintf(Key, "HostParams%d", i);
		EncodeSettingsLine(i, Param);
		settings->setValue(Key, Param);
	}

	settings->setValue("Split", Split);
	settings->setValue("ChatMode", ChatMode);
	settings->setValue("AutoTeletext", AutoTeletext);
	settings->setValue("Bells", Bells);
	settings->setValue("StripLF", StripLF);
	settings->setValue("AlertBeep", AlertBeep);
	settings->setValue("useBeep", useBeep);
	settings->setValue("ConnectBeep", ConnectBeep);
	settings->setValue("BellWAV", BellWAV);
	settings->setValue("AlertWAV", AlertWAV);
	settings->setValue("IntervalWAV", IntervalWAV);
	settings->setValue("ConnectWAV", ConnectWAV);

	settings->setValue("UseKeywords", UseKeywords);
	settings->setValue("KeyWordsFile", KeyWordsFile);

	settings->setValue("AlertInterval", AlertInterval);
	settings->setValue("CurrentHost", SavedHost);

	settings->setValue("YAPPPath", YAPPPath);
	settings->setValue("MaxRXSize", MaxRXSize);

	settings->setValue("listenPort", listenPort);
	settings->setValue("listenEnable", listenEnable);
	settings->setValue("listenCText", listenCText);
	settings->setValue("convUTF8", convUTF8);

	settings->setValue("PTT", PTTPort);
	settings->setValue("PTTBAUD", PTTBAUD);
	settings->setValue("PTTMode", PTTMode);

	settings->setValue("CATHex", CATHex);

	settings->setValue("PTTOffString", PTTOffString);
	settings->setValue("PTTOnString", PTTOnString);

	settings->setValue("pttGPIOPin", pttGPIOPin);
	settings->setValue("pttGPIOPinR", pttGPIOPinR);

	settings->setValue("CM108Addr", CM108Addr);
	settings->setValue("HamLibPort", HamLibPort);
	settings->setValue("HamLibHost", HamLibHost);
	settings->setValue("FLRigPort", FLRigPort);
	settings->setValue("FLRigHost", FLRigHost);


	// Save Sessions

	char SessionString[1024];
	int SessStringLen;

	SessStringLen = sprintf(SessionString, "%d|", _sessions.size());

	if (TermMode == MDI)
	{
		for (int i = 0; i < _sessions.size(); ++i)
		{
			Ui_ListenSession * Sess = _sessions.at(i);

			QRect r = Sess->sw->geometry();

			SessStringLen += sprintf(&SessionString[SessStringLen],
				"%d, %d, %d, %d, %d, %d|", Sess->SessionType, Sess->CurrentHost, r.top(), r.left(), r.bottom(), r.right());
		}

		settings->setValue("Sessions", SessionString);
	}

	settings->setValue("AGWEnable", AGWEnable);
	settings->setValue("AGWMonEnable", AGWMonEnable);
	settings->setValue("AGWLocalTime", AGWLocalTime);
	settings->setValue("AGWMonNodes", AGWMonNodes);
	settings->setValue("AGWTermCall", AGWTermCall);
	settings->setValue("AGWBeaconDest", AGWBeaconDest);
	settings->setValue("AGWBeaconPath", AGWBeaconPath);
	settings->setValue("AGWBeaconInterval", AGWBeaconInterval);
	settings->setValue("AGWBeaconPorts", AGWBeaconPorts);
	settings->setValue("AGWBeaconText", AGWBeaconMsg);
	settings->setValue("AGWHost", AGWHost);
	settings->setValue("AGWPort", AGWPortNum);
	settings->setValue("AGWPaclen", AGWPaclen);
	settings->setValue("AGWToCalls", AGWToCalls);

	settings->setValue("KISSEnable", KISSEnable);
	settings->setValue("KISSMonEnable", KISSMonEnable);
	settings->setValue("KISSLocalTime", KISSLocalTime);
	settings->setValue("KISSMonNodes", KISSMonNodes);

	settings->setValue("KISSListen", KISSListen);
	settings->setValue("MYCALL", MYCALL);
	settings->setValue("KISSHost", KISSHost);
	settings->setValue("KISSMode", KISSMode);
	settings->setValue("KISSPort", KISSPortNum);
	settings->setValue("KISSSerialPort", SerialPort);
	settings->setValue("KISSBAUD", KISSBAUD);
	saveAX25Params(0);

	settings->setValue("VARAEnable", VARAEnable);
	settings->setValue("VARATermCall", VARATermCall);
	settings->setValue("VARAHost", VARAHost);
	settings->setValue("VARAPort", VARAPortNum);
	settings->setValue("VARAInit", VARAInit);
	settings->setValue("VARAPath", VARAPath);
	settings->setValue("VARAHostHF", VARAHostHF);
	settings->setValue("VARAPortHF", VARAPortHF);
	settings->setValue("VARAPathHF", VARAPathHF);
	settings->setValue("VARAHostFM", VARAHostFM);
	settings->setValue("VARAPortFM", VARAPortFM);
	settings->setValue("VARAPathFM", VARAPathFM);
	settings->setValue("VARAHostSAT", VARAHostSAT);
	settings->setValue("VARAPortSAT", VARAPortSAT);
	settings->setValue("VARAPathSAT", VARAPathSAT);
	settings->setValue("VARA500", VARA500);
	settings->setValue("VARA2300", VARA2300);
	settings->setValue("VARA2750", VARA2750);
	settings->setValue("VARAHF", VARAHF);
	settings->setValue("VARAFM", VARAFM);
	settings->setValue("VARASAT", VARASAT);

	sprintf(Param, "%d %d %d %d %d %d %d %d %d %d",
		TabType[0], TabType[1], TabType[2], TabType[3], TabType[4], TabType[5], TabType[6], TabType[7], TabType[8], TabType[9]);

	settings->setValue("TabType", Param);

	sprintf(Param, "%d %d %d %d %d %d %d %d %d %d",
		AutoConnect[0], AutoConnect[1], AutoConnect[2], AutoConnect[3], AutoConnect[4], AutoConnect[5], AutoConnect[6], AutoConnect[7], AutoConnect[8], AutoConnect[9]);

	settings->setValue("AutoConnect", Param);

	sprintf(Param, "%d %d %d %d %d %d %d %d %d %d",
		currentHost[0], currentHost[1], currentHost[2], currentHost[3], currentHost[4], currentHost[5], currentHost[6], currentHost[7], currentHost[8], currentHost[9]);

	settings->setValue("currentHost", Param);

	settings->setValue("monBackground", monBackground);
	settings->setValue("monRxText", monRxText);
	settings->setValue("monTxText", monTxText);
	settings->setValue("monOtherText", monOtherText);

	settings->setValue("termBackground", termBackground);
	settings->setValue("outputText", outputText);
	settings->setValue("EchoText", EchoText);
	settings->setValue("WarningText", WarningText);

	settings->setValue("inputBackground", inputBackground);
	settings->setValue("inputText", inputText);

	settings->sync();
	delete(settings);
}

#include <QCloseEvent>

void QtTermTCP::closeEvent(QCloseEvent *event)
{
	QMessageBox::StandardButton resBtn = QMessageBox::question(this, "QtTermTCP",
		tr("Are you sure?\n"),
		QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
		QMessageBox::Yes);
	if (resBtn != QMessageBox::Yes) {
		event->ignore();
	}
	else
	{
		event->accept();
#ifdef USESERIAL
		if (hPTTDevice)
			hPTTDevice->close();
#endif
		if (process)
			process->close();
	}
}

QtTermTCP::~QtTermTCP()
{
	Ui_ListenSession * Sess;

	for (int i = 0; i < _sessions.size(); ++i)
	{
		Sess = _sessions.at(i);

		if (Sess->monLogfile)
			Sess->monLogfile->close();

		if (Sess->clientSocket)
		{
			int loops = 100;
			Sess->clientSocket->disconnectFromHost();
			while (Sess->clientSocket && loops-- && Sess->clientSocket->state() != QAbstractSocket::UnconnectedState)
				QThread::msleep(10);
		}
	}

	if (AGWSock && AGWSock->ConnectedState == QAbstractSocket::ConnectedState)
	{
		int loops = 100;
		AGWSock->disconnectFromHost();
		QThread::msleep(10);
		while (AGWSock && loops-- && AGWSock->state() != QAbstractSocket::UnconnectedState)
			QThread::msleep(10);
	}

	if (_server->isListening())
		_server->close();

	delete(_server);

	QSettings mysettings(GetConfPath(), QSettings::IniFormat);
	mysettings.setValue("geometry", saveGeometry());
	mysettings.setValue("windowState", saveState());

	SaveSettings();
}

extern "C" void timer_event();

void QtTermTCP::KISSTimerSlot()
{
	// Runs every 100 mS

	timer_event();
}

void QtTermTCP::MyTimerSlot()
{
	// Runs every 10 seconds

	Ui_ListenSession * Sess;

	for (int i = 0; i < _sessions.size(); ++i)
	{
		Sess = _sessions.at(i);

		//	for (Ui_ListenSession * Sess : _sessions)
		//	{
		if (Sess == NULL)
			continue;

		if (!ChatMode)
			continue;

		if (Sess->KISSSession ||
			(Sess->clientSocket && Sess->clientSocket->state() == QAbstractSocket::ConnectedState))
		{
			Sess->SlowTimer++;

			if (Sess->SlowTimer > 54)				// About 9 mins
			{
				unsigned char Msg[2] = "";

				Sess->SlowTimer = 0;

				if (Sess->KISSSession)
				{
					TAX25Port * ax25 = (TAX25Port *)Sess->KISSSession;

					if (ax25->status == STAT_LINK)
						SendtoAX25(Sess->KISSSession, Msg, 1);
				}
				else
					Sess->clientSocket->write("\0", 1);
			}
		}
	}

	if (AGWEnable)
		AGWTimer();

	if (VARAEnable)
		VARATimer();

	if (KISSEnable)
		KISSTimer();

}

extern "C" void myBeep(QString * WAV)
{
	if (useBeep)
	{
		QApplication::beep();
		return;
	}

	// Using .wav files

//	QSoundEffect effect;
//	effect.setSource(QUrl::fromLocalFile(*WAV));
//	effect.setLoopCount(1);
//	effect.setVolume(1.0f);
//	effect.play();

	QSound::play(*WAV);
}

void QtTermTCP::ListenSlot()
{
	// This runs the Listen Configuration dialog

	ListenDialog  * xx = new ListenDialog();
	xx->exec();
}

void QtTermTCP::AGWSlot()
{
	// This runs the AGW Configuration dialog

	AGWDialog dialog(0);
	dialog.exec();
}

Ui_Dialog * Dev;
Ui_AlertDialog * Alert;

static Ui_KISSDialog * KISS;
static Ui_ColourDialog * COLOURS;


char NewPTTPort[80];

int newSoundMode = 0;
int oldSoundMode = 0;

#ifdef USESERIAL
QList<QSerialPortInfo> Ports = QSerialPortInfo::availablePorts();
#endif


void QtTermTCP::KISSSlot()
{
	// This runs the KISS Configuration dialog

	KISS = new(Ui_KISSDialog);

	QDialog UI;

	KISS->setupUi(&UI);

	UI.setFont(*menufont);

	deviceUI = &UI;
	KISS->KISSEnable->setChecked(KISSEnable);
	KISS->KISSListen->setChecked(KISSListen);
	KISS->MYCALL->setText(MYCALL);

	//	connect(KISS->SerialPort, SIGNAL(currentIndexChanged(int)), this, SLOT(PTTPortChanged(int)));

	QStringList items;

#ifdef USESERIAL

	for (const QSerialPortInfo &info : Ports)
	{
		items.append(info.portName());
	}

	items.sort();
	items.insert(0, "TCP");

#endif

	for (const QString &info : items)
	{
		KISS->SerialPort->addItem(info);
	}

	KISS->SerialPort->setCurrentIndex(KISS->SerialPort->findText(SerialPort, Qt::MatchFixedString));
	KISS->Speed->setText(QString::number(KISSBAUD));
	KISS->Host->setText(KISSHost);
	KISS->Port->setText(QString::number(KISSPortNum));

	KISS->Paclen->setText(QString::number(kisspaclen[0]));
	KISS->Maxframe->setText(QString::number(maxframe[0]));
	KISS->Frack->setText(QString::number(frack_time[0]));
	KISS->Retries->setText(QString::number(fracks[0]));

	QObject::connect(KISS->okButton, SIGNAL(clicked()), this, SLOT(KISSaccept()));
	QObject::connect(KISS->cancelButton, SIGNAL(clicked()), this, SLOT(KISSreject()));

	UI.exec();

}


void QtTermTCP::KISSaccept()
{
	QVariant Q;
	int OldEnable = KISSEnable;
	char oldSerialPort[64];
	int OldPort = KISSPortNum;
	char oldHost[128];

	strcpy(oldSerialPort, SerialPort);
	strcpy(oldHost, KISSHost);

	KISSEnable = KISS->KISSEnable->isChecked();
	KISSListen = KISS->KISSListen->isChecked();
	actHost[18]->setVisible(KISSEnable);			// Show KISS Connect Line

	strcpy(MYCALL, KISS->MYCALL->text().toUtf8().toUpper());

	memset(axMYCALL, 0, 7);
	ConvToAX25(MYCALL, axMYCALL);

	Q = KISS->Port->text();
	KISSPortNum = Q.toInt();

	Q = KISS->Speed->text();
	KISSBAUD = Q.toInt();

	strcpy(KISSHost, KISS->Host->text().toUtf8().toUpper());

	Q = KISS->SerialPort->currentText();
	strcpy(SerialPort, Q.toString().toUtf8());

	Q = KISS->Paclen->text();
	kisspaclen[0] = Q.toInt();
	Q = KISS->Maxframe->text();
	maxframe[0] = Q.toInt();
	Q = KISS->Frack->text();
	frack_time[0] = Q.toInt();
	Q = KISS->Retries->text();
	fracks[0] = Q.toInt();

	myStatusBar->setVisible(AGWEnable | VARAEnable | KISSEnable);


	if (KISSEnable != OldEnable || KISSPortNum != OldPort ||
		strcmp(oldHost, KISSHost) != 0 ||
		strcmp(oldSerialPort, SerialPort) != 0)
	{
		// (re)start connection

		if (OldEnable)
		{
			if (KISSEnable)
				Status3->setText("KISS Closed");
			else
				Status3->setText("KISS Disabled");

			KISSConnected = 0;

			if (KISSSock && KISSSock->ConnectedState == QAbstractSocket::ConnectedState)
				KISSSock->disconnectFromHost();
			else if (m_serial)
				closeSerialPort();
		}
	}

	delete(KISS);
	SaveSettings();
	deviceUI->accept();

	//	QSize newSize(this->size());
	//	QSize oldSize(this->size());

	//	QResizeEvent *myResizeEvent = new QResizeEvent(newSize, oldSize);

	//	QCoreApplication::postEvent(this, myResizeEvent);
}

void QtTermTCP::KISSreject()
{
	delete(KISS);
	deviceUI->reject();
}


void QtTermTCP::AlertSlot()
{
	// This runs the VARA Configuration dialog

	Alert = new(Ui_AlertDialog);

	QDialog UI;

	Alert->setupUi(&UI);

	UI.setFont(*menufont);

	deviceUI = &UI;

	Alert->Bells->setChecked(Bells);
	Alert->InboundBeep->setChecked(ConnectBeep);
	Alert->InactivityBeep->setChecked(AlertBeep);
	Alert->Interval->setText(QString::number(AlertInterval));
	Alert->KeywordBeep->setChecked(UseKeywords);
	Alert->keywordFile->setText(KeyWordsFile);

	Alert->useBeep->setChecked(useBeep);
	Alert->useFiles->setChecked(!useBeep);

	Alert->connectFile->setCurrentText(ConnectWAV);
	Alert->bellsFile->setCurrentText(BellWAV);
	Alert->intervalFile->setCurrentText(IntervalWAV);
	Alert->keywordWAV->setCurrentText(AlertWAV);

	QObject::connect(Alert->chooseInbound, SIGNAL(clicked()), this, SLOT(chooseInboundWAV()));
	QObject::connect(Alert->chooseBells, SIGNAL(clicked()), this, SLOT(chooseBellsWAV()));
	QObject::connect(Alert->chooseInterval, SIGNAL(clicked()), this, SLOT(chooseIntervalWAV()));
	QObject::connect(Alert->chooseKeyAlert, SIGNAL(clicked()), this, SLOT(chooseAlertWAV()));

	QObject::connect(Alert->testBells, SIGNAL(clicked()), this, SLOT(testBellsWAV()));
	QObject::connect(Alert->TestInbound, SIGNAL(clicked()), this, SLOT(testInboundWAV()));
	QObject::connect(Alert->testInterval, SIGNAL(clicked()), this, SLOT(testIntervalWAV()));
	QObject::connect(Alert->TestKeyAlert, SIGNAL(clicked()), this, SLOT(testAlertWAV()));

	QObject::connect(Alert->okButton, SIGNAL(clicked()), this, SLOT(alertAccept()));
	QObject::connect(Alert->cancelButton, SIGNAL(clicked()), this, SLOT(alertReject()));

	UI.exec();
}






void QtTermTCP::chooseInboundWAV()
{
	ConnectWAV = QFileDialog::getOpenFileName(this,
		tr("Select Wav"), "", tr("Sound Files (*.wav)"));

	Alert->connectFile->setCurrentText(ConnectWAV);

}

void QtTermTCP::chooseBellsWAV()
{
	BellWAV = QFileDialog::getOpenFileName(this,
		tr("Select Wav"), "", tr("Sound Files (*.wav)"));

	Alert->bellsFile->setCurrentText(BellWAV);
}

void QtTermTCP::chooseIntervalWAV()
{
	IntervalWAV = QFileDialog::getOpenFileName(this,
		tr("Select Wav"), "", tr("Sound Files (*.wav)"));

	Alert->intervalFile->setCurrentText(IntervalWAV);
}

void QtTermTCP::chooseAlertWAV()
{
	AlertWAV = QFileDialog::getOpenFileName(this,
		tr("Select Wav"), "", tr("Sound Files (*.wav)"));

	Alert->keywordWAV->setCurrentText(AlertWAV);
}

void QtTermTCP::testInboundWAV()
{
	QSound::play(Alert->connectFile->currentText());
}

void QtTermTCP::testBellsWAV()
{
	QSound::play(Alert->bellsFile->currentText());
}

void QtTermTCP::testIntervalWAV()
{
	QSound::play(Alert->intervalFile->currentText());
}

void QtTermTCP::testAlertWAV()
{
	QSound::play(Alert->keywordWAV->currentText());
}



void QtTermTCP::alertAccept()
{
	QVariant Q;

	useBeep = Alert->useBeep->isChecked();

	Bells = Alert->Bells->isChecked();
	ConnectBeep = Alert->InboundBeep->isChecked();
	AlertBeep = Alert->InactivityBeep->isChecked();
	AlertInterval = Alert->Interval->text().toInt();

	UseKeywords = Alert->KeywordBeep->isChecked();
	KeyWordsFile = Alert->keywordFile->text();

	ConnectWAV = Alert->connectFile->currentText();
	BellWAV = Alert->bellsFile->currentText();
	IntervalWAV = Alert->intervalFile->currentText();
	AlertWAV = Alert->keywordWAV->currentText();

	delete(Alert);
	SaveSettings();
	deviceUI->accept();
}

void QtTermTCP::alertReject()
{
	delete(Alert);
	deviceUI->reject();
}



void QtTermTCP::VARASlot()
{
	// This runs the VARA Configuration dialog

	char valChar[80];

	Dev = new(Ui_Dialog);

	QDialog UI;

	Dev->setupUi(&UI);

	UI.setFont(*menufont);

	deviceUI = &UI;

	Dev->VARAEnable->setChecked(VARAEnable);
	Dev->TermCall->setText(VARATermCall);

	SetVARAParams();

	connect(Dev->VARAHF, SIGNAL(toggled(bool)), this, SLOT(VARAHFChanged(bool)));
	connect(Dev->VARAFM, SIGNAL(toggled(bool)), this, SLOT(VARAFMChanged(bool)));
	connect(Dev->VARASAT, SIGNAL(toggled(bool)), this, SLOT(VARASATChanged(bool)));

	if (VARA500)
		Dev->VARA500->setChecked(true);
	else if (VARA2750)
		Dev->VARA2750->setChecked(true);
	else
		Dev->VARA2300->setChecked(true);

	if (VARAHF)
		Dev->VARAHF->setChecked(true);
	else if (VARAFM)
		Dev->VARAFM->setChecked(true);
	else if (VARASAT)
		Dev->VARASAT->setChecked(true);

	if (VARAHF == 0)
		Dev->HFMode->setVisible(false);

	connect(Dev->CAT, SIGNAL(toggled(bool)), this, SLOT(CATChanged(bool)));
	connect(Dev->PTTPort, SIGNAL(currentIndexChanged(int)), this, SLOT(PTTPortChanged(int)));

	if (PTTMode == PTTCAT)
		Dev->CAT->setChecked(true);
	else
		Dev->RTSDTR->setChecked(true);

	if (CATHex)
		Dev->CATHex->setChecked(true);
	else
		Dev->CATText->setChecked(true);

	sprintf(valChar, "%d", pttGPIOPin);
	Dev->GPIOLeft->setText(valChar);
	sprintf(valChar, "%d", pttGPIOPinR);
	Dev->GPIORight->setText(valChar);

	Dev->VIDPID->setText(CM108Addr);


	QStringList items;

#ifdef USESERIAL

	for (const QSerialPortInfo &info : Ports)
	{
		items.append(info.portName());
	}

	items.sort();

#endif

	Dev->PTTPort->addItem("None");
	Dev->PTTPort->addItem("CM108");

#ifdef __ARM_ARCH

	//	Dev->PTTPort->addItem("GPIO");

#endif

	Dev->PTTPort->addItem("FLRIG");
	Dev->PTTPort->addItem("HAMLIB");

	for (const QString &info : items)
	{
		Dev->PTTPort->addItem(info);
	}

	Dev->PTTPort->setCurrentIndex(Dev->PTTPort->findText(PTTPort, Qt::MatchFixedString));

	PTTPortChanged(0);				// Force reevaluation

	QObject::connect(Dev->okButton, SIGNAL(clicked()), this, SLOT(deviceaccept()));
	QObject::connect(Dev->cancelButton, SIGNAL(clicked()), this, SLOT(devicereject()));

	UI.exec();

}

extern QProcess *process;

void ClosePTTPort();

void QtTermTCP::deviceaccept()
{
	QVariant Q;

	int OldEnable = VARAEnable;
	int OldPort = VARAPortNum;
	char oldHost[128];
	char oldPath[256];

	strcpy(oldHost, VARAHost);
	strcpy(oldPath, VARAPath);

	VARAEnable = Dev->VARAEnable->isChecked();

	strcpy(VARATermCall, Dev->TermCall->text().toUtf8().toUpper());

	Q = Dev->Port->text();

	VARAPortNum = Q.toInt();
	strcpy(VARAHost, Dev->Host->text().toUtf8().toUpper());
	strcpy(VARAPath, Dev->Path->text().toUtf8());
	strcpy(VARAInit, Dev->InitCommands->text().toUtf8());

	VARA500 = Dev->VARA500->isChecked();
	VARA2300 = Dev->VARA2300->isChecked();
	VARA2750 = Dev->VARA2750->isChecked();

	VARAHF = Dev->VARAHF->isChecked();
	VARAFM = Dev->VARAFM->isChecked();
	VARASAT = Dev->VARASAT->isChecked();

	if (VARAHF)
	{
		strcpy(VARAHostHF, VARAHost);
		strcpy(VARAPathHF, VARAPath);
		VARAPortHF = VARAPortNum;
	}
	else if (VARAFM)
	{
		strcpy(VARAHostFM, VARAHost);
		strcpy(VARAPathFM, VARAPath);
		VARAPortFM = VARAPortNum;
	}
	else if (VARASAT)
	{
		strcpy(VARAHostSAT, VARAHost);
		strcpy(VARAPathSAT, VARAPath);
		VARAPortSAT = VARAPortNum;
	}

	Q = Dev->PTTPort->currentText();
	strcpy(PTTPort, Q.toString().toUtf8());

	if (Dev->CAT->isChecked())
		PTTMode = PTTCAT;
	else
		PTTMode = PTTRTS;

	if (Dev->CATHex->isChecked())
		CATHex = 1;
	else
		CATHex = 0;

	Q = Dev->PTTOn->text();
	strcpy(PTTOnString, Q.toString().toUtf8());
	Q = Dev->PTTOff->text();
	strcpy(PTTOffString, Q.toString().toUtf8());

	Q = Dev->CATSpeed->text();
	PTTBAUD = Q.toInt();

	Q = Dev->GPIOLeft->text();
	pttGPIOPin = Q.toInt();

	Q = Dev->GPIORight->text();
	pttGPIOPinR = Q.toInt();

	Q = Dev->VIDPID->text();

	if (strcmp(PTTPort, "CM108") == 0)
		strcpy(CM108Addr, Q.toString().toUtf8());
	else if (strcmp(PTTPort, "HAMLIB") == 0)
	{
		HamLibPort = Q.toInt();
		Q = Dev->PTTOn->text();
		strcpy(HamLibHost, Q.toString().toUtf8());
	}

	if (VARAEnable != OldEnable || VARAPortNum != OldPort || strcmp(oldHost, VARAHost) != 0)
	{
		// (re)start connection

		if (OldEnable && VARASock && VARASock->ConnectedState == QAbstractSocket::ConnectedState)
		{
			VARASock->disconnectFromHost();
			if (VARADataSock)
				VARADataSock->disconnectFromHost();
			Status2->setText("VARA Disconnected");
		}
	}

	if (process && process->state() == QProcess::Running)
		if ((VARAEnable == 0 || strcmp(oldPath, VARAPath) != 0))
			process->close();

	myStatusBar->setVisible(AGWEnable | VARAEnable | KISSEnable);

	ClosePTTPort();
	OpenPTTPort();

	delete(Dev);
	SaveSettings();
	deviceUI->accept();

	QSize newSize(this->size());
	QSize oldSize(this->size());

	QResizeEvent *myResizeEvent = new QResizeEvent(newSize, oldSize);

	QCoreApplication::postEvent(this, myResizeEvent);
}

void QtTermTCP::devicereject()
{
	delete(Dev);
	deviceUI->reject();
}

// This handles incoming connections

void QtTermTCP::onNewConnection()
{
	myTcpSocket *clientSocket = (myTcpSocket *)_server->nextPendingConnection();

	clientSocket->Sess = NULL;

	Ui_ListenSession * S;
	Ui_ListenSession  * Sess = NULL;

	char Title[512];
	int i = 0;

	QByteArray Host = clientSocket->peerAddress().toString().toUtf8();

	if (TermMode == MDI)
	{
		// See if an old session can be reused

		for (int i = 0; i < _sessions.size(); ++i)
		{
			S = _sessions.at(i);

			//	for (Ui_ListenSession * S: _sessions)
			//	{
			if ((S->SessionType & Listen) && S->clientSocket == NULL)
			{
				Sess = S;
				break;
			}
		}

		// Create a window if none found, else reuse old

		if (Sess == NULL)
		{
			Sess = newWindow(this, Listen);
		}

		QByteArray Host = clientSocket->peerAddress().toString().toUtf8();
	}
	else
	{
		// Single or Tabbed - look for free session

		for (i = 0; i < _sessions.size(); ++i)
		{
			S = _sessions.at(i);

			if (S->clientSocket == NULL && S->AGWSession == NULL && S->AGWMonSession == NULL && S->KISSSession == NULL)
			{
				Sess = S;
				break;
			}
		}

		if (Sess == NULL)
		{
			// Clear connection

			clientSocket->disconnectFromHost();
			return;
		}
	}

	_sockets.push_back(clientSocket);

	clientSocket->Sess = Sess;

	// See if any data from host - first msg should be callsign

	clientSocket->waitForReadyRead(1000);

	QByteArray datas = clientSocket->readAll();

	datas.chop(2);
	datas.truncate(10);				// Just in case!

	datas.append('\0');

	sprintf(Title, "Inward Connect from %s:%d Call " + datas,
		Host.data(), clientSocket->peerPort());

	if (TermMode == MDI)
	{
		Sess->setWindowTitle(Title);
	}
	else if (TermMode == Tabbed)
	{
		tabWidget->setTabText(i, datas.data());
		tabWidget->tabBar()->setTabTextColor(i, newTabText);
	}

	else if (TermMode == Single)
		this->setWindowTitle(Title);

	connect(clientSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
	connect(clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));

	Sess->clientSocket = clientSocket;

	// We need to set Connect and Disconnect if the window is active

	if (TermMode == MDI && Sess->sw == ActiveSubWindow)
		setMenus(true);

	if (TermMode == Tabbed && Sess->Tab == tabWidget->currentIndex())
		setMenus(true);

	if (TermMode == Single)
		setMenus(true);			// Single

	// Send CText if defined

	if (listenCText[0])
		Sess->clientSocket->write(listenCText);

	// Send Message to Terminal

	char Msg[80];

	sprintf(Msg, "Listen Connect from %s\r\r", datas.data());

	WritetoOutputWindow(Sess, (unsigned char *)Msg, (int)strlen(Msg));

	if (ConnectBeep)
		myBeep(&ConnectWAV);

	QApplication::alert(mythis, 0);
}

void QtTermTCP::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
	myTcpSocket* sender = static_cast<myTcpSocket*>(QObject::sender());
	Ui_ListenSession * Sess = (Ui_ListenSession *)sender->Sess;

	if (socketState == QAbstractSocket::UnconnectedState)
	{
		char Msg[] = "Disconnected\r";

		WritetoOutputWindowEx(Sess, (unsigned char *)Msg, (int)strlen(Msg),
			Sess->termWindow, &Sess->OutputSaveLen, Sess->OutputSave, WarningText);		// Red

		if (TermMode == MDI)
		{
			if (Sess->SessionType == Mon)			// Mon Only
				Sess->setWindowTitle("Monitor Session Disconnected");
			else
				Sess->setWindowTitle("Disconnected");
		}
		else if (TermMode == Tabbed)
		{
			if (Sess->SessionType == Mon)			// Mon Only
				tabWidget->setTabText(Sess->Tab, "Monitor");
			else
			{
				char Label[16];
				sprintf(Label, "Sess %d", Sess->Tab + 1);
				tabWidget->setTabText(Sess->Tab, Label);
			}
		}
		else if (TermMode == Single)
		{
			if (Sess->AGWMonSession)
				mythis->setWindowTitle("AGW Monitor Window");
			else
			{
				if (Sess->SessionType == Mon)			// Mon Only
					this->setWindowTitle("Monitor Session Disconnected");
				else
					this->setWindowTitle("Disconnected");
			}
		}

		if (Sess->TTActive)
		{
			Sess->TTActive = 0;
			DoTermResize(Sess);
		}

		Sess->PortMonString[0] = 0;

		//		delete(Sess->clientSocket);
		Sess->clientSocket = NULL;

		discAction->setEnabled(false);

		if ((Sess->SessionType & Listen))
			_sockets.removeOne(sender);
		else
		{
			connectMenu->setEnabled(true);
			YAPPSend->setEnabled(false);
		}
	}
	else if (socketState == QAbstractSocket::ConnectedState)
	{
		char Signon[256];
		char Title[128];

		// only seems to be triggered for outward connect

		sprintf(Signon, "%s\r%s\rBPQTERMTCP\r", UserName[Sess->CurrentHost], Password[Sess->CurrentHost]);

		Sess->clientSocket->write(Signon);

		discAction->setEnabled(true);
		YAPPSend->setEnabled(true);
		connectMenu->setEnabled(false);

		SendTraceOptions(Sess);

		Sess->InputMode = 0;
		Sess->SlowTimer = 0;
		Sess->MonData = 0;
		Sess->OutputSaveLen = 0;			// Clear any part line
		Sess->MonSaveLen = 0;			// Clear any part line

		if (Sess->SessionType == Mon)		// Mon Only
			sprintf(Title, "Monitor Session Connected to %s", Host[Sess->CurrentHost]);
		else
		{
			char Label[256];

			if (SessName[Sess->CurrentHost][0])
				sprintf(Label, "%s(%s)", Host[Sess->CurrentHost], SessName[Sess->CurrentHost]);
			else
				strcpy(Label, Host[Sess->CurrentHost]);

			sprintf(Title, "Connected to %s", Label);
		}

		if (TermMode == MDI)
			Sess->setWindowTitle(Title);
		else if (TermMode == Tabbed)
		{
			if (SessName[Sess->CurrentHost][0])
				tabWidget->setTabText(Sess->Tab, SessName[Sess->CurrentHost]);
			else
				tabWidget->setTabText(Sess->Tab, Host[Sess->CurrentHost]);
		}
		else if (TermMode == Single)
			this->setWindowTitle(Title);
	}
}

void QtTermTCP::updateWindowMenu()
{
	if (TermMode == MDI)
	{
		windowMenu->clear();
		windowMenu->addAction(newTermAct);
		windowMenu->addAction(newMonAct);
		windowMenu->addAction(newCombinedAct);
		windowMenu->addSeparator();
		windowMenu->addAction(closeAct);
		windowMenu->addAction(closeAllAct);
		windowMenu->addSeparator();
		windowMenu->addAction(tileAct);
		windowMenu->addAction(cascadeAct);
		windowMenu->addSeparator();
		windowMenu->addAction(nextAct);
		windowMenu->addAction(previousAct);
		windowMenu->addAction(quitAction);
		windowMenu->addAction(windowMenuSeparatorAct);

		QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
		windowMenuSeparatorAct->setVisible(!windows.isEmpty());

		Ui_ListenSession * Sess;

		for (int i = 0; i < _sessions.size(); ++i)
		{
			Sess = _sessions.at(i);
			Sess->actActivate = windowMenu->addAction(Sess->sw->windowTitle());
			QAction::connect(Sess->actActivate, SIGNAL(triggered()), this, SLOT(actActivate()));
			Sess->actActivate->setCheckable(true);
			Sess->actActivate->setChecked(ActiveSubWindow == Sess->sw);
		}
	}
	else if (TermMode == Tabbed)
	{
		windowMenu->clear();

		Ui_ListenSession * Sess = (Ui_ListenSession *)tabWidget->currentWidget();

		QActionGroup * termGroup = new QActionGroup(this);

		delete(TabSingle);
		delete(TabBoth);
		delete(TabMon);

		TabSingle = setupMenuLine(nullptr, (char *)"Terminal Only", this, (Sess->SessionType == Term));
		TabBoth = setupMenuLine(nullptr, (char *)"Terminal + Monitor", this, (Sess->SessionType == Term + Mon));
		TabMon = setupMenuLine(nullptr, (char *)"Monitor Only", this, (Sess->SessionType == Mon));

		termGroup->addAction(TabSingle);
		termGroup->addAction(TabBoth);
		termGroup->addAction(TabMon);

		windowMenu->addAction(TabSingle);
		windowMenu->addAction(TabBoth);
		windowMenu->addAction(TabMon);

	}
}

Ui_ListenSession::~Ui_ListenSession()
{
	if (this->clientSocket)
	{
		int loops = 100;
		this->clientSocket->disconnectFromHost();
		while (loops-- && this->clientSocket->state() != QAbstractSocket::UnconnectedState)
			QThread::msleep(10);
	}
}

extern "C" void setTraceOff(Ui_ListenSession * Sess)
{
	if ((Sess->SessionType & Mon) == 0)
		return;					// Not Monitor

	if (Sess->AGWMonSession)
		return;

	char Buffer[80];
	int Len = sprintf(Buffer, "\\\\\\\\0 0 0 0 0 0 0 0\r");

	SocketFlush(Sess);
	SocketSend(Sess, Buffer, Len);
	SocketFlush(Sess);
}


extern "C" void SendTraceOptions(Ui_ListenSession * Sess)
{
	if ((Sess->SessionType & Mon) == 0)
		return;					// Not Monitor

	if (Sess->AGWMonSession)
		return;

	char Buffer[80];
	int Len = sprintf(Buffer, "\\\\\\\\%llx %x %x %x %x %x %x %x\r", Sess->portmask, Sess->mtxparam | (Sess->mlocaltime << 7),
		Sess->mcomparam, Sess->MonitorNODES, Sess->MonitorColour, Sess->monUI, 0, 1);

	strcpy(&MonParams[Sess->CurrentHost][0], &Buffer[4]);
	SaveSettings();
	SocketFlush(Sess);
	SocketSend(Sess, Buffer, Len);
	SocketFlush(Sess);
}

void QtTermTCP::doNewTerm()
{
	newWindow(this, Term);
}

void QtTermTCP::doNewMon()
{
	newWindow(this, Mon);
}

void QtTermTCP::doNewCombined()
{
	newWindow(this, Term + Mon);
}

void QtTermTCP::doCascade()
{
	// Qt Cascade Minimizes windows so do it ourselves

	int x = 0, y = 0;

	Ui_ListenSession * Sess;

	for (int i = 0; i < _sessions.size(); ++i)
	{
		Sess = _sessions.at(i);

		Sess->sw->move(x, y);
		x += 14;
		y += 30;
	}
}

void QtTermTCP::actActivate()
{
	QAction * sender = static_cast<QAction*>(QObject::sender());

	Ui_ListenSession * Sess;

	for (int i = 0; i < _sessions.size(); ++i)
	{
		Sess = _sessions.at(i);

		if (Sess->actActivate == sender)
		{
			mdiArea->setActiveSubWindow(Sess->sw);
			return;
		}
	}
}


void QtTermTCP::xon_mdiArea_changed()
{
	// This is triggered when the Active MDI window changes
	// and is used to enable/disable Connect, Disconnect and YAPP Send


	QMdiSubWindow *SW = mdiArea->activeSubWindow();

	// Dont waste time if not changed

	if (ActiveSubWindow == SW)
		return;

	ActiveSubWindow = SW;

	Ui_ListenSession * Sess;

	for (int i = 0; i < _sessions.size(); ++i)
	{
		Sess = _sessions.at(i);

		//	for (Ui_ListenSession * Sess : _sessions)
		//	{
		if (Sess->sw == SW)
		{
			ActiveSession = Sess;

			if (Sess->clientSocket && Sess->clientSocket->state() == QAbstractSocket::ConnectedState)
			{
				discAction->setEnabled(true);
				YAPPSend->setEnabled(true);
				connectMenu->setEnabled(false);
			}
			else if (Sess->AGWMonSession)
			{
				// Connected AGW Monitor Session

				discAction->setEnabled(false);
				YAPPSend->setEnabled(false);
				connectMenu->setEnabled(false);
			}
			else if (Sess->AGWSession || Sess->KISSSession)
			{
				// Connected AGW or KISS Terminal Session

				discAction->setEnabled(true);
				YAPPSend->setEnabled(true);
				connectMenu->setEnabled(false);
			}
			else
			{
				// Not connected

				discAction->setEnabled(false);
				YAPPSend->setEnabled(false);

				if ((Sess->SessionType & Listen))		// Listen Sessions can't connect
					connectMenu->setEnabled(false);
				else
					connectMenu->setEnabled(true);
			}

			// If a monitor Window, change Monitor config settings

			EnableMonLog->setChecked(Sess->LogMonitor);

			if (AGWUsers && Sess == AGWUsers->MonSess)		// AGW Monitor
			{
				for (int i = 0; i < 64; i++)
					SetPortMonLine(i, (char *)"", 0, 0);			// Set all hidden

				connectMenu->setEnabled(false);
				MonTX->setVisible(0);
				MonSup->setVisible(0);
				MonUI->setVisible(0);
				MonColour->setVisible(0);

				EnableMonitor->setVisible(1);
				EnableMonitor->setChecked(AGWMonEnable);

				MonLocalTime->setChecked(Sess->mlocaltime);
				MonNodes->setChecked(Sess->MonitorNODES);
			}
			else if (Sess == KISSMonSess)				// KISS Monitor
			{
				for (int i = 0; i < 64; i++)
					SetPortMonLine(i, (char *)"", 0, 0);			// Set all hidden

				connectMenu->setEnabled(false);
				MonTX->setVisible(0);
				MonSup->setVisible(0);
				MonUI->setVisible(0);
				MonColour->setVisible(0);

				EnableMonitor->setVisible(1);
				EnableMonitor->setChecked(KISSMonEnable);
				MonLocalTime->setChecked(Sess->mlocaltime);
				MonNodes->setChecked(Sess->MonitorNODES);
			}

			else
			{
				EnableMonitor->setVisible(0);
				MonTX->setVisible(1);
				MonSup->setVisible(1);
				MonUI->setVisible(1);
				MonColour->setVisible(1);
			}

			if (Sess->PortMonString[0])
			{
				char * ptr = (char *)malloc(2048);
				memcpy(ptr, Sess->PortMonString, 2048);

				int NumberofPorts = atoi((char *)&ptr[2]);
				char *p, *Context;
				char msg[80];
				int portnum;
				char delim[] = "|";

				// Remove old Monitor menu

				for (int i = 0; i < 64; i++)
				{
					SetPortMonLine(i, (char *)"", 0, 0);			// Set all hidden
				}

				p = strtok_s((char *)&ptr[2], delim, &Context);

				while (NumberofPorts--)
				{
					p = strtok_s(NULL, delim, &Context);
					if (p == NULL)
						break;

					portnum = atoi(p);

					sprintf(msg, "Port %s", p);

					if (portnum == 0)
						portnum = 64;

					if (Sess->portmask & (1ll << (portnum - 1)))
						SetPortMonLine(portnum, msg, 1, 1);
					else
						SetPortMonLine(portnum, msg, 1, 0);
				}
				free(ptr);

				MonLocalTime->setChecked(Sess->mlocaltime);
				MonTX->setChecked(Sess->mtxparam);
				MonSup->setChecked(Sess->mcomparam);
				MonUI->setChecked(Sess->monUI);
				MonNodes->setChecked(Sess->MonitorNODES);
				MonColour->setChecked(Sess->MonitorColour);

			}

			return;
		}
	}
}

void QtTermTCP::doFonts()
{
	fontDialog  * xx = new fontDialog(0);
	xx->exec();
}

void QtTermTCP::doMFonts()
{
	fontDialog  * xx = new fontDialog(1);
	xx->exec();
}

QColor TempmonBackground = monBackground;
QColor TemptermBackground = termBackground;
QColor TempinputBackground = inputBackground;

QColor TempmonRxText = monRxText;
QColor TempmonTxText = monTxText;
QColor TempmonOtherText = monOtherText;

QColor TempoutputText = outputText;
QColor TempEchoText = EchoText;
QColor TempWarningText = WarningText;


QColor TempinputText = inputText;


void setDialogColours()
{
	char monStyle[128];

	sprintf(monStyle, "color: rgb(%d, %d, %d); background-color: rgb(%d, %d, %d);",
		TempmonOtherText.red(), TempmonOtherText.green(), TempmonOtherText.blue(),
		TempmonBackground.red(), TempmonBackground.green(), TempmonBackground.blue());

	COLOURS->MonitorBG->setStyleSheet(monStyle);
	COLOURS->MonOther->setStyleSheet(monStyle);

	sprintf(monStyle, "color: rgb(%d, %d, %d); background-color: rgb(%d, %d, %d);",
		TempmonTxText.red(), TempmonTxText.green(), TempmonTxText.blue(),
		TempmonBackground.red(), TempmonBackground.green(), TempmonBackground.blue());
	COLOURS->MonTX->setStyleSheet(monStyle);

	sprintf(monStyle, "color: rgb(%d, %d, %d); background-color: rgb(%d, %d, %d);",
		TempmonRxText.red(), TempmonRxText.green(), TempmonRxText.blue(),
		TempmonBackground.red(), TempmonBackground.green(), TempmonBackground.blue());

	COLOURS->MonRX->setStyleSheet(monStyle);

	sprintf(monStyle, "color: rgb(%d, %d, %d); background-color: rgb(%d, %d, %d);",
		TempoutputText.red(), TempoutputText.green(), TempoutputText.blue(),
		TemptermBackground.red(), TemptermBackground.green(), TemptermBackground.blue());

	COLOURS->TermBG->setStyleSheet(monStyle);
	COLOURS->TermNormal->setStyleSheet(monStyle);

	sprintf(monStyle, "color: rgb(%d, %d, %d); background-color: rgb(%d, %d, %d);",
		TempEchoText.red(), TempEchoText.green(), TempEchoText.blue(),
		TemptermBackground.red(), TemptermBackground.green(), TemptermBackground.blue());

	COLOURS->Echoed->setStyleSheet(monStyle);

	sprintf(monStyle, "color: rgb(%d, %d, %d); background-color: rgb(%d, %d, %d);",
		TempWarningText.red(), TempWarningText.green(), TempWarningText.blue(),
		TemptermBackground.red(), TemptermBackground.green(), TemptermBackground.blue());

	COLOURS->Warning->setStyleSheet(monStyle);

	sprintf(monStyle, "color: rgb(%d, %d, %d); background-color: rgb(%d, %d, %d);",
		TempinputText.red(), TempinputText.green(), TempinputText.blue(),
		TempinputBackground.red(), TempinputBackground.green(), TempinputBackground.blue());

	COLOURS->InputBG->setStyleSheet(monStyle);
	COLOURS->InputColour->setStyleSheet(monStyle);

}

void QtTermTCP::doColours()
{
	COLOURS = new(Ui_ColourDialog);

	QDialog UI;

	COLOURS->setupUi(&UI);

	UI.setFont(*menufont);

	deviceUI = &UI;

	TempmonBackground = monBackground;
	TempmonRxText = monRxText;
	TempmonTxText = monTxText;
	TempmonOtherText = monOtherText;

	TemptermBackground = termBackground;
	TempoutputText = outputText;
	TempEchoText = EchoText;
	TempWarningText = WarningText;

	TempinputBackground = inputBackground;
	TempinputText = inputText;

	setDialogColours();

	QObject::connect(COLOURS->MonitorBG, SIGNAL(clicked()), this, SLOT(ColourPressed()));
	QObject::connect(COLOURS->TermBG, SIGNAL(clicked()), this, SLOT(ColourPressed()));
	QObject::connect(COLOURS->InputBG, SIGNAL(clicked()), this, SLOT(ColourPressed()));
	QObject::connect(COLOURS->MonRX, SIGNAL(clicked()), this, SLOT(ColourPressed()));
	QObject::connect(COLOURS->MonTX, SIGNAL(clicked()), this, SLOT(ColourPressed()));
	QObject::connect(COLOURS->MonOther, SIGNAL(clicked()), this, SLOT(ColourPressed()));
	QObject::connect(COLOURS->TermNormal, SIGNAL(clicked()), this, SLOT(ColourPressed()));
	QObject::connect(COLOURS->Echoed, SIGNAL(clicked()), this, SLOT(ColourPressed()));
	QObject::connect(COLOURS->Warning, SIGNAL(clicked()), this, SLOT(ColourPressed()));
	QObject::connect(COLOURS->InputColour, SIGNAL(clicked()), this, SLOT(ColourPressed()));

	QObject::connect(COLOURS->okButton, SIGNAL(clicked()), this, SLOT(Colouraccept()));
	QObject::connect(COLOURS->cancelButton, SIGNAL(clicked()), this, SLOT(Colourreject()));

	UI.exec();

}

void QtTermTCP::ColourPressed()
{
	char Name[32];

	strcpy(Name, sender()->objectName().toUtf8());

	if (strcmp(Name, "MonitorBG") == 0)
		TempmonBackground = setColor(TempmonBackground);

	else if (strcmp(Name, "MonTX") == 0)
		TempmonTxText = setColor(TempmonTxText);

	else if (strcmp(Name, "MonRX") == 0)
		TempmonRxText = setColor(TempmonRxText);

	else if (strcmp(Name, "MonOther") == 0)
		TempmonOtherText = setColor(TempmonOtherText);

	else if (strcmp(Name, "TermBG") == 0)
		TemptermBackground = setColor(TemptermBackground);

	else if (strcmp(Name, "InputBG") == 0)
		TempinputBackground = setColor(TempinputBackground);

	else if (strcmp(Name, "InputColour") == 0)
		TempinputText = setColor(TempinputText);

	else if (strcmp(Name, "TermNormal") == 0)
		TempoutputText = setColor(TempoutputText);

	else if (strcmp(Name, "Echoed") == 0)
		TempEchoText = setColor(TempEchoText);

	else if (strcmp(Name, "Warning") == 0)
		TempWarningText = setColor(TempWarningText);

	setDialogColours();
}


void QtTermTCP::Colouraccept()
{
	monBackground = TempmonBackground;
	monRxText = TempmonRxText;
	monTxText = TempmonTxText;
	monOtherText = TempmonOtherText;

	termBackground = TemptermBackground;
	EchoText = TempEchoText;
	WarningText = TempWarningText;
	outputText = TempoutputText;

	inputBackground = TempinputBackground;
	inputText = TempinputText;

	// Set background colour for new windows

	sprintf(monStyleSheet, "background-color: rgb(%d, %d, %d);",
		monBackground.red(), monBackground.green(), monBackground.blue());

	sprintf(termStyleSheet, "background-color: rgb(%d, %d, %d);",
		termBackground.red(), termBackground.green(), termBackground.blue());

	sprintf(inputStyleSheet, "color: rgb(%d, %d, %d); background-color: rgb(%d, %d, %d);",
		inputText.red(), inputText.green(), inputText.blue(),
		inputBackground.red(), inputBackground.green(), inputBackground.blue());

	// Update existing windows

	for (int i = 0; i < _sessions.size(); ++i)
	{
		Ui_ListenSession * S = _sessions.at(i);

		if (S->monWindow)
			S->monWindow->setStyleSheet(monStyleSheet);

		if (S->termWindow)
			S->termWindow->setStyleSheet(termStyleSheet);

		if (S->inputWindow)
			S->inputWindow->setStyleSheet(inputStyleSheet);

	}


	delete(COLOURS);

	SaveSettings();
	deviceUI->accept();
}

void QtTermTCP::Colourreject()
{
	delete(COLOURS);
	deviceUI->reject();
}


void QtTermTCP::ConnecttoVARA()
{

	delete(VARASock);

	VARASock = new myTcpSocket();

	connect(VARASock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(VARAdisplayError(QAbstractSocket::SocketError)));
	connect(VARASock, SIGNAL(readyRead()), this, SLOT(VARAreadyRead()));
	connect(VARASock, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onVARASocketStateChanged(QAbstractSocket::SocketState)));

	VARASock->connectToHost(VARAHost, VARAPortNum);

	Status2->setText("VARA Control Connecting");

	return;
}


void QtTermTCP::VARATimer()
{
	// Runs every 10 Seconds

	if (VARAConnected == 0 && VARAConnecting == 0)
	{
		if (process == nullptr || process->state() == QProcess::NotRunning)
		{
			if (VARAPath[0])
			{
				process = new QProcess(this);
				QString file = VARAPath;
				process->start(file);
			}
		}
		QThread::msleep(1000);
		VARAConnecting = true;
		ConnecttoVARA();
	}
}


void QtTermTCP::VARAdisplayError(QAbstractSocket::SocketError socketError)
{
	switch (socketError)
	{
	case QAbstractSocket::RemoteHostClosedError:
		break;

	case QAbstractSocket::HostNotFoundError:
		QMessageBox::information(this, tr("QtTermTCP"),
			tr("VARA host was not found. Please check the "
				"host name and portsettings->"));

		Status2->setText("VARA Connection Failed");

		break;

	case QAbstractSocket::ConnectionRefusedError:

		Status2->setText("VARA Connection Refused");
		break;

	default:

		Status2->setText("VARA Connection Failed");
	}

	VARAConnecting = 0;
	VARAConnected = 0;
}

void QtTermTCP::VARAreadyRead()
{
	int Read;
	char Buffer[4096];
	char * ptr;
	char * Msg;
	myTcpSocket* Socket = static_cast<myTcpSocket*>(QObject::sender());

	// read the data from the socket

	Read = Socket->read((char *)Buffer, 4095);

	Buffer[Read] = 0;

	Msg = Buffer;

	ptr = strchr(Msg, 0x0d);

	while (ptr)
	{
		*ptr++ = 0;

		if (strcmp(Msg, "IAMALIVE") == 0)
		{
		}
		else if (strcmp(Msg, "PTT ON") == 0)
		{
			RadioPTT(1);
		}
		else if (strcmp(Msg, "PTT OFF") == 0)
		{
			RadioPTT(0);
		}
		else if (strcmp(Msg, "PENDING") == 0)
		{
		}
		else if (strcmp(Msg, "CANCELPENDING") == 0)
		{
		}
		else if (strcmp(Msg, "OK") == 0)
		{
		}
		else if (memcmp(Msg, "CONNECTED ", 10) == 0)
		{
			Ui_ListenSession * Sess = (Ui_ListenSession *)VARASock->Sess;
			char Title[128] = "";
			char CallFrom[64] = "";
			char CallTo[64] = "";
			char Mode[64] = "";
			char Message[128];
			int n;


			sscanf(&Msg[10], "%s %s %s", CallFrom, CallTo, Mode);

			if (Sess)
			{
				if (Mode[0])
					sprintf(Title, "Connected to %s %s Mode", CallTo, Mode);
				else
					sprintf(Title, "Connected to %s", CallTo);

				n = sprintf(Message, "%s\r\n", Title);
				WritetoOutputWindow(Sess, (unsigned char *)Message, n);

				if (TermMode == MDI)
					Sess->setWindowTitle(Title);
				else if (TermMode == Tabbed)
					tabWidget->setTabText(Sess->Tab, CallTo);
				else if (TermMode == Single)
					mythis->setWindowTitle(Title);

				setMenus(true);
			}
			else
			{
				// Incoming Call

				Ui_ListenSession * S;
				int i = 0;

				if (TermMode == MDI)
				{
					// See if an old session can be reused

					for (int i = 0; i < _sessions.size(); ++i)
					{
						S = _sessions.at(i);

						//	for (Ui_ListenSession * S: _sessions)
						//	{
						if ((S->SessionType & Listen) && S->clientSocket == NULL)
						{
							Sess = S;
							break;
						}
					}

					// Create a window if none found, else reuse old

					if (Sess == NULL)
					{
						Sess = newWindow(this, Listen);
					}
				}
				else
				{
					// Single or Tabbed - look for free session

					for (i = 0; i < _sessions.size(); ++i)
					{
						S = _sessions.at(i);

						if (S->clientSocket == NULL && S->AGWSession == NULL && S->AGWMonSession == NULL && S->KISSSession == NULL)
						{
							Sess = S;
							break;
						}
					}
				}

				if (Sess == NULL)
				{
					// Clear connection

					VARASock->write("DISCONNECT\r");
				}
				else
				{
					if (Mode[0])
					{
						sprintf(Title, "Connected to %s Mode %s", CallFrom, Mode);
						n = sprintf(Message, "Incoming VARA Connect from %s %s Mode\r\n", CallFrom, Mode);
					}
					else
					{
						sprintf(Title, "Connected to %s", CallFrom);
						n = sprintf(Message, "Incoming VARA Connect from %s\r\n", CallFrom);
					}

					WritetoOutputWindow(Sess, (unsigned char *)Message, n);

					VARASock->Sess = Sess;
					VARADataSock->Sess = Sess;

					if (TermMode == MDI)
						Sess->setWindowTitle(Title);
					else if (TermMode == Tabbed)
					{
						tabWidget->setTabText(Sess->Tab, CallFrom);
						tabWidget->tabBar()->setTabTextColor(Sess->Tab, newTabText);
					}
					else if (TermMode == Single)
						mythis->setWindowTitle(Title);

					setMenus(true);

					if (ConnectBeep)
						myBeep(&ConnectWAV);

					if (listenCText[0])
						VARADataSock->write(listenCText);

					QApplication::alert(mythis, 0);

				}
			}
		}
		else if (strcmp(Msg, "DISCONNECTED") == 0)
		{
			Ui_ListenSession * Sess = (Ui_ListenSession *)VARASock->Sess;

			if (Sess)
			{
				WritetoOutputWindow(Sess, (unsigned char *)"Disconnected\r\n", 14);
				VARASock->Sess = 0;
				VARADataSock->Sess = 0;

				if (TermMode == MDI)
				{
					if (Sess->SessionType == Mon)			// Mon Only
						Sess->setWindowTitle("Monitor Session Disconnected");
					else
						Sess->setWindowTitle("Disconnected");
				}
				else if (TermMode == Tabbed)
				{
					if (Sess->SessionType == Mon)			// Mon Only
						tabWidget->setTabText(Sess->Tab, "Monitor");
					else
					{
						char Label[16];
						sprintf(Label, "Sess %d", Sess->Tab + 1);
						tabWidget->setTabText(Sess->Tab, Label);
					}
				}
				else if (TermMode == Single)
				{
					if (Sess->AGWMonSession)
						mythis->setWindowTitle("AGW Monitor Window");
					else
					{
						if (Sess->SessionType == Mon)			// Mon Only
							this->setWindowTitle("Monitor Session Disconnected");
						else
							this->setWindowTitle("Disconnected");
					}
				}

				setMenus(false);
			}
		}

		Msg = ptr;

		ptr = strchr(Msg, 0x0d);
	}



}

void QtTermTCP::onVARASocketStateChanged(QAbstractSocket::SocketState socketState)
{
	//	myTcpSocket* sender = static_cast<myTcpSocket*>(QObject::sender());

	if (socketState == QAbstractSocket::UnconnectedState)
	{
		// Close any connections

		Status2->setText("VARA Disconnected");
		actHost[17]->setVisible(0);

		VARAConnecting = VARAConnected = 0;
	}
	else if (socketState == QAbstractSocket::ConnectedState)
	{
		// Connect Data Session. Leave Connecting till that completes

		VARADataSock = new myTcpSocket();

		connect(VARADataSock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(VARADatadisplayError(QAbstractSocket::SocketError)));
		connect(VARADataSock, SIGNAL(readyRead()), this, SLOT(VARADatareadyRead()));
		connect(VARADataSock, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onVARADataSocketStateChanged(QAbstractSocket::SocketState)));

		VARADataSock->connectToHost(VARAHost, VARAPortNum + 1);
		Status2->setText("VARA Data Connecting");
	}
}


void QtTermTCP::VARADatadisplayError(QAbstractSocket::SocketError socketError)
{
	switch (socketError)
	{
	case QAbstractSocket::RemoteHostClosedError:
		break;

	case QAbstractSocket::HostNotFoundError:
		QMessageBox::information(this, tr("QtTermTCP"),
			tr("VARA host was not found. Please check the "
				"host name and portsettings->"));

		Status2->setText("VARA Connection Failed");

		break;

	case QAbstractSocket::ConnectionRefusedError:

		Status2->setText("VARA Connection Refused");
		break;

	default:

		Status2->setText("VARA Connection Failed");
	}

	VARAConnecting = 0;
	VARAConnected = 0;
}

void QtTermTCP::VARADatareadyRead()
{
	int Read;
	unsigned char Buffer[4096];
	myTcpSocket* Socket = static_cast<myTcpSocket*>(QObject::sender());

	Ui_ListenSession * Sess = (Ui_ListenSession *)Socket->Sess;

	// read the data from the socket

	Read = Socket->read((char *)Buffer, 2047);

	while (Read > 0)
	{
		//		if (InputMode == 'Y')			// Yapp
		//		{
		//			QString myString = QString::fromUtf8((char*)Buffer, Read);
		//			QByteArray ptr = myString.toLocal8Bit();
		//			memcpy(Buffer, ptr.data(), ptr.length());
		//			Read = ptr.length();
		//		}

		ProcessReceivedData(Sess, Buffer, Read);

		QString myString = QString::fromUtf8((char*)Buffer);
		//		qDebug() << myString;
		Read = Socket->read((char *)Buffer, 2047);
	}
}


void QtTermTCP::onVARADataSocketStateChanged(QAbstractSocket::SocketState socketState)
{
	//	myTcpSocket* sender = static_cast<myTcpSocket*>(QObject::sender());

	if (socketState == QAbstractSocket::UnconnectedState)
	{
		// Close any connections

		Status2->setText("VARA Disconnected");
		actHost[17]->setVisible(0);

		VARAConnecting = VARAConnected = 0;
	}
	else if (socketState == QAbstractSocket::ConnectedState)
	{
		char VARACommand[256];

		VARAConnected = 1;
		VARAConnecting = 0;

		Status2->setText("VARA Connected");

		actHost[17]->setVisible(1);			// Enable VARA Connect Line

		sprintf(VARACommand, "MYCALL %s\r", VARATermCall);
		VARASock->write(VARACommand);

		if (VARA500)
			VARASock->write("BW500\r");
		else if (VARA2300)
			VARASock->write("BW2300\r");
		else if (VARA2750)
			VARASock->write("BW2750\r");

		VARASock->write("COMPRESSION FILES\r");

		if (VARAInit[0])
		{
			char Copy[512];
			char * param, *context;

			strcpy(Copy, VARAInit);

			param = strtok_s(Copy, ",", &context);

			while (param && param[0])
			{
				sprintf(VARACommand, "%s\r", param);
				VARASock->write(VARACommand);
				param = strtok_s(nullptr, ",", &context);
			}
		}

		if (listenEnable)
			VARASock->write("LISTEN ON\r");
	}
}

// PTT Stuff

#include "hidapi.h"

// Serial Port Stuff


QTcpSocket * HAMLIBsock;
int HAMLIBConnected = 0;
int HAMLIBConnecting = 0;

void QtTermTCP::HAMLIBdisplayError(QAbstractSocket::SocketError socketError)
{
	switch (socketError)
	{
	case QAbstractSocket::RemoteHostClosedError:
		break;

	case QAbstractSocket::HostNotFoundError:
		QMessageBox::information(nullptr, tr("QtSM"),
			"HAMLIB host was not found. Please check the "
			"host name and portsettings->");

		break;

	case QAbstractSocket::ConnectionRefusedError:

		qDebug() << "HAMLIB Connection Refused";
		break;

	default:

		qDebug() << "HAMLIB Connection Failed";
		break;

	}

	HAMLIBConnecting = 0;
	HAMLIBConnected = 0;
}

void QtTermTCP::HAMLIBreadyRead()
{
	unsigned char Buffer[4096];
	QTcpSocket* Socket = static_cast<QTcpSocket*>(QObject::sender());

	// read the data from the socket. Don't do anyhing with it at the moment

	Socket->read((char *)Buffer, 4095);
}

void QtTermTCP::onHAMLIBSocketStateChanged(QAbstractSocket::SocketState socketState)
{
	if (socketState == QAbstractSocket::UnconnectedState)
	{
		// Close any connections

		HAMLIBConnected = 0;
		HAMLIBConnecting = 0;

		//	delete (HAMLIBsock);
		//	HAMLIBsock = 0;

		qDebug() << "HAMLIB Connection Closed";

	}
	else if (socketState == QAbstractSocket::ConnectedState)
	{
		HAMLIBConnected = 1;
		HAMLIBConnecting = 0;
		qDebug() << "HAMLIB Connected";
	}
}


void QtTermTCP::ConnecttoHAMLIB()
{
	delete(HAMLIBsock);

	HAMLIBConnected = 0;
	HAMLIBConnecting = 1;

	HAMLIBsock = new QTcpSocket();

	connect(HAMLIBsock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(HAMLIBdisplayError(QAbstractSocket::SocketError)));
	connect(HAMLIBsock, SIGNAL(readyRead()), this, SLOT(HAMLIBreadyRead()));
	connect(HAMLIBsock, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onHAMLIBSocketStateChanged(QAbstractSocket::SocketState)));

	HAMLIBsock->connectToHost(HamLibHost, HamLibPort);

	return;
}

void QtTermTCP::HAMLIBSetPTT(int PTTState)
{
	char Msg[16];

	if (HAMLIBsock == nullptr || HAMLIBsock->state() != QAbstractSocket::ConnectedState)
		ConnecttoHAMLIB();

	if (HAMLIBsock == nullptr || HAMLIBsock->state() != QAbstractSocket::ConnectedState)
		return;

	sprintf(Msg, "T %d\r\n", PTTState);
	HAMLIBsock->write(Msg);

	HAMLIBsock->waitForBytesWritten(3000);

	QByteArray datas = HAMLIBsock->readAll();

	qDebug(datas.data());
}

QTcpSocket * FLRigsock;
int FLRigConnected = 0;
int FLRigConnecting = 0;

void QtTermTCP::FLRigdisplayError(QAbstractSocket::SocketError socketError)
{
	switch (socketError)
	{
	case QAbstractSocket::RemoteHostClosedError:
		break;

	case QAbstractSocket::HostNotFoundError:
		QMessageBox::information(nullptr, tr("QtSM"),
			"FLRig host was not found. Please check the "
			"host name and portsettings->");

		break;

	case QAbstractSocket::ConnectionRefusedError:

		qDebug() << "FLRig Connection Refused";
		break;

	default:

		qDebug() << "FLRig Connection Failed";
		break;

	}

	FLRigConnecting = 0;
	FLRigConnected = 0;
}

void QtTermTCP::FLRigreadyRead()
{
	unsigned char Buffer[4096];
	QTcpSocket* Socket = static_cast<QTcpSocket*>(QObject::sender());

	// read the data from the socket. Don't do anyhing with it at the moment

	Socket->read((char *)Buffer, 4095);
}

void QtTermTCP::onFLRigSocketStateChanged(QAbstractSocket::SocketState socketState)
{
	if (socketState == QAbstractSocket::UnconnectedState)
	{
		// Close any connections

		FLRigConnected = 0;
		FLRigConnecting = 0;

		//	delete (FLRigsock);
		//	FLRigsock = 0;

		qDebug() << "FLRig Connection Closed";

	}
	else if (socketState == QAbstractSocket::ConnectedState)
	{
		FLRigConnected = 1;
		FLRigConnecting = 0;
		qDebug() << "FLRig Connected";
	}
}


void QtTermTCP::ConnecttoFLRig()
{
	delete(FLRigsock);

	FLRigConnected = 0;
	FLRigConnecting = 1;

	FLRigsock = new QTcpSocket();

	connect(FLRigsock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(FLRigdisplayError(QAbstractSocket::SocketError)));
	connect(FLRigsock, SIGNAL(readyRead()), this, SLOT(FLRigreadyRead()));
	connect(FLRigsock, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onFLRigSocketStateChanged(QAbstractSocket::SocketState)));

	FLRigsock->connectToHost(FLRigHost, FLRigPort);

	return;
}

static char MsgHddr[] = "POST /RPC2 HTTP/1.1\r\n"
"User-Agent: XMLRPC++ 0.8\r\n"
"Host: 127.0.0.1:7362\r\n"
"Content-Type: text/xml\r\n"
"Content-length: %d\r\n"
"\r\n%s";

static char Req[] = "<?xml version=\"1.0\"?>\r\n"
"<methodCall><methodName>%s</methodName>\r\n"
"%s"
"</methodCall>\r\n";

void QtTermTCP::FLRigSetPTT(int PTTState)
{
	int Len;
	char ReqBuf[512];
	char SendBuff[512];
	char ValueString[256] = "";

	sprintf(ValueString, "<params><param><value><i4>%d</i4></value></param></params\r\n>", PTTState);

	Len = sprintf(ReqBuf, Req, "rig.set_ptt", ValueString);
	Len = sprintf(SendBuff, MsgHddr, Len, ReqBuf);

	if (FLRigsock == nullptr || FLRigsock->state() != QAbstractSocket::ConnectedState)
		ConnecttoFLRig();

	if (FLRigsock == nullptr || FLRigsock->state() != QAbstractSocket::ConnectedState)
		return;

	FLRigsock->write(SendBuff);

	FLRigsock->waitForBytesWritten(3000);

	QByteArray datas = FLRigsock->readAll();

	qDebug(datas.data());
}



void QtTermTCP::CATChanged(bool State)
{
	UNUSED(State);
	PTTPortChanged(0);
}

void QtTermTCP::VARAHFChanged(bool State)
{
	Dev->HFMode->setVisible(State);

	if (State)
	{
		Dev->TNCInfo->setTitle("VARA HF Paramters");
		Dev->Host->setText(VARAHostHF);
		Dev->Port->setText(QString::number(VARAPortHF));
		Dev->Path->setText(VARAPathHF);
	}
}

void QtTermTCP::VARAFMChanged(bool State)
{
	if (State)
	{
		Dev->TNCInfo->setTitle("VARA FM Paramters");
		Dev->Host->setText(VARAHostFM);
		Dev->Port->setText(QString::number(VARAPortFM));
		Dev->Path->setText(VARAPathFM);
	}
}

void QtTermTCP::VARASATChanged(bool State)
{
	if (State)
	{
		Dev->TNCInfo->setTitle("VARA SAT Paramters");
		Dev->Host->setText(VARAHostSAT);
		Dev->Port->setText(QString::number(VARAPortSAT));
		Dev->Path->setText(VARAPathSAT);
	}
}

void QtTermTCP::SetVARAParams()
{
	Dev->Host->setText(VARAHost);
	Dev->Port->setText(QString::number(VARAPortNum));
	Dev->Path->setText(VARAPath);
	Dev->InitCommands->setText(VARAInit);
}

void QtTermTCP::PTTPortChanged(int Selected)
{
	UNUSED(Selected);

	QVariant Q = Dev->PTTPort->currentText();
	strcpy(NewPTTPort, Q.toString().toUtf8());

	Dev->RTSDTR->setVisible(false);
	Dev->CAT->setVisible(false);

	Dev->PTTOnLab->setVisible(false);
	Dev->PTTOn->setVisible(false);
	Dev->PTTOff->setVisible(false);
	Dev->PTTOffLab->setVisible(false);
	Dev->CATLabel->setVisible(false);
	Dev->CATSpeed->setVisible(false);
	Dev->CATHex->setVisible(false);
	Dev->CATText->setVisible(false);

	Dev->GPIOLab->setVisible(false);
	Dev->GPIOLeft->setVisible(false);
	Dev->GPIORight->setVisible(false);
	Dev->GPIOLab2->setVisible(false);

	Dev->CM108Label->setVisible(false);
	Dev->VIDPID->setVisible(false);

	if (strcmp(NewPTTPort, "None") == 0)
	{
	}
	else if (strcmp(NewPTTPort, "GPIO") == 0)
	{
		Dev->GPIOLab->setVisible(true);
		Dev->GPIOLeft->setVisible(true);
	}

	else if (strcmp(NewPTTPort, "CM108") == 0)
	{
		Dev->CM108Label->setVisible(true);
#ifdef WIN32
		Dev->CM108Label->setText("CM108 VID/PID");
#else
		Dev->CM108Label->setText("CM108 Device");
#endif
		Dev->VIDPID->setText(CM108Addr);
		Dev->VIDPID->setVisible(true);
	}
	else if (strcmp(NewPTTPort, "HAMLIB") == 0)
	{
		Dev->CM108Label->setVisible(true);
		Dev->CM108Label->setText("rigctrld Port");
		Dev->VIDPID->setText(QString::number(HamLibPort));
		Dev->VIDPID->setVisible(true);
		Dev->PTTOnLab->setText("rigctrld Host");
		Dev->PTTOnLab->setVisible(true);
		Dev->PTTOn->setText(HamLibHost);
		Dev->PTTOn->setVisible(true);
	}
	else if (strcmp(NewPTTPort, "FLRIG") == 0)
	{
		Dev->CM108Label->setVisible(true);
		Dev->CM108Label->setText("FLRig Port");
		Dev->VIDPID->setText(QString::number(FLRigPort));
		Dev->VIDPID->setVisible(true);
		Dev->PTTOnLab->setText("FLRig Host");
		Dev->PTTOnLab->setVisible(true);
		Dev->PTTOn->setText(FLRigHost);
		Dev->PTTOn->setVisible(true);
	}

	else
	{
		Dev->RTSDTR->setVisible(true);
		Dev->CAT->setVisible(true);

		if (Dev->CAT->isChecked())
		{
			Dev->CATHex->setVisible(true);
			Dev->CATText->setVisible(true);
			Dev->PTTOnLab->setVisible(true);
			Dev->PTTOnLab->setText("PTT On String");
			Dev->PTTOn->setText(PTTOnString);
			Dev->PTTOn->setVisible(true);
			Dev->PTTOff->setVisible(true);
			Dev->PTTOffLab->setVisible(true);
			Dev->PTTOff->setVisible(true);
			Dev->PTTOff->setText(PTTOffString);
			Dev->CATLabel->setVisible(true);
			Dev->CATSpeed->setVisible(true);
			Dev->CATSpeed->setText(QString::number(PTTBAUD));
		}
	}
}



void DecodeCM108(char * ptr)
{
	// Called if Device Name or PTT = Param is CM108

#ifdef WIN32

	// Next Param is VID and PID - 0xd8c:0x8 or Full device name
	// On Windows device name is very long and difficult to find, so 
	//	easier to use VID/PID, but allow device in case more than one needed

	char * next;
	long VID = 0, PID = 0;
	char product[256] = "Unknown";

	struct hid_device_info *devs, *cur_dev;
	const char *path_to_open = NULL;
	hid_device *handle = NULL;

	if (strlen(ptr) > 16)
		CM108Device = _strdup(ptr);
	else
	{
		VID = strtol(ptr, &next, 0);
		if (next)
			PID = strtol(++next, &next, 0);

		// Look for Device

		devs = hid_enumerate((unsigned short)VID, (unsigned short)PID);
		cur_dev = devs;

		while (cur_dev)
		{
			if (cur_dev->product_string)
				wcstombs(product, cur_dev->product_string, 255);

			printf("HID Device %s VID %X PID %X", product, cur_dev->vendor_id, cur_dev->product_id);
			if (cur_dev->vendor_id == VID && cur_dev->product_id == PID)
			{
				path_to_open = cur_dev->path;
				break;
			}
			cur_dev = cur_dev->next;
		}

		if (path_to_open)
		{
			handle = hid_open_path(path_to_open);

			if (handle)
			{
				hid_close(handle);
				CM108Device = _strdup(path_to_open);
			}
			else
			{
				printf("Unable to open CM108 device %x %x", VID, PID);
			}
		}
		else
			printf("Couldn't find CM108 device %x %x", VID, PID);

		hid_free_enumeration(devs);
	}
#else

	// Linux - Next Param HID Device, eg /dev/hidraw0

	CM108Device = strdup(ptr);
#endif
}


void QtTermTCP::OpenPTTPort()
{
	PTTMode &= ~PTTCM108;
	PTTMode &= ~PTTHAMLIB;
	PTTMode &= ~PTTFLRIG;

	if (PTTPort[0] && strcmp(PTTPort, "None") != 0)
	{
		if (PTTMode == PTTCAT)
		{
			// convert config strings from Hex

			if (CATHex == 0)		// Ascii Strings
			{
				strcpy((char *)PTTOffCmd, PTTOffString);
				PTTOffCmdLen = strlen(PTTOffString);

				strcpy((char *)PTTOnCmd, PTTOnString);
				PTTOnCmdLen = strlen(PTTOnString);
			}
			else
			{
				char * ptr1 = PTTOffString;
				unsigned char * ptr2 = PTTOffCmd;
				char c;
				int val;

				while ((c = *(ptr1++)))
				{
					val = c - 0x30;
					if (val > 15) val -= 7;
					val <<= 4;
					c = *(ptr1++) - 0x30;
					if (c > 15) c -= 7;
					val |= c;
					*(ptr2++) = val;
				}

				PTTOffCmdLen = ptr2 - PTTOffCmd;

				ptr1 = PTTOnString;
				ptr2 = PTTOnCmd;

				while ((c = *(ptr1++)))
				{
					val = c - 0x30;
					if (val > 15) val -= 7;
					val <<= 4;
					c = *(ptr1++) - 0x30;
					if (c > 15) c -= 7;
					val |= c;
					*(ptr2++) = val;
				}

				PTTOnCmdLen = ptr2 - PTTOnCmd;
			}
		}

		if (strcmp(PTTPort, "GPIO") == 0)
		{
			// Initialise GPIO for PTT if available

#ifdef __ARM_ARCH

//			if (gpioInitialise() == 0)
//			{
//				printf("GPIO interface for PTT available\n");
//				gotGPIO = TRUE;

//				SetupGPIOPTT();
//			}
//			else
//				printf("Couldn't initialise GPIO interface for PTT\n");
//
#else
			printf("GPIO interface for PTT not available on this platform\n");
#endif

		}
		else if (strcmp(PTTPort, "CM108") == 0)
		{
			DecodeCM108(CM108Addr);
			PTTMode |= PTTCM108;
		}

		else if (strcmp(PTTPort, "HAMLIB") == 0)
		{
			PTTMode |= PTTHAMLIB;
			HAMLIBSetPTT(0);			// to open port
			return;
		}

		else if (strcmp(PTTPort, "FLRIG") == 0)
		{
			PTTMode |= PTTFLRIG;
			FLRigSetPTT(0);			// to open port
			return;
		}

		else		//  Serial Port
		{
#ifdef USESERIAL
			hPTTDevice = new QSerialPort(this);
			hPTTDevice->setPortName(PTTPort);
			hPTTDevice->setBaudRate(PTTBAUD);
			hPTTDevice->setDataBits(QSerialPort::Data8);
			hPTTDevice->setParity(QSerialPort::NoParity);
			hPTTDevice->setStopBits(QSerialPort::OneStop);
			hPTTDevice->setFlowControl(QSerialPort::NoFlowControl);
			if (hPTTDevice->open(QIODevice::ReadWrite))
			{
				qDebug() << "PTT Port Opened";
			}
			else
			{
				QMessageBox msgBox;
				msgBox.setText("PTT COM Port Open Failed.");
				msgBox.exec();
				qDebug() << "PTT Port Open failed";
				delete(hPTTDevice);
				hPTTDevice = 0;
			}
#endif
		}
	}
}

void ClosePTTPort()
{
#ifdef USESERIAL
	if (hPTTDevice)
		hPTTDevice->close();
	hPTTDevice = 0;
#endif
}


void CM108_set_ptt(int PTTState)
{
	unsigned char io[5];
	int n;

	io[0] = 0;
	io[1] = 0;
	io[2] = 1 << (3 - 1);
	io[3] = PTTState << (3 - 1);
	io[4] = 0;

	if (CM108Device == NULL)
		return;

#ifdef WIN32
	hid_device *handle;

	handle = hid_open_path(CM108Device);

	if (!handle) {
		printf("unable to open device\n");
		return;
	}

	n = hid_write(handle, io, 5);
	if (n < 0)
	{
		printf("Unable to write()\n");
		printf("Error: %ls\n", hid_error(handle));
	}

	hid_close(handle);

#else

	int fd;

	fd = open(CM108Device, O_WRONLY);

	if (fd == -1)
	{
		printf("Could not open %s for write, errno=%d\n", CM108Device, errno);
		return;
	}

	io[0] = 0;
	io[1] = 0;
	io[2] = 1 << (3 - 1);
	io[3] = PTTState << (3 - 1);
	io[4] = 0;

	n = write(fd, io, 5);
	if (n != 5)
	{
		printf("Write to %s failed, n=%d, errno=%d\n", CM108Device, n, errno);
	}

	close(fd);
#endif
	return;

}



void QtTermTCP::RadioPTT(bool PTTState)
{
#ifdef __ARM_ARCH
	if (useGPIO)
	{
		//		gpioWrite(pttGPIOPin, (pttGPIOInvert ? (1 - PTTState) : (PTTState)));
		//		return;
	}

#endif

	if ((PTTMode & PTTCM108))
	{
		CM108_set_ptt(PTTState);
		return;
	}

	if ((PTTMode & PTTHAMLIB))
	{
		HAMLIBSetPTT(PTTState);
		return;
	}

	if ((PTTMode & PTTFLRIG))
	{
		FLRigSetPTT(PTTState);
		return;
	}

#ifdef USESERIAL

	if (hPTTDevice == 0)
		return;

	if ((PTTMode & PTTCAT))
	{
		if (PTTState)
			hPTTDevice->write((char *)PTTOnCmd, PTTOnCmdLen);
		else
			hPTTDevice->write((char *)PTTOffCmd, PTTOffCmdLen);

		hPTTDevice->flush();
		//		hPTTDevice->error();
		return;

	}


	if ((PTTMode & PTTRTS))
	{
		int n = hPTTDevice->setRequestToSend(PTTState);
		n = n;
	}

#endif

}



extern "C" void WriteDebugLog(char * Mess)
{
	qDebug() << Mess;
}

void QtTermTCP::ConnecttoKISS()
{
	if (strcmp(SerialPort, "TCP") == 0)
	{
		delete(KISSSock);

		KISSSock = new myTcpSocket();
		KISSSockCopy[0] = (void *)KISSSock;


		connect(KISSSock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(KISSdisplayError(QAbstractSocket::SocketError)));
		connect(KISSSock, SIGNAL(readyRead()), this, SLOT(KISSreadyRead()));
		connect(KISSSock, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onKISSSocketStateChanged(QAbstractSocket::SocketState)));

		KISSSock->connectToHost(KISSHost, KISSPortNum);

		Status3->setText("KISS Connecting");
	}
	else
		openSerialPort();

	return;
}


void QtTermTCP::KISSTimer()
{
	// Runs every 10 Seconds

	if (KISSConnected == 0 && KISSConnecting == 0)
	{
		ConnecttoKISS();
	}
	else
	{
		// Verify Serial port is still ok

 		if (m_serial && KISSConnected)
		{
			m_serial->clearError();
			boolean rc = m_serial->isDataTerminalReady();

			if (m_serial->error())
			{
				Debugprintf("Serial Port Lost - isOpen %d Error %d", m_serial->isOpen(), m_serial->error());
				closeSerialPort();
			}
		}
	}
}



void QtTermTCP::KISSdisplayError(QAbstractSocket::SocketError socketError)
{
	switch (socketError)
	{
	case QAbstractSocket::RemoteHostClosedError:
		break;

	case QAbstractSocket::HostNotFoundError:
		QMessageBox::information(this, tr("QtTermTCP"),
			tr("KISS host was not found. Please check the "
				"host name and portsettings->"));

		Status3->setText("KISS Connection Failed");

		break;

	case QAbstractSocket::ConnectionRefusedError:

		Status3->setText("KISS Connection Refused");
		break;

	default:

		Status3->setText("KISS Connection Failed");
	}

	KISSConnecting = 0;
	KISSConnected = 0;
}


extern "C" void KISSSendtoServer(myTcpSocket* Socket, char * Data, int Length)
{
	if (m_serial)
	{
		if (m_serial->isOpen())
		{
			m_serial->clearError();

			int n = m_serial->write(Data, Length);

			n = m_serial->flush();

			if (m_serial->error())
			{
				Debugprintf("Serial Flush Error - Requested = %d Actual %d Error %d", Length, n, m_serial->error());
				closeSerialPort();
			}
		}
	}
	else if (Socket)
		Socket->write(Data, Length);
}



void QtTermTCP::KISSreadyRead()
{
	int Read;
	unsigned char Buffer[4096];	myTcpSocket* Socket = static_cast<myTcpSocket*>(QObject::sender());

	// read the data from the socket

	Read = Socket->read((char *)Buffer, 4095);

	KISSDataReceived(Socket, Buffer, Read);

}

extern "C" void KISS_del_socket(void * socket);
extern "C" void KISS_add_stream(void * Socket);

void QtTermTCP::onKISSSocketStateChanged(QAbstractSocket::SocketState socketState)
{
	//	myTcpSocket* sender = static_cast<myTcpSocket*>(QObject::sender());

	QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());

	if (socketState == QAbstractSocket::UnconnectedState)
	{
		// Close any connections

		Ui_ListenSession * Sess = NULL;

		Status3->setText("KISS Disconnected");
		actHost[18]->setEnabled(0);

		KISSConnecting = KISSConnected = 0;

		// Free the monitor Window

		if (KISSMonSess)
		{
			Sess = KISSMonSess;

			if (TermMode == MDI)
				Sess->setWindowTitle("Monitor Session Disconnected");

			else if (TermMode == Tabbed)
				tabWidget->setTabText(Sess->Tab, "Monitor");

			KISSMonSess = nullptr;
		}

		KISS_del_socket(sender);
		KISSSock = NULL;
	}
	else if (socketState == QAbstractSocket::ConnectedState)
	{
		int i;

		KISSConnected = 1;
		KISSConnecting = 0;

		Status3->setText("KISS Connected");
		actHost[18]->setEnabled(1);			// Enable KISS Connect Line

		KISS_add_stream(sender);

		// Attach a Monitor Window if available

		Ui_ListenSession * Sess = NULL;
		Ui_ListenSession * S;

		if (TermMode == MDI)
		{
			// See if an old session can be reused

			for (int i = 0; i < _sessions.size(); ++i)
			{
				S = _sessions.at(i);

				//	for (Ui_ListenSession * S: _sessions)
				//	{
				if ((S->SessionType == Mon) && S->clientSocket == NULL && S->KISSSession == NULL && (AGWUsers == NULL || (S != AGWUsers->MonSess))  && S != KISSMonSess)
				{
					Sess = S;
					break;
				}
			}

			// Create a window if none found, else reuse old

			if (Sess == NULL)
			{
				Sess = newWindow((QObject *)mythis, Mon, "");
			}
		}
		else if (TermMode == Tabbed)
		{
			// Tabbed - look for free session

			for (i = 8; i; i--)
			{
				S = _sessions.at(i);

				if (S->clientSocket == NULL && S->KISSSession == NULL  && S->AGWSession == NULL && (AGWUsers == NULL || (S != AGWUsers->MonSess)) && S != KISSMonSess)
				{
					Sess = S;
					break;
				}
			}
		}
		else if (TermMode == Single && (singlemodeFormat & Mon))
		{
			S = _sessions.at(0);

			if (S->clientSocket == NULL && S->AGWSession == NULL && S->KISSSession == NULL && (AGWUsers == NULL || (S != AGWUsers->MonSess)) && S != KISSMonSess)
				Sess = S;

		}

		if (Sess)
		{
			KISSMonSess = Sess;			// Flag as in use

			if (TermMode == MDI)
				Sess->setWindowTitle("KISS Monitor Window");
			else if (TermMode == Tabbed)
				tabWidget->setTabText(Sess->Tab, "KISS Mon");
			else if (TermMode == Single)
				mythis->setWindowTitle("KISS Monitor Window");

			Sess->mlocaltime = KISSLocalTime;
			Sess->MonitorNODES = KISSMonNodes;

			//			if (TermMode == Single)	
			//			{
			//				discAction->setEnabled(false);
			//				YAPPSend->setEnabled(false);
			//				connectMenu->setEnabled(false);
			//			}
		}
	}
}


extern "C" char * frame_monitor(string * frame, char * code, bool tx_stat);
extern "C" char * ShortDateTime();

extern "C" void monitor_frame(int snd_ch, string * frame, char * code, int  tx, int excluded)
{
	UNUSED(excluded);
	UNUSED(snd_ch);

	int Len;
	char Msg[1024];

	if (tx)
		sprintf(Msg, "\x1b\x10%s", frame_monitor(frame, code, tx));
	else
		sprintf(Msg, "\x1b\x11%s", frame_monitor(frame, code, tx));

	Len = strlen(Msg);

	if (Len < 10)		// Suppressed NODES
		return;

	if (Msg[Len - 1] != '\r')
	{
		Msg[Len++] = '\r';
		Msg[Len] = 0;
	}

	if (KISSMonSess)
		WritetoMonWindow(KISSMonSess, (unsigned char *)Msg, Len);

}

extern "C" Ui_ListenSession * ax25IncomingConnect(TAX25Port * AX25Sess)
{
	// Look for/create Terminal Window for connection

	Ui_ListenSession * Sess = NULL;
	Ui_ListenSession * S;
	char Title[80];
	int i = 0;

	if (TermMode == MDI)
	{
		// See if an old session can be reused

		for (int i = 0; i < _sessions.size(); ++i)
		{
			S = _sessions.at(i);

			if ((S->SessionType & Listen) && S->clientSocket == NULL && S->AGWSession == NULL && S->KISSSession == NULL)
			{
				Sess = S;
				break;
			}
		}

		// Create a window if none found, else reuse old

		if (Sess == NULL)
		{
			Sess = newWindow((QObject *)mythis, Listen, "");
		}
	}
	else
	{
		// Single or Tabbed - look for free session


		for (i = 0; i < _sessions.size(); ++i)
		{
			S = _sessions.at(i);

			if (S->clientSocket == NULL && S->AGWSession == NULL && S->KISSSession == NULL)
			{
				Sess = S;
				break;
			}
		}

		if (Sess == NULL)
		{
			// Clear connection

			return NULL;
		}
	}

	if (Sess)
	{
		sprintf(Title, "Connected to %s", AX25Sess->corrcall);

		if (TermMode == MDI)
		{
			Sess->setWindowTitle(Title);
		}
		else if (TermMode == Tabbed)
		{
			tabWidget->setTabText(i, AX25Sess->corrcall);
			tabWidget->tabBar()->setTabTextColor(i, newTabText);
		}
		else if (TermMode == Single)
			mythis->setWindowTitle(Title);

		AX25Sess->port = 0;
		AX25Sess->Sess = Sess;				// Crosslink KISS and Term Sessions
		AX25Sess->PID = 240;;

		Sess->KISSSession = AX25Sess;

		setMenus(true);

		if (ConnectBeep)
			myBeep(&ConnectWAV);

		QApplication::alert(mythis, 0);

		// Send CText if defined

		if (listenCText[0])
			SendtoAX25(Sess->KISSSession, (unsigned char *)listenCText, (int)strlen(listenCText));
	}
	return Sess;
}


extern "C" void AX25_disc(TAX25Port * AX25Sess, Byte mode)
{
	char Msg[128];
	int Len = 0;
	Ui_ListenSession * Sess = (Ui_ListenSession *)AX25Sess->Sess;

	if (AX25Sess->status == STAT_TRY_LINK)
	{
		// Connect failed

		Len = sprintf(Msg, "Connection to %s failed\r", AX25Sess->corrcall);
	}
	else
	{
		switch (mode)
		{
		case MODE_OTHER:
		case MODE_OUR:

			Len = sprintf(Msg, "Disconnected from %s\r", AX25Sess->corrcall);
			break;

		case MODE_RETRY:

			Len = sprintf(Msg, "Disconnected from %s - Retry count exceeded\r", AX25Sess->corrcall);
			break;

		};
	}

	SendtoTerm(Sess, Msg, Len);
	ClearSessLabel(Sess);
	Sess->KISSSession = NULL;
	AX25Sess->Sess = 0;

	setMenus(0);
};

int QtTermTCP::openSerialPort()
{
	if (m_serial && m_serial->isOpen())
	{
		m_serial->close();
	}

	m_serial = nullptr;
	m_serial = new QSerialPort(this);

	m_serial->setPortName(SerialPort);
	boolean ok = m_serial->setBaudRate(KISSBAUD);

	if (m_serial->open(QIODevice::ReadWrite))
	{
		int i;

		ok = m_serial->setRequestToSend(true);

		connect(m_serial, &QSerialPort::readyRead, this, &QtTermTCP::readSerialData);
		//		connect(m_serial, &QSerialPort::errorOccurred, this, &QtTermTCP::handleError);

		KISSConnected = 1;
		KISSConnecting = 0;

		Status3->setText("KISS Connected");
		actHost[18]->setEnabled(1);			// Enable KISS Connect Line

		KISS_add_stream(m_serial);

		// Attach a Monitor Window if available

		Ui_ListenSession * Sess = NULL;
		Ui_ListenSession * S;

		if (TermMode == MDI)
		{
			// See if an old session can be reused

			for (int i = 0; i < _sessions.size(); ++i)
			{
				S = _sessions.at(i);

				//	for (Ui_ListenSession * S: _sessions)
				//	{
				if ((S->SessionType == Mon) && S->clientSocket == NULL && S->KISSSession == NULL)
				{
					Sess = S;
					break;
				}
			}

			// Create a window if none found, else reuse old

			if (Sess == NULL)
			{
				Sess = newWindow((QObject *)mythis, Mon, "");
			}
		}
		else if (TermMode == Tabbed)
		{
			// Tabbed - look for free session

			for (i = 8; i; i--)
			{
				S = _sessions.at(i);

				if (S->clientSocket == NULL && S->KISSSession == NULL)
				{
					Sess = S;
					break;
				}
			}
		}
		else if (TermMode == Single && (singlemodeFormat & Mon))
		{
			S = _sessions.at(0);

			if (S->clientSocket == NULL && S->KISSSession == NULL)
				Sess = S;

		}

		if (Sess)
		{
			KISSMonSess = Sess;			// Flag as in use

			if (TermMode == MDI)
				Sess->setWindowTitle("KISS Monitor Window");
			else if (TermMode == Tabbed)
				tabWidget->setTabText(Sess->Tab, "KISS Mon");
			else if (TermMode == Single)
				mythis->setWindowTitle("KISS Monitor Window");

			//			if (TermMode == Single)	
			//			{
			//				discAction->setEnabled(false);
			//				YAPPSend->setEnabled(false);
			//				connectMenu->setEnabled(false);
			//			}
		}



		return 1;
	}
	else
	{
		Status3->setText("KISS Open Failed");
		KISSConnected = 0;
		KISSConnecting = 0;
		return 0;
	}
}



void closeSerialPort()
{
	if (m_serial && m_serial->isOpen())
	{
		m_serial->close();
		m_serial = nullptr;
	}

	m_serial = nullptr;

	KISSConnected = 0;
	KISSConnecting = 0;

	Status3->setText("KISS Closed");
	actHost[18]->setEnabled(0);			// Enable KISS Connect Line
}

void QtTermTCP::readSerialData()
{
	int Read;
	unsigned char Buffer[8192];

	// read the data from the socket

	m_serial->clearError();

	Read = m_serial->read((char *)Buffer, 2047);

	if (m_serial->error())
	{
		Debugprintf("Serial Read Error - RC %d Error %d", Read, m_serial->error());
		closeSerialPort();
		return;
	}


	while (Read > 0)
	{
		KISSDataReceived(m_serial, Buffer, Read);
		Read = m_serial->read((char *)Buffer, 2047);
	}
}

void QtTermTCP::handleError(QSerialPort::SerialPortError serialPortError)
{
	Debugprintf("Serial port Error %d", serialPortError);
	closeSerialPort();
}

extern "C" void CheckUIFrame(unsigned char * path, string * data)
{
	// If we have KISS enabled and dest is UIDEST look for a KISS window in UI Mode

	if (KISSSock == 0)
		return;

	char Dest[10];
	char From[10];

	Dest[ConvFromAX25(path, Dest)] = 0;
	From[ConvFromAX25(&path[7], From)] = 0;

	// ok, Find a Kiss Session with this Dest

	Ui_ListenSession * Sess = NULL;

	for (int i = 0; i < _sessions.size(); ++i)
	{
		Sess = _sessions.at(i);

		if (Sess->KISSMode == 1 && strcmp(Dest, Sess->UIDEST) == 0)
		{
			char Msg[512];
			int Len;

			data->Data[data->Length] = 0;

			Len = sprintf(Msg, "%s:%s", From, data->Data);
			SendtoTerm(Sess, Msg, Len);
			return;
		}
	}

}


QColor QtTermTCP::setColor(QColor Colour)
{
	//    const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());


	QColor col = Colour;

	QColorDialog dialog;
	dialog.setCurrentColor(Colour);
	dialog.setOption(QColorDialog::DontUseNativeDialog);

	if (dialog.exec() == QColorDialog::Accepted)
		col = QVariant(dialog.currentColor()).toString();


	//	const QColor color = QColorDialog::getColor(Qt::green, this, "Select Color", 0);

	return col;
}

// Experimental Viewdata/Teletext/Prestel/CEEFAX Mode

// Uses ideas and some code from 

/***************************************************************
 * Name:      wxTEDMain.cpp
 * Purpose:   Teletext editor Application Frame
 * Author:    Peter Kwan (peterk.vt80@gmail.com)
 * Created:   2014-10-30
 * Copyright: Peter Kwan
 * License:
  *
 * Copyright (C) 2014-2022, Peter Kwan
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaims all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 *************************************************************************** **/



int FontWidth = 16;
int FontHeight = 16;

bool isMosaic(char ch)
{
	ch &= 0x7f;
	return (ch >= 0x20 && ch < 0x40) || ch >= 0x60;
}


void DecodeTeleText(Ui_ListenSession * Sess, char * page)
{

	// redraw the whole teletext page

	QColor fg = Qt::white;
	QColor bg = Qt::black;
	QColor holdColour;
	QColor holdfg = fg;			// Colour for held graphic
	int usehold = 0;
	int sep = 0;

	QPainter p(Sess->TTBitmap);

	QSettings settings(GetConfPath(), QSettings::IniFormat);

  //p.setFont(QFont("Courier", 14, QFont::Bold));

	p.setFont(QFont(settings.value("FontFamily", "Courier New").value<QString>(), 14));

	p.setPen(QPen(fg));

	bool graphicsMode = false;
	bool separated = false;
	bool doubleHeight = false;
	int skipnextrow = 99;
	bool flashing = false;
	bool hold = false;
	char holdChar = 0;
	char holdMode = 0;
	Sess->timer.stop();

	bool concealed = false;
	char c;

	fg = Qt::white;
	bg = Qt::black;

	char * ptr = page;

	int col = 0;
	int line = 0;

	// XXXXXXXXTEEFAX %%# %%a %d %%b C %H:%M.%S

	//p.drawText(0, 19, "P199            HAMFAX");


	// interpret data, for now, line by line


	while ((c = *(ptr++)))
	{
		char ch = c;

		if (c == 0x11)			// Curson On ??
			continue;

		if (c == 0x14)			// Curson Off ??
			continue;

		if (c == 9)			// Curson On ??
		{
			col++;
			continue;
		}

		if (c == 0x0a)
		{
			line++;

//			if (doubleHeight)
//				line++;

			if (line > 24)
				line = 0;

			continue;
		}

		if (c == 0x1e)			// Cursor Home
		{
			line = col = 0;
			c = 13;					// So we reset page flags below
		}

		if (c == 12)
		{
			// Clear the page

			Sess->TTBitmap->fill(Qt::black);
			Sess->TTLabel->setPixmap(QPixmap::fromImage(*Sess->TTBitmap));

			line = col = 0;
			
			c = 13;					// So we reset page flags below
		}

		if (c == 13)
		{
			col = 0;

			graphicsMode = false;
			separated = false;
			doubleHeight = false;
			flashing = false;
			hold = false;
			holdChar = 0;
			concealed = false;

			fg = Qt::white;
			bg = Qt::black;

			continue;			// Next line
		}

		if (c == 0x1b) // Esc
		{
			// I think the control char is displayed as it was before being actioned, sa save current state

			holdfg = holdColour;						// Colour if using held - may be changed before displaying
			usehold = hold;								// May be changed
			sep = holdMode;

			char echar = *(ptr++);

			if (echar == 0)
			{
				// Esc pair spilt - wait for rest

				Sess->TTLabel->setPixmap(QPixmap::fromImage(*Sess->TTBitmap));
				return;
			}
			
			switch (echar)
			{
			case '@':
				fg = Qt::black;
				concealed = false;    // Side effect of colour. It cancels a conceal.
				graphicsMode = false;
//				hold = false;

				break;
			case 'A':
				fg = Qt::red;
				concealed = false;
				graphicsMode = false;
				hold = false;

				break;
			case 'B':
				fg = Qt::green;
				concealed = false;
				graphicsMode = false;
				hold = false;
				break;
			case 'C':
				fg = Qt::yellow;
				concealed = false;
				graphicsMode = false;
				hold = false;
				break;
			case 'D':
				fg = Qt::blue;
				concealed = false;
				graphicsMode = false;
				hold = false;
				break;
			case 'E':
				fg = Qt::magenta;
				concealed = false;
				graphicsMode = false;
				hold = false;
				break;
			case 'F':
				fg = Qt::cyan;
				concealed = false;
				graphicsMode = false;
				hold = false;
				break;
			case 'G':
				fg = Qt::white;
				concealed = false;
				graphicsMode = false;
				hold = false;
				break;
			case 'H':					// Flash
				flashing = true;
				Sess->timer.start(1000, Sess);
				concealed = false;
				graphicsMode = false;
				hold = false;

				break;

			case 'I':					// Steady
				flashing = false;
				concealed = false;
				graphicsMode = false;
				hold = false;

				break;
				//		
			case 'J':			//ttxCodeEndBox:
			case 'K':			//ttxCodeStartBox:

				concealed = false;
				graphicsMode = false;
				hold = false;
					
				break;
			case 'L': // Normal height
				doubleHeight = false;
				hold = false;  
				break;

			case 'M': // Double height
				doubleHeight = true;
				skipnextrow = line + 1;   // ETSI: row to ignore
				hold = false;
				holdChar = 0;
				break;
			case 'P': // Graphics black
//				concealed = false;
				graphicsMode = true;
				holdColour = fg = Qt::black;

				break;
			case 'Q': // Graphics red
//				concealed = false;
				graphicsMode = true;
				holdColour = fg = Qt::red;

				break;
			case 'R': // Graphics green
//				concealed = false;
				graphicsMode = true;
				holdColour = fg = Qt::green;

				break;
			case 'S': // Graphics yellow
//				concealed = false;
				graphicsMode = true;
				holdColour = fg = Qt::yellow;

				break;
			case 'T': // Graphics blue
//				concealed = false;
				graphicsMode = true;
				holdColour = fg = Qt::blue;

				break;
			case 'U': // Graphics magenta
//				concealed = false;
				graphicsMode = true;
				holdColour = fg = Qt::magenta;

				break;
			case 'V': // Graphics cyan
//				concealed = false;
				graphicsMode = true;
				holdColour = fg = Qt::cyan;

				break;
			case 'W': // Graphics white
//				concealed = false;
				graphicsMode = true;
				holdColour = fg = Qt::white;

				break;

			case 'X': // Conceal display
				
				concealed = 1;
				break;

			case 'Y': // Contiguous graphics

				separated = false;
				break;

			case 'Z': // Separated gfx

				separated = true;
				break;

			case 0x5c: // Background black
				bg = Qt::black;
				break;

			case 0x5d: // New background

				bg = fg;
				break;

			case 0x5e: // Hold gfx

				if (hold == 0)
				{
					hold = true;
					holdColour = fg;
					holdMode = separated;
				}
				break;

			case 0x5f: // Non-hold gfx
				hold = false;
				break;

			default:
				echar++;					// For testign
				break;

			}  //end of esc case processing

			if (usehold)
			{
				// We set sep and color for held char earlier 
			
				ch = holdChar;			// Repeat held char
			}
			else
				ch = 0x0;				// Default is space


			if (line > skipnextrow)
				skipnextrow = 99;

			if (line == skipnextrow)
			{
				line = line;
			}
			else
			{
				if (concealed == 0 && (flashing == 0 || Sess->TTFlashToggle == 0))
				{
					p.fillRect(col * 15, line * 19, 15, 19, bg);

					// if double height also draw background for next row

					if (doubleHeight)
						p.fillRect(col * 15, line * 19 + 19, 15, 19, bg);


					if (sep)
					{
						if (ch & 1)
							p.fillRect(col * 15 + 2, line * 19 + 2, 5, 4, holdfg);
						if (ch & 2)
							p.fillRect(col * 15 + 9, line * 19 + 2, 6, 4, holdfg);
						if (ch & 4)
							p.fillRect(col * 15 + 2, line * 19 + 8, 5, 4, holdfg);
						if (ch & 8)
							p.fillRect(col * 15 + 9, line * 19 + 8, 6, 4, holdfg);
						if (ch & 16)
							p.fillRect(col * 15 + 2, line * 19 + 14, 5, 5, holdfg);
						if (ch & 32)
							p.fillRect(col * 15 + 9, line * 19 + 14, 6, 5, holdfg);

					}
					else
					{
						if (ch & 1)
							p.fillRect(col * 15, line * 19, 7, 6, holdfg);
						if (ch & 2)
							p.fillRect(col * 15 + 7, line * 19, 8, 6, holdfg);
						if (ch & 4)
							p.fillRect(col * 15, line * 19 + 6, 7, 6, holdfg);
						if (ch & 8)
							p.fillRect(col * 15 + 7, line * 19 + 6, 8, 6, holdfg);
						if (ch & 16)
							p.fillRect(col * 15, line * 19 + 12, 7, 7, holdfg);
						if (ch & 32)
							p.fillRect(col * 15 + 7, line * 19 + 12, 8, 7, holdfg);
					}
				}
			}
			col++;
		}
		else
		{
			// Not esc - so normal or graphics

			if (ch < 0x20)
				continue;

			if (line > skipnextrow)
				skipnextrow = 99;

			if (line == skipnextrow)
			{
				line = line;
			}
			else
			{
				if (concealed == 0 && (flashing == 0 || Sess->TTFlashToggle == 0))
				{
					p.fillRect(col * 15, line * 19, 15, 19, bg);

					if (doubleHeight)
						p.fillRect(col * 15, line * 19 + 19, 15, 19, bg);

					p.setPen(QPen(fg));;

					if (graphicsMode)
					{
						if (ch < 0x40)
							ch -= 0x20;
						else
							if (ch >= 0x60)
								ch -= 0x40;
							else
								goto isText;

						holdChar = ch;

						// Now have 00 - 3f 


						// C is bit mask bit posns are
						// 01
						// 23
						// 45
						// Char cell is 15 * 19 which is a bit asymetrical

						// Chars are 20 - 3f and 60 to 7f but I cant see a logic to the mapping


						if (separated)
						{
							if (ch & 1)
								p.fillRect(col * 15 + 2, line * 19 + 2, 5, 4, fg);
							if (ch & 2)
								p.fillRect(col * 15 + 9, line * 19 + 2, 6, 4, fg);
							if (ch & 4)
								p.fillRect(col * 15 + 2, line * 19 + 8, 5, 4, fg);
							if (ch & 8)
								p.fillRect(col * 15 + 9, line * 19 + 8, 6, 4, fg);
							if (ch & 16)
								p.fillRect(col * 15 + 2, line * 19 + 14, 5, 5, fg);
							if (ch & 32)
								p.fillRect(col * 15 + 9, line * 19 + 14, 6, 5, fg);

						}
						else
						{
							if (ch & 1)
								p.fillRect(col * 15, line * 19, 7, 6, fg);
							if (ch & 2)
								p.fillRect(col * 15 + 7, line * 19, 8, 6, fg);
							if (ch & 4)
								p.fillRect(col * 15, line * 19 + 6, 7, 6, fg);
							if (ch & 8)
								p.fillRect(col * 15 + 7, line * 19 + 6, 8, 6, fg);
							if (ch & 16)
								p.fillRect(col * 15, line * 19 + 12, 7, 7, fg);
							if (ch & 32)
								p.fillRect(col * 15 + 7, line * 19 + 12, 8, 7, fg);
						}
					}
					else
					{
						// Just write char at current col and line

					isText:
						char s[5];
						unsigned char su[5] = "";

						// Some chars are in wrong char set

						su[0] = ch;

						if (ch == '_')
							su[0] = '#';

						else if (ch == 0x7e)					// division
						{
							su[0] = 0xC3;
							su[1] = 0xB7;
						}
						else if (ch == 0x5e)			// up arrow
						{
							su[0] = 0xF0;
							su[1] = 0x9F;
							su[2] = 0xA0;
							su[3] = 0x95;
						}
						else if (ch == 0x7f)			// up arrow
						{
							su[0] = 0xE2;
							su[1] = 0x96;
							su[2] = 0x88;
						}

						memcpy(s, su, 5);

//						if (doubleHeight)
	//							p.drawText(col * 15, line * 19 + 25, s);
		//					else
			//					p.drawText(col * 15, line * 19 + 15, s);

						// if double height draw normally then copy pixels each row of pixels to two scanlines (starting at the bottom)

						if (doubleHeight)
						{
							int inscanline = line * 19 + 18;
							int outscanline = line * 19 + 35;
							unsigned char * inptr = Sess->TTBitmap->scanLine(inscanline);
							unsigned char * outptr = Sess->TTBitmap->scanLine(outscanline);
							int linelen = Sess->TTBitmap->bytesPerLine();
							int charlen = linelen / 40;			// bytes per char position

							p.drawText(col * 15, line * 19 + 16, s);

							inptr += col * charlen;
							outptr += col * charlen;

							for (int i = 0; i < 18; i++)
							{
								memcpy(outptr, inptr, charlen);
								outptr -= linelen;
								memcpy(outptr, inptr, charlen);

								inptr -= linelen;
								outptr -= linelen;

							}
						}
						else
							p.drawText(col * 15, line * 19 + 15, s);

					}
				}
			}
			col++;
		}

		if (col > 39)
		{
			col = 0;
			line++;
			if (line > 24)
				line = 0;

			graphicsMode = false;
			separated = false;
			doubleHeight = false;
			flashing = false;
			hold = false;
			holdChar = 0;
			concealed = false;

			fg = Qt::white;
			bg = Qt::black;
		}
	}
	Sess->TTLabel->setPixmap(QPixmap::fromImage(*Sess->TTBitmap));
	return;

	QFile file("D:/savepage.txt");
	file.open(QIODevice::WriteOnly);
	file.write(Sess->pageBuffer);
	file.close();
}


void Ui_ListenSession::timerEvent(QTimerEvent *event)
{
	Ui_ListenSession * Sess = NULL;

	// Used for flashing - resfresh window

	for (int i = 0; i < _sessions.size(); ++i)
	{
		Sess = _sessions.at(i);

		if (Sess->timer.timerId() == event->timerId())
		{
			if (Sess->TTActive)
			{
				Sess->TTFlashToggle ^= 1;

				// if in Tabbed mode only refresh active tab

				if (TermMode == Tabbed && Sess != ActiveSession)
					return;

				DecodeTeleText(Sess, (char *)Sess->pageBuffer);			// Re-decode same data until we get the end
			}
			else
				Sess->timer.stop();

			return;
		}
	}
	QWidget::timerEvent(event);
}

// Monitor Log FIle routines

char * doXMLTransparency(char * string)
{
	// Make sure string doesn't contain forbidden XML chars (<>"'&)

	char * newstring = (char *)malloc(5 * strlen(string) + 1);		// If len is zero still need null terminator

	char * in = string;
	char * out = newstring;
	char c;

	c = *(in++);

	while (c)
	{
		switch (c)
		{
		case '<':

			strcpy(out, "&lt;");
			out += 4;
			break;

		case '>':

			strcpy(out, "&gt;");
			out += 4;
			break;

		case '"':

			strcpy(out, "&quot;");
			out += 6;
			break;

		case '\'':

			strcpy(out, "&apos;");
			out += 6;
			break;

		case '&':

			strcpy(out, "&amp;");
			out += 5;
			break;

		default:

			*(out++) = c;
		}
		c = *(in++);
	}

	*(out++) = 0;
	return newstring;
}

void WriteMonitorLog(Ui_ListenSession * Sess, char * Msg)
{
	// Write as HTML to preserve formatting

	char Line[512];
	char * HTMLText;

	if (Sess->monLogfile == nullptr)
	{
		QString FN = "QTTermMonLog" + timeLoaded + "_" + QString::number(Sess->sessNo) + ".html";
		Sess->monLogfile = new QFile(FN);

		if (Sess->monLogfile)
			Sess->monLogfile->open(QIODevice::WriteOnly | QIODevice::Text);
		else
			return;
	}

	if (Msg[0] == 0x1b)
	{
		// Colour Escape

		if (Msg[1] == 17)
			Sess->monSpan = (char *)"<span style=\"color:blue; white-space:pre;font-family: monospace\">";// , monRxColour.data());
		else
			Sess->monSpan = (char *)"<span style=\"color:red; white-space:pre;font-family: monospace\">";// , monTxColour.data());

		HTMLText = doXMLTransparency(&Msg[2]);
	}
	else
	{
		// Leave colour at last set value

		HTMLText = doXMLTransparency(Msg);
	}

	sprintf(Line, "%s%s</span><br>\r\n", Sess->monSpan, HTMLText);
	
	Sess->monLogfile->write(Line);

	free(HTMLText);

}

