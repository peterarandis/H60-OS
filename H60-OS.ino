
/*  HUSDATA.SE, Peter Hansson. h60@husdata.se
    Open H60 Arduino code as a foundation for own development projects on the H60 platform
    Provided as-is with no support. Can be freely used in personal och commercial projects.
    Fubnctions:
  - Wifi connection with DHCP and fixed SSID/PASS  
  - Basic code for communication with heat pump
  - Web page showing status and heat pump data
  - restAPI at  /api/alldata
  - LED status H60 red/green
  - Routine for button pushed
  - Debug printout via serial/usb port
*/

#include <ESP8266WiFi.h>       
#include <ESP8266WebServer.h>   // Webserver
#include <ESP8266HTTPClient.h>  // Web Client
#include <SoftwareSerial.h>     // Serial port for H1
#include <ArduinoJson.h>        // Json parser support
#include "HD_Globals.h"         
SoftwareSerial H1(13, 12, false, 1024);  // Init software serial  Pin 13, 12, non inv, buff for H1 H60
os_timer_t myTimer;                      // Init soft interrupt timer
#define LED 14                  // Define green LED
#include "HD_Utilities.h"       // Helper functions
#include "HD_Webserver.h"       // Internal webserver and API
#include "HD_H1comm.h"          // Communications towards H1 interface and heatpump

//===============================================================================
void setup() { // H60 Boot sequence 
  
  pinMode(LED, OUTPUT);       // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED, LOW);     // LED Status: ON: Init | Quick-blink: WifiOK, No Online | One-sec-blink: All OK
  Serial.begin(115200);       // Debug serial usb setup
  H1.begin(19200);            // H1 soft serial setup
  
  Serial.println("\n\n\r--------------------------------");  
  Serial.println      ("  Husdata H60 Demo open source");
  Serial.println      ("--------------------------------\n\n");  
   
  // ------------- WIFI CONNECT --------------
  Serial.println("Wifi connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.begin("Guest", ""); // Set SWSID and PASSWORD
  byte i=0;
  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if (i++>25) break;   // Try to connect 25 times every 0.5 sec
         }
      
  if (WiFi.status() == WL_CONNECTED) {
         trace("Wifi connected!","SER LOG LF");
         state_Wifi=true;
         }
        else 
         {
         trace("Giving up, no Wifi connection","SER LOG LF");
         WiFi.disconnect();
         } 
         
   digitalWrite(LED, HIGH);   // Turn off when Wifi Int process is done
  
  
   // ----------- GET IP, RSSI, MAC --------------
   rssi_update(); 
   trace("Signal strength: " + String(g_wifi_rssi),"SER LOG LF");
   sprintf(g_wifi_ip,"%d.%d.%d.%d", WiFi.localIP()[0],WiFi.localIP()[1],WiFi.localIP()[2],WiFi.localIP()[3]);  
   trace("DHCP IP address: " + String(g_wifi_ip), "SER LOG LF");
   byte mac[8];  WiFi.macAddress(mac); 
   sprintf(g_wifi_mac,"%02X%02X%02X%02X%02X%02X", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);   tolow(g_wifi_mac); trace("MAC address: " + String(g_wifi_mac),"SER LOG LF");

   // --------- INTERUPT TIMER INIT ------------------
   os_timer_setfn(&myTimer, timerCallback, NULL);    
   os_timer_arm(&myTimer, 1000, true); // every sec
  
   setup_webserver();    // Init Web server
   H1.write("\r"); // Flush

 
}



//---------------------------------------------------------------------------------------

void timerCallback(void *pArg) {  // Every second

 static byte B1_Count;
 static bool B1_Pushed;
 
  if (secondsCount++>=5)   every5seconds = true;
  if (mincount++>=60)      onemin=true;
  onesec=true;
  
  if (digitalRead(D2) == LOW) { // Read button press
         B1_Count++; // Button 1
         B1_Pushed=true;
         Serial.println("Button Pushed " + String(B1_Count) + " sec");
     
        }
    else
        { // Button release
          
          //if (B1_Count >5 ) {}  // Button 5 sek
          B1_Count=0;
          B1_Pushed = false;
        }
       

} // End of timerCallback

//####################################################################################################################################################################

void loop() {
  int i;
  // --- Fast loop  --

  H1_Recv();          // Receive H1 data
  H1_HandleData();    // Handle new data from interface
  SerialDebugRX();    // Read serial debug port
  webserver_loop();   // Handle Webserver requests
  FlashLed();
                           
  // ======= Every SEK ========
  if (onesec){ onesec=false;  }
      
  
  // ======= EVERY MINUTE ========
  if (onemin){ 
   
      if (WiFi.status() == 3) state_Wifi=true; else state_Wifi=false; // Check Wifi status 3 = ok, 0/1/6=fault
      uptime++;
      mincount=0;
      onemin=false; 
   }
   
  // ======= EVERY 5th SECOND ========
   if (every5seconds)  {  
        every5seconds=0;
        secondsCount=0;
        H1_CommInit(0);
            
       }
 
}
  

//---------------------------------------------------------------------------------------
void SerialDebugRX(){   // Data mottages pÃ¥ debugport
   
  char c;
  static String rxb;
 
 
  if(Serial.available())
    {
      c = (char)Serial.read();
      //Serial.print (c, DEC);
      if (c!=10 && c!=13) {rxb += c; return;}

      //handleCmds(rxb);
      rxb="";
    
    } 
}   



  
