// AGW Interface

#include "QtTermTCP.h"
#include "TabDialog.h"
#include <time.h>
#include <QVBoxLayout>
#include <QLabel>

#ifndef WIN32
#define strtok_s strtok_r
#endif

#define UCHAR unsigned char
#define byte unsigned char

myTcpSocket * AGWSock;

extern QColor monRxText;
extern QColor monTxText;

extern QColor outputText;
extern QColor EchoText;
extern QColor WarningText;

extern QColor newTabText;

extern QTabWidget *tabWidget;
extern QWidget * mythis;

extern int AGWEnable;
extern int AGWMonEnable;
extern int AGWLocalTime;
extern int AGWMonNodes;
extern char AGWTermCall[12];
extern char AGWBeaconDest[12];
extern char AGWBeaconPath[80];
extern int AGWBeaconInterval;
extern char AGWBeaconPorts[80];
extern char AGWBeaconMsg[260];

extern char AGWHost[128];
extern int AGWPortNum;
extern int AGWPaclen;

extern Ui_ListenSession * KISSMonSess;

extern char listenCText[4096];
extern int ConnectBeep;

extern QList<Ui_ListenSession *> _sessions;

extern QLabel * Status1;
extern QLabel * Status2;
extern QLabel * Status3;
extern QLabel * Status4;

extern QMenu *connectMenu;
extern QAction *discAction;
extern QAction *YAPPSend;

extern QAction *actHost[17];

// Session Type Equates

#define Term 1
#define Mon 2
#define Listen 4

extern int TermMode;

#define Single 0
#define MDI 1
#define Tabbed 2

extern int singlemodeFormat;

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


struct AGWHeader
{
	int Port;
	unsigned char DataKind;
	unsigned char filler2;
	unsigned char PID;
	unsigned char filler3;
	char callfrom[10];
	char callto[10];
	int DataLength;
	int reserved;
};

#define AGWHDDRRLEN sizeof(struct AGWHeader)

typedef struct TAGWPort_t
{
	byte PID;
	int port;
	char corrcall[10];
	char mycall[10];
	int Active;
	QTcpSocket * socket;
	Ui_ListenSession * Sess;			// Terminal Session

} TAGWPort;


int AGWConnected = 0;
int AGWConnecting = 0;

char * AGWPortList = NULL;

#define max_sessions 8

TAGWPort AGWPort[max_sessions];

#define    LSB 29
#define    MSB 30
#define    MON_ON '1'
#define    MON_OFF '0'
#define    MODE_OUR 0
#define    MODE_OTHER 1
#define    MODE_RETRY 2

// Don't think we will support more than one, but leave in option

AGWUser *AGWUsers = NULL;				// List of currently connected clients

void AGW_add_socket(QTcpSocket * socket);
void AGW_Process_Input(AGWUser * AGW);
void Send_AGW_X_Frame(QTcpSocket* socket, char * CallFrom);
void Send_AGW_G_Frame(QTcpSocket* socket); 
void Send_AGW_m_Frame(QTcpSocket* socket);
void Send_AGW_R_Frame(QTcpSocket* socket);

Ui_ListenSession * FindFreeWindow();
Ui_ListenSession * newWindow(QObject * parent, int Type, const char * Label = nullptr);
void AGW_frame_header(UCHAR * Msg, char AGWPort, char DataKind, unsigned char PID, const char * CallFrom, const char * CallTo, int Len);


void Debugprintf(const char * format, ...)
{
	char Mess[10000];
	va_list arglist;

	va_start(arglist, format);
	vsprintf(Mess, format, arglist);
	qDebug() << Mess;

	return;
}

int ConvToAX25(char * callsign, unsigned char * ax25call)
{
	int i;

	memset(ax25call, 0x40, 6);		// in case short
	ax25call[6] = 0x60;				// default SSID

	for (i = 0; i < 7; i++)
	{
		if (callsign[i] == '-')
		{
			//
			//	process ssid and return
			//
			i = atoi(&callsign[i + 1]);

			if (i < 16)
			{
				ax25call[6] |= i << 1;
				return (1);
			}
			return (0);
		}

		if (callsign[i] == 0 || callsign[i] == 13 || callsign[i] == ' ' || callsign[i] == ',')
		{
			//
			//	End of call - no ssid
			//
			return (1);
		}

		ax25call[i] = callsign[i] << 1;
	}

	//
	//	Too many chars
	//

	return (0);
}


