#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

#include "QtTermTCP.h"

#define _CRT_SECURE_NO_WARNINGS

#define TRUE 1
#define FALSE 0
#define UCHAR unsigned char
#define MAX_PATH 256
#define WCHAR char

#ifndef WIN32
#define strtok_s strtok_r
#endif


typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;

#define MAXHOSTS 16

#define TCHAR char

void QueueMsg(Ui_ListenSession * Sess, char * Msg, int Len);
int ProcessYAPPMessage(Ui_ListenSession * Sess, UCHAR * Msg, int Len);
void SetPortMonLine(int i, char * Text, int visible, int enabled);
void AGW_AX25_data_in(void  * Sess, UCHAR * data, int Len);
int checkUTF8(unsigned char * Msg, int Len, unsigned char * out);
void DoTermResize(Ui_ListenSession * Sess);
void DecodeTeleText(Ui_ListenSession * Sess, char * page);

int Bells = TRUE;
int StripLF = FALSE;
int LogOutput = FALSE;
int SendDisconnected = TRUE;
int ChatMode = TRUE;
int AutoTeletext = 1;

int MonPorts = 1;
int ListenOn = FALSE;

time_t LastWrite = 0xffffffff;
int AlertInterval = 300;
int AlertBeep = TRUE;
int AlertFreq = 600;
int AlertDuration = 250;
TCHAR AlertFileName[256] = { 0 };
int ConnectBeep = TRUE;
int UseKeywords = TRUE;

QString KeyWordsFile = "Keywords.sys";

char ** KeyWords = NULL;
int NumberofKeyWords = 0;


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

#define YAPPTX	32768					// Sending YAPP file

int MaxRXSize = 100000;
char BaseDir[256] = "";

unsigned char InputBuffer[1024];

char  YAPPPath[MAX_PATH] = "";	// Path for saving YAPP Files

int paclen = 128;

int InputLen;				// Data we have already = Offset of end of an incomplete packet;

unsigned char * MailBuffer;			// Yapp Message being received
int MailBufferSize;
int YAPPLen;				// Bytes sent/received of YAPP Message
long YAPPDate;				// Date for received file - if set enables YAPPC
char ARQFilename[200];		// Filename from YAPP Header

unsigned char SavedData[8192];		// Max receive is 4096 is should never get more that 8k
int SaveLen = 0;

void YAPPSendData(Ui_ListenSession * Sess);


char * strlop(char * buf, char delim)
{
	// Terminate buf at delim, and return rest of string

	char * ptr = strchr(buf, delim);

	if (ptr == NULL) return NULL;

	*(ptr)++ = 0;

	return ptr;
}

#ifdef WIN32

char * strcasestr(char *ch1, char *ch2)
{
	char	*chN1, *chN2;
	char	*chNdx;
	char	*chRet = NULL;

	chN1 = _strdup(ch1);
	chN2 = _strdup(ch2);

	if (chN1 && chN2)
	{
		chNdx = chN1;
		while (*chNdx)
		{
			*chNdx = (char)tolower(*chNdx);
			chNdx++;
		}
		chNdx = chN2;

		while (*chNdx)
		{
			*chNdx = (char)tolower(*chNdx);
			chNdx++;
		}

		chNdx = strstr(chN1, chN2);

		if (chNdx)
			chRet = ch1 + (chNdx - chN1);
	}

	free(chN1);
	free(chN2);
	return chRet;
}

#endif

