// ax.25 code for QtTERMTCP KISS Mode

// based on SoundModem/QtSoundModem ax.25 code

// The underlying code allows for up to four KISS ports, but at the moment
// the config and user interface only supports 1

// The code supports KISS over a Serial port or TCP connection

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#define UNUSED(x) (void)(x)

#define single float
#define boolean int
#define Byte unsigned char		//                  0 to 255
#define Word unsigned short	//                        0 to 65,535
#define SmallInt short 		//                  -32,768 to 32,767
#define LongWord unsigned int	//                        0 to 4,294,967,295
//  Int6 : Cardinal; //                        0 to 4,294,967,295
#define LongInt int			//           -2,147,483,648 to 2,147,483,647
#define Integer int  //           -2,147,483,648 to 2,147,483,647
//#define Int64 long long		 // -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807

//#define Byte unsigned char		//                  0 to 255
#define word unsigned short	//                        0 to 65,535
#define smallint short 		//                  -32,768 to 32,767
#define longword unsigned int	//                        0 to 4,294,967,295
 //  Int6 : Cardinal; //                        0 to 4,294,967,295
#define longint int			//           -2,147,483,648 to 2,147,483,647
#define integer int  //           -2,147,483,648 to 2,147,483,647

typedef unsigned long ULONG;

#define UCHAR unsigned char
#define UINT unsigned int
#define BOOL int
#define TRUE 1
#define FALSE 0

#define FEND 0xc0
#define FESC 0xDB
#define TFEND 0xDC
#define TFESC 0xDD

#define port_num 32		// ?? Max AGW sessions
#define PKT_ERR 15		// Minimum packet size, bytes
#define I_MAX 7			// Maximum number of packets


// Status flags

#define STAT_NO_LINK  0
#define STAT_LINK 1
#define STAT_CHK_LINK 2
#define STAT_WAIT_ANS 3
#define STAT_TRY_LINK 4
#define STAT_TRY_UNLINK 5


	// Сmd,Resp,Poll,Final,Digipeater flags
#define 	SET_P 1
#define 	SET_F 0
#define 	SET_C 1
#define 	SET_R 0
#define 	SET_NO_RPT 0
#define 	SET_RPT 1
	// Frame ID flags
#define 	I_FRM 0
#define 	S_FRM 1
#define 	U_FRM 2
#define 	I_I 0
#define 	S_RR 1
#define 	S_RNR 5
#define 	S_REJ 9
#define		S_SREJ 0x0D
#define 	U_SABM 47
#define 	U_DISC 67
#define 	U_DM 15
#define 	U_UA 99
#define 	U_FRMR 135
#define 	U_UI 3
	// PID flags
#define 	PID_X25 0x01       // 00000001-CCIT X25 PLP
#define 	PID_SEGMENT 0x08   // 00001000-Segmentation fragment
#define 	PID_TEXNET 0xC3    // 11000011-TEXNET Datagram Protocol
#define 	PID_LQ 0xC4        // 11001000-Link Quality Protocol
#define 	PID_APPLETALK 0xCA // 11001010-Appletalk
#define 	PID_APPLEARP 0xCB  // 11001011-Appletalk ARP
#define 	PID_IP 0xCC        // 11001100-ARPA Internet Protocol
#define 	PID_ARP 0xCD       // 11001101-ARPA Address Resolution Protocol
#define 	PID_NET_ROM 0xCF   // 11001111-NET/ROM

#define FX25_LOAD 1

#define    MODE_OUR 0
#define    MODE_OTHER 1
#define    MODE_RETRY 2

#define TIMER_FREE 0
#define TIMER_BUSY 1
#define TIMER_OFF 2
#define TIMER_EVENT_ON 3
#define TIMER_EVENT_OFF 4


typedef struct string_T
{
	unsigned char * Data;
	int Length;
	int AllocatedLength;				// A reasonable sized block is allocated at the start to speed up adding chars

}string;

typedef struct TStringList_T
{
	int Count;
	string ** Items;

} TStringList;


typedef struct AGWUser_t
{
	void *socket;
	string * data_in;
	TStringList AGW_frame_buf;
	boolean	Monitor;
	boolean	Monitor_raw;
	boolean reportFreqAndModem;			// Can report modem and frequency to host

} AGWUser;

typedef struct  TAX25Info_t
{
	longint	stat_s_pkt;
	longint stat_s_byte;
	longint stat_r_pkt;
	longint stat_r_byte;
	longint stat_r_fc;
	longint stat_fec_count;
	time_t stat_begin_ses;
	time_t stat_end_ses;
	longint stat_l_r_byte;
	longint stat_l_s_byte;

} TAX25Info;