int ConvFromAX25(unsigned char * incall, char * outcall)
{
	int in, out = 0;
	unsigned char chr;

	memset(outcall, 0x20, 10);

	for (in = 0; in < 6; in++)
	{
		chr = incall[in];
		if (chr == 0x40)
			break;
		chr >>= 1;
		outcall[out++] = chr;
	}

	chr = incall[6];				// ssid

	if (chr == 0x42)
	{
		outcall[out++] = '-';
		outcall[out++] = 'T';
		return out;
	}

	if (chr == 0x44)
	{
		outcall[out++] = '-';
		outcall[out++] = 'R';
		return out;
	}

	chr >>= 1;
	chr &= 15;

	if (chr > 0)
	{
		outcall[out++] = '-';
		if (chr > 9)
		{
			chr -= 10;
			outcall[out++] = '1';
		}
		chr += 48;
		outcall[out++] = chr;
	}
	return (out);
}

void doAGWBeacon()
{
	if (AGWBeaconDest[0] && AGWBeaconPorts[0])
	{
		// Send as M or V depending on Digis

		UCHAR Msg[512];	
		char ports[80];

		char * ptr, * context;
		int DataLen;

		strcpy(ports, AGWBeaconPorts);			// strtok changes it


		// Replace newlines with CR

		while ((ptr = strchr(AGWBeaconMsg, 10)))
			*ptr = 13;

		ptr = strtok_s(ports, " ,", &context);

		if (AGWBeaconPath[0])
		{

		}
		else
		{
			while (ptr)
			{
				DataLen = (int)strlen(AGWBeaconMsg);

				AGW_frame_header(Msg, atoi(ptr) - 1, 'M', 240, AGWTermCall, AGWBeaconDest, DataLen);
				memcpy(&Msg[AGWHDDRRLEN], AGWBeaconMsg, DataLen);
				DataLen += AGWHDDRRLEN;
				AGWSock->write((char *)Msg, DataLen);

				ptr = strtok_s(NULL, " ,",  &context);
			}
		}
	}
}

TAGWPort * get_free_port()
{
	int i = 0;

	while (i < max_sessions)
	{
		if (AGWPort[i].Active == 0)
			return &AGWPort[i];

		i++;
	}
	return nullptr;
}

TAGWPort * findSession(int AGWChan, char * MyCall, char * OtherCall)
{
	int i = 0;

	TAGWPort * Sess = nullptr;

	while (i < max_sessions)
	{
		Sess = &AGWPort[i];

		if (Sess->Active && Sess->port == AGWChan && strcmp(Sess->corrcall, OtherCall) == 0 && strcmp(Sess->mycall, MyCall) == 0)
			return Sess;

		i++;
	}
	return nullptr;
}



void QtTermTCP::AGWdisplayError(QAbstractSocket::SocketError socketError)
{
	switch (socketError)
	{
	case QAbstractSocket::RemoteHostClosedError:
		break;

	case QAbstractSocket::HostNotFoundError:
		QMessageBox::information(this, tr("QtTermTCP"),
			tr("AGW host was not found. Please check the "
				"host name and port settings."));

		Status1->setText("AGW Connection Failed");

		break;

	case QAbstractSocket::ConnectionRefusedError:

		Status1->setText("AGW Connection Refused");
		break;

	default:

		Status1->setText("AGW Connection Failed");
	}

	AGWConnecting = 0;
	AGWConnected = 0;
}

void QtTermTCP::AGWreadyRead()
{
	int Read;
	unsigned char Buffer[4096];
	myTcpSocket* Socket = static_cast<myTcpSocket*>(QObject::sender());

	// read the data from the socket

	Read = Socket->read((char *)Buffer, 4095);

	int AGWHeaderLen = sizeof(struct AGWHeader);

	AGWUser * AGW = NULL;

	AGW = AGWUsers;

	if (AGW == NULL)
		return;

	memcpy(&AGW->data_in[AGW->data_in_len], Buffer, Read);

	AGW->data_in_len += Read;

	while (AGW->data_in_len >= AGWHeaderLen)		// Make sure have at least header
	{
		struct AGWHeader * Hddr = (struct AGWHeader *)AGW->data_in;

		int AgwLen = Hddr->DataLength + AGWHeaderLen;

		if (AGW->data_in_len >= AgwLen)
		{
			// Have frame as well

			AGW_Process_Input(AGW);

			AGW->data_in_len -= AgwLen;

			memmove(AGW->data_in, &AGW->data_in[AgwLen], AGW->data_in_len);
		}
		else
			return;					// Wait for the data
	}

}