void GetKeyWordFile()
{
	DWORD FileSize;
	char * ptr1, *ptr2;
	char * KeyWordFile;

	QFile file(KeyWordsFile);
	
	if (!file.open(QIODevice::ReadOnly))
	{
		if (UseKeywords)				// Don't need to alert if not being used
		{
			QMessageBox msgBox;
			msgBox.setText("Keyword File " + KeyWordsFile + " not found");
			msgBox.exec();
		}
		return;
	}

	FileSize = file.size();

	KeyWordFile = (char *)malloc(FileSize + 1);

	file.read(KeyWordFile, FileSize);

	file.close();

	KeyWordFile[FileSize] = 0;
 
	ptr1 = KeyWordFile;

	while (ptr1)
	{
		if (*ptr1 == '\n') ptr1++;

		ptr2 = strtok_s(NULL, "\r\n", &ptr1);
		if (ptr2)
		{
			if (*ptr2 != '#')
			{
				KeyWords = (char **)realloc(KeyWords, (++NumberofKeyWords + 1) * 4);
				KeyWords[NumberofKeyWords] = ptr2;
			}
		}
		else
			break;
	}
}


int CheckKeyWord(char * Word, char * Msg)
{
	char * ptr1 = Msg, *ptr2;
	int len = (int)strlen(Word);

	while (*ptr1)					// Stop at end
	{
		ptr2 = strcasestr(ptr1, Word);

		if (ptr2 == NULL)
			return FALSE;				// OK

		// Only bad if it ia not part of a longer word

		if ((ptr2 == Msg) || !(isalpha(*(ptr2 - 1))))	// No alpha before
			if (!(isalpha(*(ptr2 + len))))			// No alpha after
				return TRUE;					// Bad word

		// Keep searching

		ptr1 = ptr2 + len;
	}

	return FALSE;					// OK
}

int CheckKeyWords(UCHAR * Msg, int len)
{
	int i;

	if (UseKeywords == 0 || NumberofKeyWords == 0)
		return FALSE;

	// we need to null terminate Msg, so create a copy

	unsigned char * copy = (unsigned char *)malloc(len + 1);

	memcpy(copy, Msg, len);
	copy[len] = 0;

	for (i = 1; i <= NumberofKeyWords; i++)
	{
		if (CheckKeyWord(KeyWords[i], (char *)copy))
		{
			myBeep(&AlertWAV);
			free (copy);
			return TRUE;			// Alert
		}
	}

	free(copy);
	return FALSE;					// OK

}


void ProcessReceivedData(Ui_ListenSession * Sess, unsigned char * Buffer, int len)
{
	int MonLen = 0;
	unsigned char * ptr;
	unsigned char * Buffptr;
	unsigned char * FEptr = 0;

	if (Sess->InputMode == 'Y')			// Yapp
	{
		ProcessYAPPMessage(Sess, Buffer, len);
		return;
	}

	// See if Teletext data

			// We need to identify a viewdata frame. Seems to start

		//1e 0a 0a 0a 0a 0a 0a 0a  0a 0a 0a 0a 0a 0a 0a 0a   ........ ........
			//00000170  0a 0a 0a 0a 0a 0a 0a 0a  11 0c 1b

		// Page seems to start 1c

		// After page get 

		//1e 0a 0a 0a 0a 0a 0a 0a  0a 0a 0a 0a 0a 0a 0a 0a   ........ ........
		//	0000049D  0a 0a 0a 0a 0a 0a 0a 0a  11 14 1b 44 1b 5d 1b 43   ........ ...D.].C
		//	000004AD  53 65 6c 65                                        Sele
		//	000004B1  63 74 20 69 74 65 6d 20  6f 72 1b 47 2a 70 61 67   ct item or .G*pag
		//	000004C1  65 5f 20 3a 20 20 20 20  20 20 20 20 20 20 20 20   e_ :
		//	000004D1  20 0d 09 09 09 09 09 09  09 09 09 09 09 09 09 09    ....... ........
		//	000004E1  09 09 09 09 09 09 09 09  09 09 09 09 09 11

	Buffer[len] = 0;

	if (AutoTeletext && (Buffer[0] == 0x1e || Buffer[0] == 0x0c))
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

		if (strlen(&Sess->pageBuffer[0] + len) > 4090)
			Sess->pageBuffer[0] = 0;							// Protect buffer

		strcat(Sess->pageBuffer, (char *)Buffer);

		DecodeTeleText(Sess, (char *)Sess->pageBuffer);			// Re-decode same data until we get the end
		return;
	}



	//	mbstowcs(Buffer, BufferB, len);

	// Look for MON delimiters (FF/FE)


	Buffptr = Buffer;

	if (Sess->MonData)
	{
		// Already in MON State

		FEptr = (UCHAR *)memchr(Buffptr, 0xfe, len);

		if (!FEptr)
		{
			// no FE - so send all to monitor

			WritetoMonWindow(Sess, Buffer, len);
			return;
		}

		Sess->MonData = FALSE;

		MonLen = FEptr - Buffptr;		// Mon Data, Excluding the FE

		WritetoMonWindow(Sess, Buffptr, MonLen);

		Buffptr = ++FEptr;				// Char following FE

		if (++MonLen < len)
		{
			len -= MonLen;
			goto MonLoop;				// See if next in MON or Data
		}

		// Nothing Left

		return;
	}