typedef struct TAX25Port_t
{
	Byte hi_vs;
	Byte vs;
	Byte vr;
	Byte PID;
	TStringList in_data_buf;
	TStringList frm_collector;
	string frm_win[8];
	string out_data_buf;
	word t1;
	word t2;
	word t3;
	Byte i_lo;
	Byte i_hi;
	word n1;
	word n2;
	word IPOLL_cnt;
	TStringList frame_buf; //буфер кадров на передачу
	TStringList I_frame_buf;
	Byte status;
	word clk_frack;
	char corrcall[10];
	char mycall[10];
	UCHAR digi[56];
	UCHAR Path[80];				// Path in ax25 format - added to save building it each time
	UCHAR ReversePath[80];
	int snd_ch;					// Simplifies parameter passing
	int port;
	int pathLen;
	void * socket;
	void * Sess;
	char kind[16];
	TAX25Info info;
} TAX25Port;

typedef struct TKISSMode_t
{
	string * data_in;
	void * Socket;				// Used as a key

	// Not sure what rest are used for. Seems to be one per channel

	TStringList buffer[4];			// Outgoing Frames

} TKISSMode;

// Dephi emulation functions

string * Strings(TStringList * Q, int Index);
void Clear(TStringList * Q);
int Count(TStringList * List);

string * newString();
string * copy(string * Source, int StartChar, int Count);
TStringList * newTStringList();

void freeString(string * Msg);
void initString(string * S);
void initTStringList(TStringList* T);

// Two delete() This is confusing!!
// Not really - one acts on String, other TStringList

void Delete(TStringList * Q, int Index);
void mydelete(string * Source, int StartChar, int Count);
void move(UCHAR * SourcePointer, UCHAR * DestinationPointer, int CopyCount);
void fmove(float * SourcePointer, float * DestinationPointer, int CopyCount);
void setlength(string * Msg, int Count);		// Set string length
string * stringAdd(string * Msg, UCHAR * Chars, int Count);		// Extend string 
void Assign(TStringList * to, TStringList * from);	// Duplicate from to to
string * duplicateString(string * in);
int  my_indexof(TStringList * l, string * s);
boolean compareStrings(string * a, string * b);
int Add(TStringList * Q, string * Entry);
void Debugprintf(const char * format, ...);
void ax25_info_init(TAX25Port * AX25Sess);
void  clr_frm_win(TAX25Port * AX25Sess);
void decode_frame(Byte * frame, int len, Byte * path, string * data,
	Byte * pid, Byte * nr, Byte * ns, Byte * f_type, Byte * f_id,
	Byte *  rpt, Byte * pf, Byte * cr);
#ifdef __cplusplus
extern "C" void KISSSendtoServer(myTcpSocket* Socket, char * Data, int Length);
extern "C" void monitor_frame(int snd_ch, string * frame, char * code, int  tx, int excluded);
extern "C" void WriteDebugLog(char * Mess);
extern "C" void SendtoTerm(Ui_ListenSession * Sess, char * Msg, int Len);
extern "C" void ClearSessLabel(Ui_ListenSession * Sess);
extern "C" void rst_timer(TAX25Port * AX25Sess);
extern "C" void Send_UI(int port, Byte PID, char * CallFrom, char *CallTo, Byte *  Msg, int MsgLen);
#else
void monitor_frame(int snd_ch, string * frame, char * code, int  tx, int excluded);
void SendtoTerm(void * Sess, char * Msg, int Len);
void ClearSessLabel(void * Sess);
void WriteDebugLog(char * Mess);
void AX25_disc(TAX25Port * AX25Sess, Byte mode);
void rst_timer(TAX25Port * AX25Sess);
void Send_UI(int port, Byte PID, char * CallFrom, char *CallTo, Byte *  Msg, int MsgLen);
#endif

BOOL ConvToAX25(char * callsign, unsigned char * ax25call);
int ConvFromAX25(unsigned char * incall, char * outcall);
void reverse_addr(Byte * path, Byte * revpath, int Len);
void set_DM(int snd_ch, Byte * path);
void set_link(TAX25Port * AX25Sess, UCHAR * axpath);
boolean is_last_digi(Byte *path);
boolean is_correct_path(Byte * path, Byte pid);
int number_digi(unsigned char * path);
void AX25_conn(TAX25Port * AX25Sess, int snd_ch, Byte mode);
void  write_ax25_info(TAX25Port * AX25Sess);
void rst_values(TAX25Port * AX25Sess);

#ifdef __cplusplus
extern "C"
{
#endif

extern boolean dyn_frack[4];
extern Byte recovery[4];
extern Byte users[4];

extern int resptime[4];
extern int slottime[4];
extern int persist[4];
extern int kisspaclen[4];
extern int fracks[4];
extern int frack_time[4];
extern int idletime[4];
extern int redtime[4];
extern int IPOLL[4];
extern int maxframe[4];
extern int TXFrmMode[4];

extern char MyDigiCall[4][512];
extern char exclude_callsigns[4][512];
extern char exclude_APRS_frm[4][512];

extern TStringList  list_exclude_callsigns[4];
extern TStringList list_exclude_APRS_frm[4];
extern TStringList list_digi_callsigns[4];

extern int max_frame_collector[4];
extern boolean KISS_opt[4];

extern TAX25Port AX25Port[4][port_num];

extern TStringList KISS_acked[];
extern TStringList KISS_iacked[];

#ifdef __cplusplus
}
#endif