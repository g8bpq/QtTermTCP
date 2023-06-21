#pragma once

#include <QMainWindow>
#include "ui_QtTermTCP.h"
//#include "ui_AGWParams.h"
//#include "ui_AGWConnect.h"
#include "ui_ColourConfig.h"
#include "ui_VARAConfig.h"
#include "ui_KISSConfig.h"
#include "QTextEdit"
#include "QSplitter"
#include "QLineEdit"
#include "QTcpSocket"
#include <QDataStream>
#include <QKeyEvent>
#include "QThread"
#include "QTcpServer"
#include "QMdiArea"
#include <QMdiSubWindow>
#include "QMessageBox"
#include "QTimer"
#include "QSettings"
#include "QThread"
#include <QFontDialog>
#include <QScrollBar>
#include <QFileDialog>
#include <QTabWidget>
#include <QMenuBar>
#include <QProcess>
#include <QtSerialPort/QSerialPort>

#define MAXHOSTS 16
#define MAXPORTS 64

QT_BEGIN_NAMESPACE
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTcpSocket;
class QNetworkSession;

class myTcpSocket : public QTcpSocket
{
public:
	QWidget * Sess;
};


class Ui_ListenSession : public QMainWindow
{
	Q_OBJECT

public:
	explicit Ui_ListenSession(QWidget *Parent = 0) : QMainWindow(Parent) {}
	~Ui_ListenSession();

	int SessionType;				// Type Mask - Term, Mon, Listen
	int CurrentWidth;
	int CurrentHeight;				// Saved so can be restored after Cascade

	QTextEdit *termWindow;
	QTextEdit *monWindow;
	QLineEdit *inputWindow;
	QLabel * TTLabel;

	myTcpSocket *clientSocket;

	QAction * actActivate;			// From active Windows menu

	char * KbdStack[50];
	int StackIndex;

	QMdiSubWindow *sw;				// The MdiSubwindow is the container for this session

	int InputMode;
	int SlowTimer;
	int MonData;

	int OutputSaveLen;
	char OutputSave[16384];

	int MonSaveLen;
	char MonSave[4096];

	char PortMonString[1024];		// 32 ports 32 Bytes
	unsigned long long portmask;
	int mtxparam;
	int mcomparam;
	int monUI;
	int MonitorNODES;
	int MonitorColour;
	int CurrentHost;
	int Tab;						// Tab Index if Tabbed Mode
	void * AGWSession;				// Terinal sess - Need to cast to TAGWPort to use it
	void * AGWMonSession;
	void * KISSSession;
	int KISSMode;					// Connected or UI
	char UIDEST[32];
	char UIPATH[128];

	// For Teletext Emulator

	QImage * TTBitmap;    // TT Image buffer
	QDialog TTUI;

	int TTActive;
	int TTFlashToggle;
	char pageBuffer[4096];
	QBasicTimer timer;

protected:

	void timerEvent(QTimerEvent *event) override;

private:



private slots:

};


class QtTermTCP : public QMainWindow
{
	Q_OBJECT

public:
	QtTermTCP(QWidget *parent = NULL);
	void closeEvent(QCloseEvent * event);
	static void setFonts();
	~QtTermTCP();

private slots:
	void Disconnect();
	void doYAPPSend();
	void doYAPPSetRX();
	void menuChecked();
	void Connect();
	void displayError(QAbstractSocket::SocketError socketError);
	void readyRead();