MonLoop:

	ptr = (UCHAR *)memchr(Buffptr, 0xff, len);

	if (ptr)
	{
		unsigned char telcmd[] = "\xff\xfb\x03\xff\xfb\x01";

		// Try to trap connect to normal Telnet Port

		if (memcmp(ptr, telcmd, 6) == 0)
			return;

		// Buffer contains Mon Data

		if (ptr > Buffptr)
		{
			// Some Normal Data before the FF

			int NormLen = ptr - Buffptr;				// Before the FF

			if (NormLen == 1 && Buffptr[0] == 0)
			{
				// Keepalive
			}

			else
			{
				CheckKeyWords(Buffptr, NormLen);
				WritetoOutputWindow(Sess, Buffptr, NormLen);
			}

			len -= NormLen;
			Buffptr = ptr;
			goto MonLoop;
		}

		if (ptr[1] == 0xff)
		{
			// Port Definition String

			int NumberofPorts = atoi((char *)&ptr[2]);
			char *p, *Context;
			int i = 1;
			TCHAR msg[80];
			int portnum;
			char delim[] = "|";
			int m;

			// Save for changes of Window

			if (len < 2048)
				memcpy(Sess->PortMonString, ptr, len);

			
			// Remove old menu
			
			for (i = 0; i < 65; i++)
			{
				SetPortMonLine(i, (char *)"", 0, 0);
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

				if (Sess->portmask & (1ll << (m - 1)))
					SetPortMonLine(portnum, msg, 1, 1);
				else
					SetPortMonLine(portnum, msg, 1, 0);
			}
			return;
		}

		MonLen = len;				// in case no fe

		Sess->MonData = 1;

		FEptr = (UCHAR *)memchr(Buffptr, 0xfe, len);

		if (FEptr)
		{
			Sess->MonData = 0;

			MonLen = FEptr + 1 - Buffptr;				// MonLen includes FF and FE

			WritetoMonWindow(Sess, Buffptr + 1, MonLen - 2);

			len -= MonLen;
			Buffptr += MonLen;							// Char Following FE

			if (len <= 0)
			{
				return;
			}
			goto MonLoop;
		}
		else
		{
			// No FE, so rest of buffer is MON Data

			if (MonLen)
				WritetoMonWindow(Sess, Buffptr + 1, MonLen - 1);
			return;
		}
	}

	// No FF, so must be session data

	if (Sess->InputMode == 'Y')			// Yapp
	{
		ProcessYAPPMessage(Sess, Buffer, len);
		return;
	}


	if (len == 1 && Buffptr[0] == 0)
		return;							// Keepalive

	// Could be a YAPP Header

	if (len == 2 && Buffptr[0] == ENQ && Buffptr[1] == 1)		// YAPP Send_Init
	{
		char YAPPRR[2];

		// Turn off monitoring

		setTraceOff(Sess);
		
		Sess->InputMode = 'Y';

		YAPPRR[0] = ACK;
		YAPPRR[1] = 1;

		SocketFlush(Sess);			// To give Monitor Msg time to be sent
		mySleep(1000);
		QueueMsg(Sess, YAPPRR, 2);

		return;
	}
	// Check UTF8
	{
		CheckKeyWords(Buffptr, len);
		WritetoOutputWindow(Sess, Buffptr, len);
	}
	Sess->SlowTimer = 0;
	return;
}