void QtTermTCP::onAGWSocketStateChanged(QAbstractSocket::SocketState socketState)
{
	myTcpSocket* sender = static_cast<myTcpSocket*>(QObject::sender());
	Ui_ListenSession * Sess = (Ui_ListenSession *)sender->Sess;
	int i;

	if (socketState == QAbstractSocket::UnconnectedState)
	{
		// Close any connections

		Status1->setText("AGW Disconnected");
		actHost[16]->setVisible(0);

		int i = 0;

		TAGWPort * AGW = nullptr;

		for (i = 0; i < max_sessions; i++)
		{
			AGW = &AGWPort[i];

			if (AGW->Active && AGW->socket)
			{
				AGW->Active = 0;
				AGW->socket = nullptr;

				Sess = AGW->Sess;

				if (Sess)
				{
					// Send Disconnected

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
							char Label[32];
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

					setMenus(false);	
					
					// if Single Split or Monitor leave session allocated to AGW

					if (TermMode == Single && (Sess->SessionType & Mon))
					{
						return;
					}

					Sess->AGWSession = nullptr;
					AGW->Sess = nullptr;
				}
			}
		}

		// Free the monitor Window

		if (AGWUsers && AGWUsers->MonSess)
		{
			Sess = AGWUsers->MonSess;

			char Msg[] = "Disconnected\r";

			WritetoOutputWindowEx(Sess, (unsigned char *)Msg, (int)strlen(Msg),
				Sess->monWindow, &Sess->OutputSaveLen, Sess->OutputSave, WarningText);		// Red

			if (TermMode == MDI)
				Sess->setWindowTitle("Monitor Session Disconnected");

			else if (TermMode == Tabbed)
				tabWidget->setTabText(Sess->Tab, "Monitor");

			Sess->AGWSession = nullptr;
			AGWUsers->MonSess = nullptr;
		}

		if (TermMode == Single && (singlemodeFormat & Mon))
		{
			//Re-renable TCP connects

			for (int i = 0; i < MAXHOSTS; i++)
				actHost[i]->setVisible(1);
		}


		AGWConnected = 0;
	}
	else if (socketState == QAbstractSocket::ConnectedState)
	{
		AGWConnected = 1;
		AGWConnecting = 0;

		Status1->setText("AGW Connected");

		AGW_add_socket(sender);

		actHost[16]->setVisible(1);			// Enable AGW Connect Line

		// Send X frame to register Term Call

		if (AGWTermCall[0])
			Send_AGW_X_Frame(sender, AGWTermCall);

		Send_AGW_G_Frame(sender);					// Request Port List

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
				if ((S->SessionType == Mon) && S->clientSocket == NULL && S->AGWSession == NULL && S->KISSSession == NULL && (AGWUsers == NULL || (S != AGWUsers->MonSess)) && S != KISSMonSess)
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

				if (S->clientSocket == NULL && S->KISSSession == NULL && S->AGWSession == NULL && (AGWUsers == NULL || (S != AGWUsers->MonSess)) && S != KISSMonSess)
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
			AGWUsers->MonSess = Sess;
//			Sess->AGWSession = sender;			// Flag as in use

			if (TermMode == MDI)
				Sess->setWindowTitle("AGW Monitor Window");
			else if (TermMode == Tabbed)
				tabWidget->setTabText(Sess->Tab, "AGW Mon");
			else if (TermMode == Single)
				mythis->setWindowTitle("AGW Monitor Window");

			if (TermMode == Single && (singlemodeFormat & Mon))
			{
				// Can't be connected, so leave state alone, but disable TCP connects

				for (int i = 0; i < MAXHOSTS; i++)
					actHost[i]->setVisible(0);

			}
			else if (TermMode != Tabbed)				// Not ideal, but AGW mon window is unlikely to be active window
			{
				discAction->setEnabled(false);
				YAPPSend->setEnabled(false);
				connectMenu->setEnabled(false);
			}

			Sess->mlocaltime = AGWLocalTime;
			Sess->MonitorNODES = AGWMonNodes;

			Send_AGW_R_Frame(sender);			// Request Version
			Send_AGW_m_Frame(sender);			// Request Monitor Frames
		}
	}
}


void QtTermTCP::ConnecttoAGW()
{
	delete(AGWSock);

	AGWSock = new myTcpSocket();

	connect(AGWSock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(AGWdisplayError(QAbstractSocket::SocketError)));
	connect(AGWSock, SIGNAL(readyRead()), this, SLOT(AGWreadyRead()));
	connect(AGWSock, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onAGWSocketStateChanged(QAbstractSocket::SocketState)));

	AGWSock->connectToHost(AGWHost, AGWPortNum);

	Status1->setText("AGW Connecting");

	return;
}

int AGWBeaconTimer = 0;


void QtTermTCP::AGWTimer()
{
	// Runs every 10 Seconds

	if (AGWConnected == 0 && AGWConnecting == 0)
	{
		AGWConnecting = true;
		ConnecttoAGW();
	}

	if (AGWBeaconInterval)
	{
		AGWBeaconTimer++;

		if (AGWBeaconTimer >= AGWBeaconInterval * 6)
		{
			AGWBeaconTimer = 0;

			if (AGWConnected)
				doAGWBeacon();
		}
	}
}


void AGW_del_socket(void * socket)
{
	if (AGWUsers->socket == socket)
	{
		free(AGWUsers);
		AGWUsers = nullptr;
	}
}



