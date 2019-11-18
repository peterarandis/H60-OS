
byte H1_IDXtoID (String idx)
{
  byte i=0;
  for (i=0;i<indexNo;i++)  if (String(H1_ID[i]) == idx) return i;
  return 255; // Nothing found       
        
}

//------------------------------------------------------------------------------
void H1_HandleData() {   // Handle incoming H1 data
  
  if (!new_H1_data) return;
  
  //trace(incomingH1,"SER VB");
            
  byte i;
  int len;
  String row, sensid;
  char v[5];
  char *endptr;
  
// XR READ DATA UPDATE FROM SENSOR  
    if (incomingH1.substring(0,2)=="XR" && state_XL_ListRead){
        
        if (!state_HPComm &&  state_XL_ListRead) {state_HPComm = 1;trace("H1 XR received, HP comm OK.","SER LOG LF");}
     
        sensid = incomingH1.substring(2,6);  // Parse Sensor ID
    
        if (sensid[0] != 'A'){ // Ignore el meter
        
          i = H1_IDXtoID (sensid);
          if (i!=255)
            {
             incomingH1.substring(6,10).toCharArray(v,5);
             H1_ValueNow[i] = strtol(v, &endptr, 16);    // Lagra HP vÃ¤rde till variabel 
             //trace (incomingH1 + " " + H1_Name[i] + " " + H1_format_value (i, 0) + H1_unit_value (i),"SER LF");
             trace (H1_Name[i] + " " + H1_format_value (i, 0) + H1_unit_value (i),"SER LF");
            }
             
                
        }
    }  
      

// XL READ IN SENSOR LIST FROM INTERFACE         
    else if (incomingH1.substring(0,2)=="XL"){
        len = incomingH1.length();
       
        if (incomingH1.substring(3,6) == "EOF") {
                          state_H1Comm= 1; state_XL_ListRead=true; trace("H1 XL Sensor list loaded","SER LOG LF"); 
     
                          H1.write("XR\r");
                          }
              
        if (len>10 && state_XL_ListRead == false){
                incomingH1.substring(3,7).toCharArray(H1_ID[indexNo],5); // Save Index number 
                if (indexNo<40) H1_Name[indexNo] = incomingH1.substring(8, len-3);  // Store Description form H1
                indexNo++;
              }
        
        }


// XE, XM MESSAGE
    else if (incomingH1.substring(0,2)=="XE" || incomingH1.substring(0,2)=="XM"){
        len = incomingH1.length();
        trace("H1 Message: " + incomingH1.substring(0,len-2),"SER LOG LF"); 
       }
       
// XW SET VALUE STATUS     
    else if (incomingH1.substring(0,2)=="XW"){
        len = incomingH1.length();
        trace("H1 Set value response: " + incomingH1.substring(0,len-2),"SER LOG LF"); 
       }    

//XV READ VERSION AND TYPE OF INTERFACE  
    else if (incomingH1.substring(0,2)=="XV" && (H1_Type.length()<2 || H1_Ver.length()<2)){
        H1_Type = incomingH1.substring(2,4); // Set global VP id no
        H1_Ver = incomingH1.substring(4,9);   // Set global id interface FW
        H1_Ver.trim();
        trace(H1_PrintVpModel(),"LOG SER LF");
        
        
       }
    
   
    else {
         
         trace("H1>" + incomingH1,"LF SER VB");
         }
   
   new_H1_data = false;
   
  
}

//-------------------------------------------------------------------------------------------------
String H1_PrintVpModel()
{
String vpmod;

         if (H1_Type == "00")  vpmod = "Rego 600";
    else if (H1_Type == "05")  vpmod = "Rego 400";
    else if (H1_Type == "10")  vpmod = "Rego 2000/Pro Control 600";
    else if (H1_Type == "30")  vpmod = "Rego 1000/Pro Control 500";
    else if (H1_Type == "35")  vpmod = "IVT Rego 800"  ;          
    else if (H1_Type == "40")  vpmod = "NIBE EB100";
    else if (H1_Type == "50")  vpmod = "NIBE Styr2002";
    else if (H1_Type == "60")  vpmod = "Thermia Diplomat";
    else if (H1_Type == "70")  vpmod = "Thermia Villa";
    else return "Not initialized";
  
  return vpmod + ", Firmware: " + H1_Type+H1_Ver; //trace (t,"SER LOG LF");    

  //return t;

}
//-------------------------------------------------------------------------------------------------
void H1_Recv() {     // Data from H1 controller

  
  static byte ndx = 0;
  char endMarker = '\r';
  char rc;
  
  
  while (H1.available() > 0 && new_H1_data == false) {
    rc = H1.read();
   
    if (rc != endMarker) {
  
      receivedCharsH1 += rc;
      
      ndx++;
      if (ndx >= rx_buffsizeH1) {
               //ndx = rx_buffsizeH1 - 1;
               receivedCharsH1="";ndx=0;
              }
         
       
      if ((rc == '\n' || rc == '\r') && ndx < 2) {ndx=0;receivedCharsH1="";}   
    }
    else {
      //receivedChars[ndx] = '\0'; // terminate the string
      receivedCharsH1 += '\r';
      receivedCharsH1 += '\n';
      receivedCharsH1 += '\0';
      new_H1_data = true;
      incomingH1=receivedCharsH1;
      ndx = 0;
      receivedCharsH1="";
    }
    
      
    yield();
  }
  //}
}