extern myTcpSocket * VARASock;
extern myTcpSocket * VARADataSock;
extern "C" void SendtoAX25(void * conn, unsigned char * Msg, int Len);

int SendMsg(Ui_ListenSession * Sess, TCHAR * Buffer, int len)
{
	if (Sess->KISSSession)
	{
		// Send to ax.25 code

		SendtoAX25(Sess->KISSSession, (unsigned char *)Buffer, len);
		return len;
	}
	else if (Sess->AGWSession)
	{
		// Terminal is in AGWPE mode - send as AGW frame

		AGW_AX25_data_in(Sess->AGWSession, (unsigned char *)Buffer, len);
		return len;
	}
	else if (VARASock && VARASock->Sess == Sess)
	{
		VARADataSock->write(Buffer, len);
		return len;
	}

	return SocketSend(Sess, Buffer, len);
}



void QueueMsg(Ui_ListenSession * Sess, char * Msg, int len)
{
	int Sent = SendMsg(Sess, Msg, len);

	if (Sent != len)
		Sent = 0;
}

int InnerProcessYAPPMessage(Ui_ListenSession * Sess, UCHAR * Msg, int Len);

int ProcessYAPPMessage(Ui_ListenSession * Sess, UCHAR * Msg, int Len)
{
	// may have saved data

	memcpy(&SavedData[SaveLen], Msg, Len);

	SaveLen += Len;

	while (SaveLen && Sess->InputMode == 'Y')
	{
		int Used = InnerProcessYAPPMessage(Sess, SavedData, SaveLen);

		if (Used == 0)
			return 0;			// Waiting for more

		SaveLen -= Used;

		if (SaveLen)
			memmove(SavedData, &SavedData[Used], SaveLen);
	}
	return 0;
}

extern int VARAEnable;