void AGW_add_socket(QTcpSocket * socket)
{
	AGWUser * User = (struct AGWUser_t *)malloc(sizeof(struct AGWUser_t));			// One Client
	memset(User, 0, sizeof(struct AGWUser_t));

	User->socket = socket;

	AGWUsers = User;
};

void AGWWindowClosing(Ui_ListenSession *Sess)
{
	if (AGWUsers && AGWUsers->MonSess == Sess)
		AGWUsers->MonSess = NULL;
}



void AGW_frame_header(UCHAR * Msg, char AGWPort, char DataKind, unsigned char PID, const char * CallFrom, const char * CallTo, int Len)
{
	struct AGWHeader * Hddr = (struct AGWHeader *)Msg;
	memset(Hddr, 0, sizeof(struct AGWHeader));

	Hddr->Port = AGWPort;
	Hddr->DataLength = Len;
	Hddr->DataKind = DataKind;
	Hddr->PID = PID;
	strcpy(Hddr->callfrom, CallFrom);
	strcpy(Hddr->callto, CallTo);
};


/*

// AGW to APP frames

UCHAR * AGW_R_Frame()
{
	UCHAR * Msg = AGW_frame_header(0, 'R', 0, "", "", 8);

//	stringAdd(Msg, (UCHAR *)AGWVersion, 8);

	return Msg;
};
*/


void Send_AGW_X_Frame(QTcpSocket* socket, char * CallFrom)
{
	UCHAR Msg[512];

	AGW_frame_header(Msg, 0, 'X', 0, CallFrom, "", 0);
	socket->write((char *)Msg, AGWHDDRRLEN);
}

void Send_AGW_G_Frame(QTcpSocket* socket)
{
	UCHAR Msg[512];

	AGW_frame_header(Msg, 0, 'G', 0, "", "", 0);
	socket->write((char *)Msg, AGWHDDRRLEN);
}

void Send_AGW_m_Frame(QTcpSocket* socket)
{
	UCHAR Msg[512];

	// Request Monitoring. Add Extended form if connected to BPQ

	AGW_frame_header(Msg, 0, 'm', 0, "", "", 12);

	Msg[AGWHDDRRLEN] = AGWLocalTime;
	Msg[AGWHDDRRLEN + 1] = AGWMonNodes;
	Msg[AGWHDDRRLEN + 2] = AGWMonEnable;
	Msg[AGWHDDRRLEN + 3] = 0;

	memcpy(&Msg[AGWHDDRRLEN + 4], (void *)&AGWUsers->MonSess->portmask, 8);

	socket->write((char *)Msg, AGWHDDRRLEN + 12);
}

void Send_AGW_R_Frame(QTcpSocket* socket)
{
	UCHAR Msg[512];

	// Request Version

	AGW_frame_header(Msg, 0, 'R', 0, "", "", 0);
	socket->write((char *)Msg, AGWHDDRRLEN);
}


/*


UCHAR * AGW_Y_Frame(int port, char * CallFrom, char *CallTo, int frame_outstanding)
{
	UCHAR * Msg;

	Msg = AGW_frame_header(port, 'Y', 0, CallFrom, CallTo, 4);

	stringAdd(Msg, (UCHAR *)&frame_outstanding, 4);
	return Msg;
}



*/


void Send_AGW_C_Frame(Ui_ListenSession * Sess, int Port, char * CallFrom, char * CallTo, char * Data, int DataLen)
{
	UCHAR Msg[512];
	char Title[64];

	// We should allocate a AGW session record

	TAGWPort * AX25Sess;

	// FIrst check that we don't already have a session between these calla

	if (Port == -1)
		return;

	Debugprintf("Send_AGW_C_Frame");

	AX25Sess = findSession(Port, CallTo, CallFrom);

	if (AX25Sess)
	{
		// Seems this can be called twice
		
		if (Sess->AGWSession == nullptr)
			QMessageBox::information(mythis, "QtTermTCP", "You already have a conenction to that call");
		
		return;
	}

	AX25Sess = get_free_port();

	if (AX25Sess)
	{
		AX25Sess->Active = 1;
		AX25Sess->port = Port;
		AX25Sess->Sess = Sess;				// Crosslink AGW and Term Sessions
		AX25Sess->PID = 240;

		Sess->AGWSession = AX25Sess;

		strcpy(AX25Sess->mycall, CallTo);
		strcpy(AX25Sess->corrcall, CallFrom);

		AX25Sess->socket = AGWSock;

		if (DataLen)				// Digis so use 'v' frame
		{
			AGW_frame_header(Msg, Port, 'v', 240, CallFrom, CallTo, DataLen);
			memcpy(&Msg[AGWHDDRRLEN], (UCHAR *)Data, DataLen);
		}
		else
		{
			AGW_frame_header(Msg, Port, 'C', 240, CallFrom, CallTo, DataLen);
		}
		
		DataLen += AGWHDDRRLEN;
	
		AGWSock->write((char *)Msg, DataLen);

		sprintf(Title, "Connecting to %s", CallTo);

		if (TermMode == MDI)
			Sess->setWindowTitle(Title);
		else if (TermMode == Tabbed)
			tabWidget->setTabText(Sess->Tab, "Connecting");
		else if (TermMode == Single)
			mythis->setWindowTitle(Title);

	}
}