//--------------------------------------------------------------------------------------------------------

void H1_hp_set(String idx, int value) // Set parameter in Heat pump
{
  char v_hex[10];
  String s;
  const char *tmp;

  tmp = idx.c_str();
  
  sprintf(v_hex,"XW%s%04hX\r", tmp, value);

  Serial.println("H1 Set:" +String(v_hex));
  
  H1.write(v_hex);
  
}
//-------------------------------------------------------------------------------------------------

String H1_unit_value (char idx)

{
   char z[2];
   int unit;
   String val;  
   char *endptr;
   
   z[0] = H1_ID[idx][0];
   unit = strtol(z,&endptr, 16);                // Hex to Int
      
   switch (unit)
   {
     case 0:  val = "C";     break; // Temp  ALT + 0186
     case 1:  val = "";      break; // State
     case 2:  val = "";      break; // Number 
     case 3:  val = "%";     break; // Percent      
     case 4:  val = "Amp";   break; 
     case 5:  val = "kWh";   break; 
     case 6:  val = "hr";    break; 
     case 7:  val = "min";   break; 
     case 8:  val = "C min"; break; // ALT + 0186
     case 9:  val = "kw";    break; 
     case 10: val = "pulse"; break; 
   }

  return val; 
}

//-------------------------------------------------------------------------------------------------
String H1_format_value (char idx, char type)
{
 char z[2];
 char *endptr;
 String val;
 float v;
 int i;
 int unit;
    
   
   z[0] = H1_ID[idx][0];
   unit = strtol(z,&endptr, 16);                // Hex to Int
   i = int(H1_ValueNow[idx]);
   v = float(H1_ValueNow[idx]);

   if (v > 32768) v = v - 0x10000; // Negative if Highest bit is set           
   if (v == 0x8000){v = 0;}        // No sensor connected
  
       
   if (H1_Type == "50") // Styr2002
      {if (v!=0 && unit!=1 && unit!=6 && unit!=7 && unit!=10) v=v/10;}             // Divide number with 10 to convert to float                
     else
      {if (v!=0 && unit!=1 && unit!=2 && unit!=6 && unit!=7 && unit!=10) v=v/10;}  // Divide number with 10 to convert to float    

   if (H1_Type == "00" && String(H1_ID[idx]) == "2205") v=v/10;     // Exception Heatcurve Rego 600
   if (H1_Type == "30" && String(H1_ID[idx]) == "2204") v=v/10;     // Exception Influence Rego 1000
   
      
   val = String(v); // default
   if (unit==0 || unit==2 || unit==3) // Temp, Num, Percent
        val = String(v);
      else
        val = String(i);

  
  if      (type==0)    return val;                 
  else if (type==2)    return String(int(v*10));  
  else                 return "err";
  
}


//-------------------------------------------------------------------------------------------------

void H1_CommInit(bool val) // Run from Loop initialize and secure H1 communication initialization
{


 if (!val)
   {   // RUN OFTEN, 10 SECONDS FOR INIT
   // First read out of the H1 interface Version
        if (!H1_Type.length()){
            trace("Requesting Version H1...","SER LOG LF");
            H1.write("XV\r"); // Skicka XV, Check version
           }
         
    // Next get the sensor list
        if (!state_XL_ListRead && H1_Type.length()){
            trace("Requesting sensorlist H1...","SER LOG LF");
            H1.write("XL\r"); // Skicka XL, List of sensors
           }
           
   }
    else
    {   // RUN AT 20 MIN, CHECK STATUS AND SECURE COMM

      // Läs in allt från H1 för refresh.
         if (state_XL_ListRead && H1_Type.length()){
            trace("Periodic re-read all from H1...","SER LOG LF VB");
            H1.write("XR\r"); // Skicka XR,
           }
    
   }

   
           
}