int InnerProcessYAPPMessage(Ui_ListenSession * Sess, UCHAR * Msg, int Len)
{
	int pktLen = Msg[1];
	char Reply[2] = { ACK };
	size_t NameLen, SizeLen, OptLen;
	char * ptr;
	int FileSize;
	WCHAR MsgFile[MAX_PATH];
	FILE * hFile;
	char Mess[255];
	int len;
	UCHAR Buffer[2000];
	struct stat STAT;

	switch (Msg[0])
	{
	case ENQ: // YAPP Send_Init

		// Shouldn't occur in session. Reset state and process

		if (MailBuffer)
		{
			free(MailBuffer);
			MailBufferSize = 0;
			MailBuffer = 0;
		}

		Mess[0] = ACK;
		Mess[1] = 1;

		Sess->InputMode = 'Y';
		QueueMsg(Sess, Mess, 2);

		// Turn off monitoring

		mySleep(1000);				// To give YAPP Msg time to be sent

		setTraceOff(Sess);

		return Len;

	case SOH:

		// HD Send_Hdr     SOH  len  (Filename)  NUL  (File Size in ASCII)  NUL (Opt) 

		// YAPPC has date/time in dos format

		if (Len < Msg[1] + 1)
			return 0;				// Wait till we have it all

		NameLen = strlen((char *)&Msg[2]);
		strcpy(ARQFilename, (char *)&Msg[2]);

		ptr = (char *)&Msg[3 + NameLen];
		SizeLen = strlen(ptr);
		FileSize = atoi(ptr);

		OptLen = pktLen - (NameLen + SizeLen + 2);

		YAPPDate = 0;

		if (OptLen >= 8)		// We have a Date/Time for YAPPC
		{
			ptr = ptr + SizeLen + 1;
			YAPPDate = strtol(ptr, NULL, 16);
		}

		// Check Size

		if (FileSize > MaxRXSize && VARAEnable == 0)
		{
			Mess[0] = NAK;
			Mess[1] = sprintf(&Mess[2], "File %s size %d larger than limit %d\r", ARQFilename, FileSize, MaxRXSize);
			mySleep(1000);				// To give YAPP Msg time to be sent
			QueueMsg(Sess, Mess, Mess[1] + 2);

			len = sprintf((char *)Buffer, "YAPP File %s size %d larger than limit %d\r", ARQFilename, FileSize, MaxRXSize);
			WritetoOutputWindow(Sess, Buffer, len);

			Sess->InputMode = 0;
			SendTraceOptions(Sess);

			return Len;
		}

		// Check that Path is set

		if (YAPPPath[0] == 0)
		{
			Mess[0] = NAK;
			Mess[1] = sprintf(&Mess[2], "%s", "YAPP Receive directory not set");
			mySleep(1000);				// To give YAPP Msg time to be sent
			QueueMsg(Sess, Mess, Mess[1] + 2);
			len = sprintf((char *)Buffer, "YAPP File Receive Failed - YAPP Receive directory not set\r");
			WritetoOutputWindow(Sess, Buffer, len);

			Sess->InputMode = 0;
			SendTraceOptions(Sess);

			return Len;
		}

		// Make sure file does not exist

		sprintf(MsgFile, "%s/%s", YAPPPath, ARQFilename);

		if (stat(MsgFile, &STAT) == 0)
		{
			FileSize = STAT.st_size;

			Mess[0] = NAK;
			Mess[1] = sprintf(&Mess[2], "%s", "File Already Exists");
			mySleep(1000);				// To give YAPP Msg time to be sent
			QueueMsg(Sess, Mess, Mess[1] + 2);
			len = sprintf((char *)Buffer, "YAPP File Receive Failed - %s already exists\r", MsgFile);
			WritetoOutputWindow(Sess, Buffer, len);

			Sess->InputMode = 0;
			SendTraceOptions(Sess);

			return Len;
		}


		MailBufferSize = FileSize;
		MailBuffer = (UCHAR *)malloc(FileSize);
		YAPPLen = 0;

		if (YAPPDate)			// If present use YAPPC
			Reply[1] = ACK;			//Receive_TPK
		else
			Reply[1] = 2;			//Rcv_File

		QueueMsg(Sess, Reply, 2);

		len = sprintf((char *)Buffer, "YAPP Receving File %s size %d\r", ARQFilename, FileSize);
		WritetoOutputWindow(Sess, Buffer, len);

		return Len;

	case STX:

		// Data Packet

		// Check we have it all

		if (YAPPDate)			// If present use YAPPC so have checksum
		{
			if (pktLen > (Len - 3))		// -2 for header and checksum
				return 0;				// Wait for rest
		}
		else
		{
			if (pktLen > (Len - 2))		// -2 for header
				return 0;				// Wait for rest
		}

		// Save data and remove from buffer

		// if YAPPC check checksum

		if (YAPPDate)
		{
			UCHAR Sum = 0;
			int i;
			UCHAR * uptr = &Msg[2];

			i = pktLen;

			while (i--)
				Sum += *(uptr++);

			if (Sum != *uptr)
			{
				// Checksum Error

				Mess[0] = CAN;
				Mess[1] = sprintf(&Mess[2], "YAPPC Checksum Error");
				QueueMsg(Sess, Mess, Mess[1] + 2);

				len = sprintf((char *)Buffer, "YAPPC Checksum Error on file %s\r", MsgFile);
				WritetoOutputWindow(Sess, Buffer, len);

				Sess->InputMode = 0;
				SendTraceOptions(Sess);
				return Len;
			}
		}

		if ((YAPPLen) + pktLen > MailBufferSize)
		{
			// Too Big ??

			Mess[0] = CAN;
			Mess[1] = sprintf(&Mess[2], "YAPP Too much data received");
			QueueMsg(Sess, Mess, Mess[1] + 2);

			len = sprintf((char *)Buffer, "YAPP Too much data received on file %s\r", MsgFile);
			WritetoOutputWindow(Sess, Buffer, len);

			Sess->InputMode = 0;
			SendTraceOptions(Sess);
			return Len;
		}


		memcpy(&MailBuffer[YAPPLen], &Msg[2], pktLen);
		YAPPLen += pktLen;

		if (YAPPDate)
			++pktLen;				// Add Checksum

//		if (YAPPLen == MailBufferSize)
//			pktLen = pktLen;

		return pktLen + 2;

	case ETX:

		// End Data

		if (YAPPLen == MailBufferSize)
		{
			// All received

			int Written = 0;

			sprintf(MsgFile, "%s/%s", YAPPPath, ARQFilename);

			hFile = fopen(MsgFile, "wb");

			if (hFile)
			{
				Written = (int)fwrite(MailBuffer, 1, YAPPLen, hFile);
				fclose(hFile);

				if (YAPPDate)
				{
//					struct tm TM;
//					struct timeval times[2];
					/*
										The MS-DOS date. The date is a packed value with the following format.

										cant use DosDateTimeToFileTime on Linux

										Bits	Description
										0-4	Day of the month (1–31)
										5-8	Month (1 = January, 2 = February, and so on)
										9-15	Year offset from 1980 (add 1980 to get actual year)
										wFatTime
										The MS-DOS time. The time is a packed value with the following format.
										Bits	Description
										0-4	Second divided by 2
										5-10	Minute (0–59)
										11-15	Hour (0–23 on a 24-hour clock)
					*/
					/*
				
					memset(&TM, 0, sizeof(TM));

					TM.tm_sec = (YAPPDate & 0x1f) << 1;
					TM.tm_min = ((YAPPDate >> 5) & 0x3f);
					TM.tm_hour = ((YAPPDate >> 11) & 0x1f);

					TM.tm_mday = ((YAPPDate >> 16) & 0x1f);
					TM.tm_mon = ((YAPPDate >> 21) & 0xf) - 1;
					TM.tm_year = ((YAPPDate >> 25) & 0x7f) + 80;

					Debugprintf("%d %d %d %d %d %d", TM.tm_year, TM.tm_mon, TM.tm_mday, TM.tm_hour, TM.tm_min, TM.tm_sec);

					times[0].tv_sec = times[1].tv_sec = mktime(&TM);
					times[0].tv_usec = times[1].tv_usec = 0;
						*/
				}
			}


			free(MailBuffer);
			MailBufferSize = 0;
			MailBuffer = 0;

			if (Written != YAPPLen)
			{
				Mess[0] = CAN;
				Mess[1] = sprintf(&Mess[2], "Failed to save YAPP File");
				QueueMsg(Sess, Mess, Mess[1] + 2);

				len = sprintf((char *)Buffer, "Failed to save YAPP File %s\r", MsgFile);
				WritetoOutputWindow(Sess, Buffer, len);

				Sess->InputMode = 0;
				SendTraceOptions(Sess);
			}
		}

		Reply[1] = 3;		//Ack_EOF
		QueueMsg(Sess, Reply, 2);

		len = sprintf((char *)Buffer, "Reception of file %s complete\r", MsgFile);
		WritetoOutputWindow(Sess, Buffer, len);

		return Len;

	case EOT:

		// End Session

		Reply[1] = 4;		// Ack_EOT
		QueueMsg(Sess, Reply, 2);
		SocketFlush(Sess);
		Sess->InputMode = 0;

		SendTraceOptions(Sess);
		return Len;

	case CAN:

		// Abort

		Mess[0] = ACK;
		Mess[1] = 5;			// CAN Ack
		QueueMsg(Sess, Mess, 2);

		if (MailBuffer)
		{
			free(MailBuffer);
			MailBufferSize = 0;
			MailBuffer = 0;
		}

		// May have an error message

		len = Msg[1];

		if (len)
		{
			len = sprintf((char *)Buffer, "YAPP Transfer cancelled - %s\r", &Msg[2]);
		}
		else
			len = sprintf(Mess, "YAPP Transfer cancelled\r");

		Sess->InputMode = 0;
		SendTraceOptions(Sess);

		return Len;

	case ACK:

		switch (Msg[1])
		{
			char * ptr;

		case 1:					// Rcv_Rdy

			// HD Send_Hdr     SOH  len  (Filename)  NUL  (File Size in ASCII)  NUL (Opt)

			// Remote only needs filename so remove path

			ptr = ARQFilename;

			while (strchr(ptr, '/'))
				ptr = strchr(ptr, '/') + 1;

			len = (int)strlen(ptr) + 3;

			strcpy(&Mess[2], ptr);
			len += sprintf(&Mess[len], "%d", MailBufferSize);
			len++;					// include null
//			len += sprintf(&Mess[len], "%8X", YAPPDate);
//			len++;					// include null
			Mess[0] = SOH;
			Mess[1] = len - 2;

			QueueMsg(Sess, Mess, len);

			return Len;

		case 2:

			YAPPDate = 0;				// Switch to Normal (No Checksum) Mode

			// Drop through

		case 6:							// Send using YAPPC

			//	Start sending message

			YAPPSendData(Sess);
			return Len;

		case 3:

			// ACK EOF - Send EOT

			Mess[0] = EOT;
			Mess[1] = 1;
			QueueMsg(Sess, Mess, 2);

			return Len;

		case 4:

			// ACK EOT

			Sess->InputMode = 0;
			SendTraceOptions(Sess);

			len = sprintf((char *)Buffer, "File transfer complete\r");
			WritetoOutputWindow(Sess, Buffer, len);
	

			return Len;

		default:
			return Len;

		}

	case NAK:

		// Either Reject or Restart

		// RE Resume       NAK  len  R  NULL  (File size in ASCII)  NULL

		if (Len > 2 && Msg[2] == 'R' && Msg[3] == 0)
		{
			int posn = atoi((char *)&Msg[4]);

			YAPPLen += posn;
			MailBufferSize -= posn;

			YAPPSendData(Sess);
			return Len;

		}

		// May have an error message

		len = Msg[1];

		if (len)
		{
			char ws[256];

			Msg[len + 2] = 0;

			strcpy(ws, (char *)&Msg[2]);

			len = sprintf((char *)Buffer, "File rejected - %s\r", ws);
		}
		else
			len = sprintf((char *)Buffer, "File rejected\r");

		WritetoOutputWindow(Sess, Buffer, len);
	

		Sess->InputMode = 0;
		SendTraceOptions(Sess);

		return Len;

	}

	len = sprintf((char *)Buffer, "Unexpected message during YAPP Transfer. Transfer cancelled\r");
	WritetoOutputWindow(Sess, Buffer, len);
	
	Sess->InputMode = 0;
	SendTraceOptions(Sess);

	return Len;

}