void Send_AGW_Ds_Frame(void * Sess)
{
	TAGWPort * AGW = (TAGWPort *)Sess;
	UCHAR Msg[512];

	AGW_frame_header(Msg, AGW->port, 'd', 240, AGW->corrcall, AGW->mycall, 0);
	if (AGW->socket)
		AGW->socket->write((char *)Msg, AGWHDDRRLEN);
}



void Send_AGW_D_Frame(TAGWPort * AGW, UCHAR * Data,  int DataLen)
{
	UCHAR Msg[512];

	AGW_frame_header(Msg, AGW->port, 'D', AGW->PID, AGW->corrcall, AGW->mycall, DataLen);

	memcpy(&Msg[AGWHDDRRLEN], Data, DataLen);

	DataLen += AGWHDDRRLEN;

	AGW->socket->write((char *)Msg, DataLen);
}



/*

UCHAR * AGW_T_Frame(int port, char * CallFrom, char * CallTo, char * Data)
{
	UCHAR * Msg;
	int DataLen;

	DataLen = strlen(Data);

	Msg = AGW_frame_header(port, 'T', 0, CallFrom, CallTo, DataLen);

	stringAdd(Msg, (byte *)Data, DataLen);

	return Msg;
}

// Raw Monitor 
UCHAR * AGW_K_Frame(int port, int PID, char * CallFrom, char * CallTo, UCHAR * Data, int DataLen)
{
	UCHAR Msg[512];

	DataLen = Data->Length;

	AGW_frame_header(Msg, port, 'K', PID, CallFrom, CallTo, DataLen);

//	stringAdd(Msg, Data->Data, Data->Length);

//	freeString(Data);

	return Msg;
}

// APP to AGW frames

void on_AGW_P_frame(AGWUser * AGW)
{
	
}

*/

void on_AGW_G_frame(unsigned char * Data)
{
	if (AGWPortList)
		free(AGWPortList);
	
	AGWPortList = (char *)strdup((char *)Data);
};