	void LreturnPressed(Ui_ListenSession * LUI);
	void LDisconnect(Ui_ListenSession * LUI);
	void SetupHosts();
	void MyTimerSlot();
	void KISSTimerSlot();
	void ListenSlot();
	void AGWSlot();
	void VARASlot();
	void KISSSlot();
	void deviceaccept();
	void KISSaccept();
	void KISSreject();
	void devicereject();
	void showContextMenuM(const QPoint &pt);
	void showContextMenuT(const QPoint &pt);
	void showContextMenuL();
	void doQuit();
	void onTEselectionChanged();
	void onLEselectionChanged();
	void setSplit();
	void ClearScreen();
	void setVDMode();
	void showContextMenuMT(const QPoint & pt);
	void showContextMenuMOnly(const QPoint & pt);
	void onNewConnection();
	void onSocketStateChanged(QAbstractSocket::SocketState socketState);
	void updateWindowMenu();
	void doNewTerm();
	void doNewMon();
	void doNewCombined();
	void doCascade();
	void actActivate();
	void xon_mdiArea_changed();
	void doFonts();
	void doMFonts();
	void ConnecttoVARA();
	void VARATimer();
	void AGWdisplayError(QAbstractSocket::SocketError socketError);
	void AGWreadyRead();
	void onAGWSocketStateChanged(QAbstractSocket::SocketState socketState);
	void VARAdisplayError(QAbstractSocket::SocketError socketError);
	void VARAreadyRead();
	void onVARASocketStateChanged(QAbstractSocket::SocketState socketState);
	void KISSdisplayError(QAbstractSocket::SocketError socketError);
	void KISSreadyRead();
	void onKISSSocketStateChanged(QAbstractSocket::SocketState socketState);
	int openSerialPort();
	void readSerialData();
	void handleError(QSerialPort::SerialPortError serialPortError);
	void doColours();
	void ColourPressed();
	void Colouraccept();
	void Colourreject();
	QColor setColor(QColor Colour);
	void VARADatadisplayError(QAbstractSocket::SocketError socketError);
	void VARADatareadyRead();
	void onVARADataSocketStateChanged(QAbstractSocket::SocketState socketState);
	void HAMLIBdisplayError(QAbstractSocket::SocketError socketError);
	void HAMLIBreadyRead();
	void onHAMLIBSocketStateChanged(QAbstractSocket::SocketState socketState);
	void ConnecttoHAMLIB();
	void HAMLIBSetPTT(int PTTState);
	void FLRigdisplayError(QAbstractSocket::SocketError socketError);
	void FLRigreadyRead();
	void onFLRigSocketStateChanged(QAbstractSocket::SocketState socketState);
	void ConnecttoFLRig();
	void FLRigSetPTT(int PTTState);
	void CATChanged(bool State);
	void PTTPortChanged(int Selected);
	void OpenPTTPort();
	void RadioPTT(bool PTTState);
	void tabSelected(int);
	void VARAHFChanged(bool state);
	void VARAFMChanged(bool State);
	void VARASATChanged(bool State);
	void SetVARAParams();

protected:
	bool eventFilter(QObject* obj, QEvent *event);

private:

	void ConnecttoAGW();

	void AGWTimer();

	Ui::QtTermTCPClass ui;
	
	QMenu *hostsubMenu;

	QAction *closeAct;
	QAction *closeAllAct;
	QAction *tileAct;
	QAction *cascadeAct;
	QAction *nextAct;
	QAction *previousAct;
	QAction *windowMenuSeparatorAct;
	QAction *newTermAct;
	QAction *newMonAct;
	QAction *newCombinedAct;
	QAction *AGWAction;
	QAction *VARAAction;
	QAction *KISSAction;
	QAction *quitAction;

	QList<myTcpSocket*>  _sockets;

	QWidget *centralWidget;
	void ConnecttoKISS();
	void KISSTimer();
};

extern "C"
{
	void EncodeSettingsLine(int n, char * String);
	void DecodeSettingsLine(int n, char * String);
	void WritetoOutputWindow(Ui_ListenSession * Sess, unsigned char * Buffer, int Len);
	void WritetoOutputWindowEx(Ui_ListenSession * Sess, unsigned char * Buffer, int len, QTextEdit * termWindow, int *OutputSaveLen, char * OutputSave, QColor Colour);
	void WritetoMonWindow(Ui_ListenSession * Sess, unsigned char * Buffer, int Len);
	void ProcessReceivedData(Ui_ListenSession * Sess, unsigned char * Buffer, int len);
	void SendTraceOptions(Ui_ListenSession * LUI);
	void setTraceOff(Ui_ListenSession * Sess);
	void SetPortMonLine(int i, char * Text, int visible, int enabled);
	void SaveSettings();
	void myBeep();
	void YAPPSendFile(Ui_ListenSession *  Sess, char * FN);
	int SocketSend(Ui_ListenSession * Sess, char * Buffer, int len);
	void SendTraceOptions(Ui_ListenSession * Sess);
	int SocketFlush(Ui_ListenSession * Sess);
	extern void mySleep(int ms);
	extern void setTraceOff(Ui_ListenSession * Sess);
}


char * strlop(char * buf, char delim);
extern "C" void setMenus(int State);
void Send_AGW_Ds_Frame(void * AGW);
