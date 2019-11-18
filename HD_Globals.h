// Prototypes -------------
String H1_format_value (char idx, char type);
String H1_unit_value (char idx);
byte H1_IDXtoID (String idx);
void timerCallback(void *pArg);
void SerialDebugRX();   // Tar emot data from inbyggd serieport
void H1_Recv();          // Tar emot data from H1 serieport
void H1_HandleData();
void rssi_update();
String H1_PrintVpModel();

// Global variables -----------------

// Serial comm
bool new_H1_data=false;                   // Flagga ny inkommen data
const char rx_buffsizeH1 = 70;            // Serial buffer rcv
byte stored_minute_ptr = 0;               // Index lagrad minut
String receivedCharsH1;                   // Mottaget H1 Serial
String incomingH1;                        // Mottagsbuffer2 H1 for behandling
String debug_log;                         // Large string contaning last debug log

// Time counters
bool onesec=false;                        // Varje sekund
byte secondsCount=0;
byte mincount=0; bool onemin=false;       // Varje minute
bool every5seconds=false;                 // Var 5:e sekund
byte FailCounter = 15;                    // Retry x times, then reboot ESP
unsigned int uptime=0;                    // Minuter upptid sedan boot

char g_wifi_rssi[20] = "";
char g_wifi_ip[20];
char g_wifi_mac[20];


// H1 comm related
String H1_Ver="";
String H1_Type="";
String H1_NewVer=""; // Avail new ver
char   H1_ID[50][6];    // XL List of sensors, max 50 
String H1_Name[40];     // XL List of sensors
int    H1_Value[50][10];   // Value sample, Last 5 minutes
int    H1_ValueNow[50];    // Value sample
int    indexNo=0;

bool   flash_ip=false;  // Led flash

//--------STATUS--------
bool state_Wifi=0;
bool state_HttpServer=0;
bool state_H1Comm=0;
bool state_HPComm=0;
bool state_XL_ListRead = false;
bool state_Indexes_Received=0;