/*

void on_AGW_Ms_frame(AGWUser * AGW)
{
	AGW->Monitor ^= 1;				// Flip State
}


void on_AGW_R_frame(AGWUser * AGW)
{
//	AGW_send_to_app(AGW->socket, AGW_R_Frame());
}



void on_AGW_Y_frame(void * socket, int snd_ch, char * CallFrom, char * CallTo)
{
	int  frames;
	TAGWPort * AX25Sess;

//	AX25Sess = get_user_port_by_calls(snd_ch, CallFrom, CallTo);

//	if (AX25Sess)
//	{
		//frames = AX25port[snd_ch][user_port].in_data_buf.Count;
//		frames = AX25Sess->in_data_buf.Count + AX25Sess->I_frame_buf.Count;
//		;
//		AGW_send_to_app(socket, AGW_Y_Frame(snd_ch, CallFrom, CallTo, frames));
//	}
}

// UI Transmit

void on_AGW_M_frame(int port, byte PID, char * CallFrom, char *CallTo, byte *  Msg, int MsgLen)
{
	byte path[80];
	char Calls[80];
	UCHAR * Data = newString();

	stringAdd(Data, Msg, MsgLen);

	sprintf(Calls, "%s,%s", CallTo, CallFrom);

	get_addr(Calls, 0, 0, path);

	Add(&all_frame_buf[port],
		make_frame(Data, path, PID, 0, 0, U_FRM, U_UI, FALSE, SET_F, SET_C));

}

*/
void on_AGW_C_frame(AGWUser * AGW, struct AGWHeader * Frame, byte * Msg)
{
	int snd_ch = Frame->Port;
	char * CallFrom = Frame->callfrom;
	char * CallTo = Frame->callto;
	char Title[64];
	int i = 0;

	TAGWPort * AX25Sess;

	// Connection received - may be incoming or response to our connect

		// See if incoming

	if (strstr((char *)Msg, (const char *)"CONNECTED To"))
	{
		// incoming call

		AX25Sess = get_free_port();

		if (AX25Sess)
		{
			// Look for/create Terminal Window for connection

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
					if ((S->SessionType & Listen) && S->clientSocket == NULL)
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
			else if (TermMode == Tabbed)
			{
				// Single Tabbed - look for free session


				for (i = 0; i < _sessions.size(); ++i)
				{
					S = _sessions.at(i);

					if (S->clientSocket == NULL && S->AGWSession == NULL && S->KISSSession == NULL)
					{
						Sess = S;
						break;
					}
				}
			}
			else
			{
				// Single - Only one available
				S = _sessions.at(0);

				if (S->clientSocket == NULL && S->AGWSession == NULL && S->KISSSession == NULL)
					Sess = S;
			}

			if (Sess == NULL)
			{
				// Clear connection by sendinf d fraame

				UCHAR Msg[512];

				AGW_frame_header(Msg, Frame->Port, 'd', 240, Frame->callto, Frame->callfrom, 0);
				if (AGW->socket)
					AGW->socket->write((char *)Msg, AGWHDDRRLEN);

				return;
			}
		
			if (Sess)
			{
				sprintf(Title, "Connected to %s", CallFrom);

				QApplication::alert(mythis, 0);

				if (TermMode == MDI)
				{
					Sess->setWindowTitle(Title);
				}
				else if (TermMode == Tabbed)
				{
					tabWidget->setTabText(i, CallFrom);
					tabWidget->tabBar()->setTabTextColor(i, newTabText);
				}

				else if (TermMode == Single)
					mythis->setWindowTitle(Title);

				AX25Sess->Active = 1;
				AX25Sess->port = snd_ch;
				AX25Sess->Sess = Sess;				// Crosslink AGW and Term Sessions
				AX25Sess->PID = 240;;

				Sess->AGWSession = AX25Sess;

				strcpy(AX25Sess->mycall, CallFrom);
				strcpy(AX25Sess->corrcall, CallTo);

				AX25Sess->socket = AGW->socket;

				setMenus(true);

				if (ConnectBeep)
					myBeep(&ConnectWAV);

				// Send CText if defined

				if (listenCText[0])
					Send_AGW_D_Frame(AX25Sess, (unsigned char *)listenCText, (int)strlen(listenCText));

				// Send Message to Terminal

				char Msg[80];

				sprintf(Msg, "Incoming AGW Connection from %s\r\r", CallFrom);

				WritetoOutputWindow(Sess, (unsigned char *)Msg, (int)strlen(Msg));
				return;
			}
		}
	}

	// Reply to out connect request - look for our connection

	AX25Sess = findSession(snd_ch, CallFrom, CallTo);

	if (AX25Sess)
	{
		Ui_ListenSession * Sess = AX25Sess->Sess;

		sprintf(Title, "Connected to %s", CallFrom);

		if (TermMode == MDI)
			Sess->setWindowTitle(Title);
		else if (TermMode == Tabbed)
			tabWidget->setTabText(Sess->Tab, CallFrom);
		else if (TermMode == Single)
			mythis->setWindowTitle(Title);

		setMenus(true);

		int Len = sprintf(Title, "*** Connected to %s\r", CallFrom);

		ProcessReceivedData(Sess, (unsigned char *)Title, Len);
	}

}



void on_AGW_D_frame(int snd_ch, char * CallFrom, char * CallTo, byte * Msg, int Len)
{
	TAGWPort * AX25Sess;

	AX25Sess = findSession(snd_ch, CallFrom, CallTo);

	if (AX25Sess)
	{
		ProcessReceivedData(AX25Sess->Sess, Msg, Len);
	}
}

extern "C" void WritetoMonWindow(Ui_ListenSession * Sess, unsigned char * Msg, int len);

void on_AGW_Mon_frame(byte * Msg, int Len, char Type)
{
	if (AGWUsers && AGWUsers->MonSess && AGWUsers->MonSess->monWindow)
	{
		unsigned char copy[512];

		copy[0] = 0x1b;
		if	(Type == 'T')
			copy[1] = 91;
		else
			copy[1] = 17;

		memcpy(&copy[2], Msg, Len);
		WritetoMonWindow(AGWUsers->MonSess, copy, Len + 2);
	}
}



