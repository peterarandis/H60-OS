void trace (String text, String func) ;


// Time / NTP
#define TZ              0               // (utc+) TZ in hours
#define DST_MN          60              // use 60mn for summer time in some countries
#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)
timeval tv;
time_t now;

void FlashLed()
{

 static unsigned long m=0 ; // Millisec mem
 static byte pattern_pos=0;
 byte pattern_no = 0;
 // Flash LED pattern, on-time, off-time....
 const int pattern[][15]= { {1,5,5000,0},                       // 0: Dark
                            {1,70,3000,0},                      // 1: .        Wifi Err
                            {1,70,300,70,3000,0},               // 2: . .      H1 Err
                            {1,70,300,70,300,70,3000,0},        // 3: . . .    Not used
                            {1,70,300,70,70,300,300,70,3000,0}, // 4: . . . .  Not used
                            {1,1000,1000,0},                    // 5: -   -    Slow flash, all OK
                            {1,50,50,0},                        // 6: .......  Super fast flash, AP mode not used
                            {1,70,300,70,300,700,2000,0},       // 7: . . -    Test not used
                            }; 

if(!flash_ip)
 {
 
     if      (!state_Wifi) pattern_no = 1 ;                                    // ERR: Wifi Not Connected      ".     "
     else if (!state_XL_ListRead)  pattern_no = 2 ;                            // ERR: Ej H1 kontakt           ". .    "
     else     pattern_no = 5;                                                  // All is ok SLOW FLASH
  
 }


  if (!pattern[pattern_no][pattern_pos]) pattern_pos=0;  
  if (pattern_pos == 0) {m = millis(); digitalWrite(LED, HIGH);pattern_pos++;}    // INIT
  else if ( pattern_pos % 2 == 0)
       {if (millis() > m + pattern[pattern_no][pattern_pos]) {m = millis();digitalWrite(LED, HIGH);pattern_pos++;} }  // EVEN 2,4,6
     else  
       {if (millis() > m + pattern[pattern_no][pattern_pos]) {m = millis();digitalWrite(LED, LOW);pattern_pos++;} }  // ODD   1,3,5
  
  
}


//-----------------------------------------------------------------------------------------------           

void rssi_update()
{
   int i = WiFi.RSSI();
   char s[7] = "Strong";
   if (i< -60) sprintf(s,"Good");
   if (i< -70) sprintf(s,"Okay");
   if (i< -80) sprintf(s,"Weak");
   if (i< -90) sprintf(s,"Bad");
   sprintf(g_wifi_rssi,"%s %ddBm", s, i); // Uppdatera signalstyrka
}

//-----------------------------------------------------------------------------------------------

void trace (String text, String func)  // SER LOG LF
{
  Serial.println(text);
}
 
//-----------------------------------------------------------------------------------------------

char *tolow (char *s)
{
   char *p;
   for (p = s; *p != '\0'; ++p)  { *p = tolower(*p);  }
   return s;  
}






 