void YAPPSendFile(Ui_ListenSession * Sess, WCHAR * FN)
{
	int FileSize = 0;
	char MsgFile[MAX_PATH];
	FILE * hFile;
	struct stat STAT;
	UCHAR Buffer[2000];
	int Len;

	strcpy(MsgFile, FN);

	if (MsgFile[0] == 0)
	{
		Len = sprintf((char *)Buffer, "Filename missing\r");
		WritetoOutputWindow(Sess, Buffer, Len);
	
		SendTraceOptions(Sess);

		return;
	}

	if (stat(MsgFile, &STAT) != -1)
	{
		FileSize = STAT.st_size;

		hFile = fopen(MsgFile, "rb");

		if (hFile)
		{
			char Mess[255];
//			time_t UnixTime = STAT.st_mtime;

//			FILETIME ft;
//			long long ll;
//			SYSTEMTIME st;
//			WORD FatDate;
//			WORD FatTime;
//			struct tm TM;

			strcpy(ARQFilename, MsgFile);

			if (MailBuffer)
			{
				free(MailBuffer);
				MailBufferSize = 0;
				MailBuffer = 0;
			}

			MailBuffer = (UCHAR *)malloc(FileSize);
			MailBufferSize = FileSize;
			YAPPLen = 0;
			fread(MailBuffer, 1, FileSize, hFile);

			// Get Date and Time for YAPPC Mode

/*					The MS-DOS date. The date is a packed value with the following format.

					cant use DosDateTimeToFileTime on Linux

					Bits	Description
					0-4	Day of the month (1–31)
					5-8	Month (1 = January, 2 = February, and so on)
					9-15	Year offset from 1980 (add 1980 to get actual year)
					wFatTime
					The MS-DOS time. The time is a packed value with the following format.
					Bits	Description
					0-4	Second divided by 2
					5-10	Minute (0–59)
					11-15	Hour (0–23 on a 24-hour clock)

					memset(&TM, 0, sizeof(TM));

					TM.tm_sec = (YAPPDate & 0x1f) << 1;
					TM.tm_min = ((YAPPDate >> 5) & 0x3f);
					TM.tm_hour =  ((YAPPDate >> 11) & 0x1f);

					TM.tm_mday =  ((YAPPDate >> 16) & 0x1f);
					TM.tm_mon =  ((YAPPDate >> 21) & 0xf) - 1;
					TM.tm_year = ((YAPPDate >> 25) & 0x7f) + 80;


// Note that LONGLONG is a 64-bit value

			ll = Int32x32To64(UnixTime, 10000000) + 116444736000000000;
			ft.dwLowDateTime = (DWORD)ll;
			ll >>= 32;
			ft.dwHighDateTime = (DWORD)ll;

			FileTimeToSystemTime(&ft, &st);
			FileTimeToDosDateTime(&ft, &FatDate, &FatTime);

			YAPPDate = (FatDate << 16) + FatTime;

			memset(&TM, 0, sizeof(TM));

			TM.tm_sec = (YAPPDate & 0x1f) << 1;
			TM.tm_min = ((YAPPDate >> 5) & 0x3f);
			TM.tm_hour = ((YAPPDate >> 11) & 0x1f);

			TM.tm_mday = ((YAPPDate >> 16) & 0x1f);
			TM.tm_mon = ((YAPPDate >> 21) & 0xf) - 1;
			TM.tm_year = ((YAPPDate >> 25) & 0x7f) + 80;
*/
			fclose(hFile);

			Mess[0] = ENQ;
			Mess[1] = 1;

			QueueMsg(Sess, Mess, 2);
			Sess->InputMode = 'Y';

			Len = sprintf((char *)Buffer, "Sending File %s ...\r", FN);
			WritetoOutputWindow(Sess, Buffer, Len);

			return;
		}
	}

	Len = sprintf((char *)Buffer, "File %s not found\r", FN);
	WritetoOutputWindow(Sess, Buffer, Len);

}

void YAPPSendData(Ui_ListenSession * Sess)
{
	char Mess[258];

	while (1)
	{
		int Left = MailBufferSize;

		if (Left == 0)
		{
			// Finished - send End Data

			Mess[0] = ETX;
			Mess[1] = 1;

			QueueMsg(Sess, Mess, 2);

			break;
		}

		if (Left > paclen - 3)		// two bytes header and possible checksum
			Left = paclen - 3;

		memcpy(&Mess[2], &MailBuffer[YAPPLen], Left);

		YAPPLen += Left;
		MailBufferSize -= Left;

		// if YAPPC add checksum

		if (YAPPDate)
		{
			UCHAR Sum = 0;
			int i;
			UCHAR * uptr = (UCHAR *)&Mess[2];

			i = Left;

			while (i--)
				Sum += *(uptr++);

			*(uptr) = Sum;

			Mess[0] = STX;
			Mess[1] = Left;

			QueueMsg(Sess, Mess, Left + 3);
		}
		else
		{
			Mess[0] = STX;
			Mess[1] = Left;

			QueueMsg(Sess, Mess, Left + 2);
		}
	}
}