void on_AGW_Ds_frame(int AGWChan, char * CallFrom, char * CallTo, struct AGWHeader * Frame)
{
	TAGWPort * AX25Sess;
	char * Msg = (char *)Frame;
	Msg = &Msg[36];

	char Reply[128] = "";

	memcpy(Reply, Msg, Frame->DataLength);

	// Disconnect Sessiom

	AX25Sess = findSession(AGWChan, CallFrom, CallTo);

	if (AX25Sess)
	{
		Ui_ListenSession * Sess = AX25Sess->Sess;

		WritetoOutputWindowEx(Sess, (unsigned char *)Reply, (int)strlen(Reply),
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
				char Label[32];
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

		Sess->AGWSession = nullptr;
		AX25Sess->Sess = nullptr;
		AX25Sess->Active = 0;

		setMenus(false);
	}
	else
	{
		// Session not found
	}
}


void AGW_AX25_data_in(void  * Sess, UCHAR * data, int Len)
{
	TAGWPort * AX25Sess = (TAGWPort *)Sess;
	int len = 250, sendlen;
	UCHAR pkt[512];

	while (Len)
	{
		if (Len > len)
			sendlen = len;
		else
			sendlen = Len;

		memcpy(pkt, data, sendlen);

		Len -= sendlen;

		memmove(data, &data[sendlen], Len);

		Send_AGW_D_Frame(AX25Sess, pkt, sendlen);
	}

}


/*

void AGW_frame_monitor(byte snd_ch, byte * path, UCHAR * data, byte pid, byte nr, byte ns, byte f_type, byte f_id, byte  rpt, byte pf, byte cr, byte RX)
{
	char mon_frm[512];
	char AGW_path[256];
	UCHAR * AGW_data;

	const char * frm;
	byte * datap = data->Data;
	byte _data[512];
	byte * p_data = _data;
	int _datalen;

	char  agw_port;
	char  CallFrom[10], CallTo[10], Digi[80];

	int i;
	const char * ctrl;
	int len;

	AGWUser * AGW;

	UNUSED(rpt);

	len = data->Length;

	//	if (pid == 0xCF)
	//		data = parse_NETROM(data, f_id);
		// IP parsing
	//	else if (pid == 0xCC)
	//		data = parse_IP(data);
		// ARP parsing
	//	else if (pid == 0xCD)
	//		data = parse_ARP(data);
		//

	if (len > 0)
	{
		for (i = 0; i < len; i++)
		{
			if (datap[i] > 31 || datap[i] == 13 || datap[i] == 9)
				*(p_data++) = datap[i];
		}
	}

	_datalen = p_data - _data;

	if (_datalen)
	{
		byte * ptr = _data;
		i = 0;

		// remove successive cr or cr on end		while (i < _datalen)

		while (i < _datalen)
		{
			if ((_data[i] == 13) && (_data[i + 1] = 13))
				i++;
			else
				*(ptr++) = _data[i++];
		}

		if (*(ptr - 1) == 13)
			ptr--;

		*ptr = 0;

		_datalen = ptr - _data;
	}

	agw_port = snd_ch;

	get_monitor_path(path, CallTo, CallFrom, Digi);

	ctrl = "";
	frm = "";

	switch (f_id)
	{
	case I_I:

		frm = "I";
		if (cr == SET_C && pf == SET_P)
			ctrl = " P";

		break;

	case S_RR:

		frm = "RR";
		if (pf == SET_P)
			ctrl = " P/F";

		break;

	case S_RNR:

		frm = "RNR";
		if (pf == SET_P)
			ctrl = " P/F";

		break;

	case S_REJ:

		frm = "REJ";
		if (pf == SET_P)
			ctrl = " P/F";

		break;

	case U_SABM:

		frm = "SABM";
		if (cr == SET_C && pf == SET_P)
			ctrl = " P";

		break;

	case U_DISC:

		frm = "DISC";
		if (cr == SET_C && pf == SET_P)
			ctrl = " P";
		break;

	case U_DM:

		frm = "DM";
		if ((cr == SET_R) && (pf == SET_P))
			ctrl = " F ";
		break;

	case U_UA:

		frm = "UA";
		if ((cr == SET_R) && (pf == SET_P))
			ctrl = " F ";

		break;

	case U_FRMR:

		frm = "FRMR";
		if ((cr == SET_R) && (pf == SET_P))
			ctrl = " F ";
		break;

	case U_UI:

		frm = "UI";
		if ((pf == SET_P))
			ctrl = " P/F";
	}

	if (Digi[0])
		sprintf(AGW_path, " %d:Fm %s To %s Via %s <%s", snd_ch + 1, CallFrom, CallTo, Digi, frm);
	else
		sprintf(AGW_path, " %d:Fm %s To %s <%s", snd_ch + 1, CallFrom, CallTo, frm);


	switch (f_type)
	{
	case I_FRM:

		//mon_frm = AGW_path + ctrl + ' R' + inttostr(nr) + ' S' + inttostr(ns) + ' pid=' + dec2hex(pid) + ' Len=' + inttostr(len) + ' >' + time_now + #13 + _data + #13#13;
		sprintf(mon_frm, "%s%s R%d S%d pid=%X Len=%d >[%s]\r%s\r", AGW_path, ctrl, nr, ns, pid, len, ShortDateTime(), _data);

		break;

	case U_FRM:

		if (f_id == U_UI)
		{
			sprintf(mon_frm, "%s pid=%X Len=%d >[%s]\r%s\r", AGW_path, pid, len, ShortDateTime(), _data); // "= AGW_path + ctrl + '>' + time_now + #13;
		}
		else if (f_id == U_FRMR)
		{
			//			_data = copy(_data + #0#0#0, 1, 3);
			//			mon_frm = AGW_path + ctrl + '>' + time_now + #13 + inttohex((byte(data[1]) shl 16) or (byte(data[2]) shl 8) or byte(data[3]), 6) + #13#13;
		}
		else
			sprintf(mon_frm, "%s%s>[%s]\r", AGW_path, ctrl, ShortDateTime()); // "= AGW_path + ctrl + '>' + time_now + #13;

		break;

	case S_FRM:

		//		mon_frm = AGW_path + ctrl + ' R' + inttostr(nr) + ' >' + time_now + #13;
		sprintf(mon_frm, "%s%s R%d>[%s]\r", AGW_path, ctrl, nr, ShortDateTime()); // "= AGW_path + ctrl + '>' + time_now + #13;

		break;

	}

	//	stringAdd(mon_frm, "", 1); //  Add 0 at the end of each frame

		// I think we send to all AGW sockets

	for (i = 0; i < AGWConCount; i++)
	{
		AGW = AGWUsers[i];

		if (AGW->Monitor)
		{
			if (RX)
			{
				

			case f_id of
			  I_I   : AGW_data = AGW_I_frame(agw_port,CallFrom,CallTo,mon_frm);
			  S_RR  : AGW_data = AGW_S_frame(agw_port,CallFrom,CallTo,mon_frm);
			  S_RNR : AGW_data = AGW_S_frame(agw_port,CallFrom,CallTo,mon_frm);
			  S_REJ : AGW_data = AGW_S_frame(agw_port,CallFrom,CallTo,mon_frm);
			  U_SABM: AGW_data = AGW_S_frame(agw_port,CallFrom,CallTo,mon_frm);
			  U_DISC: AGW_data = AGW_S_frame(agw_port,CallFrom,CallTo,mon_frm);
			  U_DM  : AGW_data = AGW_S_frame(agw_port,CallFrom,CallTo,mon_frm);
			  U_UA  : AGW_data = AGW_S_frame(agw_port,CallFrom,CallTo,mon_frm);
			  U_FRMR: AGW_data = AGW_S_frame(agw_port,CallFrom,CallTo,mon_frm);
			  U_UI  : AGW_data = AGW_U_frame(agw_port,CallFrom,CallTo,mon_frm);

							AGW_send_to_app(AGW->socket, AGW_data);

			  
			}
			else
			{
				AGW_data = AGW_T_Frame(agw_port, CallFrom, CallTo, mon_frm);
				AGW_send_to_app(AGW->socket, AGW_data);
			}
		}
	}
}

*/




void AGW_Process_Input(AGWUser * AGW)
{
	struct AGWHeader * Frame = (struct AGWHeader *)AGW->data_in;
	byte * Data = &AGW->data_in[36];

	switch (Frame->DataKind)
	{
	case 'P':

//		on_AGW_P_frame(AGW);
//		return;

	case 'X':

		// Registration response - no need to process

		return;

	case 'G':

		on_AGW_G_frame(Data);
		return;
/*

	case 'x':

		on_AGW_Xs_frame(Frame->callfrom);
		return;



	case 'm':

		on_AGW_Ms_frame(AGW);
		return;

	case 'R':

		on_AGW_R_frame(AGW);
		return;


		//	'g': on_AGW_Gs_frame(AGW,Frame->Port);
		//	'H': on_AGW_H_frame(AGW,Frame->Port);
		//	'y': on_AGW_Ys_frame(AGW,Frame->Port);

	case 'Y':
		on_AGW_Y_frame(AGW->socket, Frame->Port, Frame->callfrom, Frame->callto);
		break;

	case 'M':

		on_AGW_M_frame(Frame->Port, Frame->PID, Frame->callfrom, Frame->callto, Data, Frame->DataLength);
		break;
*/

	case 'C':
//	case 'v':				// Call with digis

		on_AGW_C_frame(AGW, Frame, Data);
		return;
	
	
	case 'D':

		on_AGW_D_frame(Frame->Port, Frame->callfrom, Frame->callto, Data, Frame->DataLength);
		return;

	case 'd':

		on_AGW_Ds_frame(Frame->Port, Frame->callfrom, Frame->callto, Frame);
		return;

	case 'U':
	case 'S':
	case 'I':
	case 'T':

			// Monitor Data

			if (AGWMonEnable)
				on_AGW_Mon_frame(Data, Frame->DataLength, Frame->DataKind);
		
			return;

/*

		//	'V': on_AGW_V_frame(AGW,Frame->Port,PID,CallFrom,CallTo,Data);
		//	'c': on_AGW_Cs_frame(sAGWocket,Frame->Port,PID,CallFrom,CallTo);


	case 'K':

		on_AGW_K_frame(Frame);
		return;

	case 'k':
		on_AGW_Ks_frame(AGW);
		return;
		*/
	default:
		Debugprintf("AGW %c", Frame->DataKind);
	}
}
