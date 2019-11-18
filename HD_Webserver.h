ESP8266WebServer *server;

void H1_hp_set(String idx, int value);
void handleNotFound();
void ApiAllData();
void ApiSet();

const String title = "<!DOCTYPE html><html><head><title>";

const String header = "\
     </title><meta name='viewport' content='width=device-width, initial-scale=1.0'></head>\
     <style>\
       .rounded td{border:solid #133670 6px; border-radius:6px;text-align:center;}\
       table{font-size: 110%; color: black; background-color:white; border-collapse:separate; border:solid #133670 6px; border-radius:6px; }\
       input{font-size: 90%;}\  
       body,textarea,a{font-size: 120%; }\
       .setting {font-size: 75%; }\
     @media only screen and (max-width:500px) {\  
       table{font-size: 100%; border:solid #133670 2px; border-radius:6px; }\
       body {font-size: 80%;}\
       input{font-size: 80%;}\
       textarea,a{font-size: 140%; }\
       }\
     </style>";
     
const String footer = "<br><br>(C) 2019 Arandis AB  Husdata.se\
                </body></html>";
  
//---------------------------------------------------------------------------------------------------------------

void pageRoot() {
  
  String buff,typ;
  bool col=0;
  String bgcolor;
  byte i;
  int ID=99;
  float v;
      
    buff =  title + "H60" + header;
    buff += "<br><form action='/' method='post' autocomplete='off'>\
    <table width=100% border=0 cellspacing=0 cellpadding=4>\
             <tr  bgcolor=#133670><td width=40><font color='white'><b>&nbsp;Status</td><td width=40><font color='white'></td></tr>\
             <tr><td>&nbsp;IP Address</td><td>" + String(g_wifi_ip) + "</td></tr>\
             <tr><td>&nbsp;Mac Address</td><td>" +  String(g_wifi_mac) + "</td></tr>\
             <tr><td>&nbsp;Signal strength</td><td>" + String(g_wifi_rssi) + "</td></tr>\
             <tr><td>&nbsp;Interface cable</td><td>" + H1_PrintVpModel() + "</td></tr>";
                                              
     buff+="</table>";
     
     if(server->args()>0) // Any arguments
     {
          Serial.println ("Arg:" + server->argName(0) + " " + server->arg(0)); 
          if (server->argName(0)=="id") // If ID some item has been selected for setting
                  {
                    ID = server->arg("id").toInt();
                  } 
               else // Otherwise some item is saving
                  {
                    ID=99; // Nothing
                    //v = server->arg(0); // Värde som är satt
                    v = server->arg(0).toFloat(); 
                                   
                    if (server->arg(0).length() > 0)
                        //if (H1_format_value(ID,0) != String(v)) // If Changed
                             { 
                               
                                // Undantag för de settings som ej ska multipliceras med 10
                               if  (H1_Type == "00" && (server->argName(0) == "2204"))   // REGO600 
                                     H1_hp_set(server->argName(0), v);
                                 else 
                                     H1_hp_set(server->argName(0), v*10); // Multiply by 10 for most settings
                               
                               buff += "<br>" + H1_Name [H1_IDXtoID(server->argName(0))] + " has been set to " + v  + "<br>";
                          
                             } 
                             //   else
                             //    Serial.println ("No change"); 
                            else   
                        
                        buff += "<br>Setting value has too few characters!<br>";
                    //    f.print(buff);
                  }   
     }           
                    

     
     buff+="<br>\
     <table width=100% cellspacing=0 cellpadding=4 border = 0>\
             <tr bgcolor=#133670>\
                <td width=20%><font color='white'><b>&nbsp;Idx</td>";
               
     buff +=   "<td width=40%><font color='white'><b>Name</td>\
                <td width=30%><font color='white'><b>Value</td>\
            </tr>";


  
  for (i=0;i<indexNo;i++)
  {
  if (H1_ID[i][0] != 'A') // Dont display Electrical meters (AFF1, AFF2)
   {
 
 
   if (H1_ID[i][1] == '2')   // If a settable parameter
      {
       
       if (col)  {bgcolor="b3b3b3";col=false;} else {bgcolor="c5c5c5";col=true;}
       buff += "<tr bgcolor='#" + bgcolor + "'><td>&nbsp;<a href='?id=" + String(i) + "'><span class='setting'>" + String(H1_ID[i]) + "</span></a></td>";   //SETTABLE
      } 
     else
      { // if NOT a setting
       if (col)  {bgcolor="eeeeee";col=false;} else {bgcolor="FFFFFF";col=true;}
       buff += "<tr bgcolor='#" + bgcolor + "'><td>&nbsp;" + String(H1_ID[i])    + "</td>";   // NON Settable
      }
      
 
   buff +=    "<td>" + H1_Name[i]  + "</td>";

   if (i != ID) // Print Value
          buff +=    "<td>" + H1_format_value(i,0) + "&nbsp;" + H1_unit_value (i) + "</td>";
      else
          buff +="<td><input type='text' size='4' maxlength='8' name='" + String(H1_ID[i]) + "' autofocus value='" + H1_format_value(i,0) + "'>&nbsp;" + H1_unit_value (i) + "&nbsp;<input type='submit' value='Set'></td></tr>";

   
   buff += "</tr>";
  
   }
  }
   buff += "</table></form>";
   
 server->send(200, "text/html", buff+footer);
  
 rssi_update(); 
}



//----------------------------------------------------------------------------------------------

void handleNotFound() {
  String buff = "Page Not Found\n\n";
  buff += "URI: ";
  buff += server->uri();
  buff += "\nMethod: ";
  buff += (server->method() == HTTP_GET) ? "GET" : "POST";
  buff += "\nArguments: ";
  buff += server->args();
  buff += "\n";
  for (uint8_t i = 0; i < server->args(); i++) {
    buff += " " + server->argName(i) + ": " + server->arg(i) + "\n";
  }
  server->send(404, "text/plain", buff);
}

//----------------------------------------------------------------------------------------------

void setup_webserver(void) {

  server = new ESP8266WebServer(80);

  server->on("/", pageRoot);
  server->on("/api/alldata", ApiAllData); 
  server->on("/api/set", ApiSet);         
  server->onNotFound(handleNotFound);
  server->begin();
  trace("HTTP server started ", "SER LOG LF");
  state_HttpServer = true;

}


//----------------------------------------------------------------------------------------------

void ApiAllData() { // Read all registers in JSON format
  String buff;
  byte i;
  //{"0001":238,"0002":459,"0003":167,"0004":426,"0005":26,"0006":43,"0007":8,"0008":133,"0009":356,"000A":388,"000B":310,"3104":472,"0107":364,"0111":278,"0203":204,"2204":36,"2205":230,"0207":441,"0208":229,"7209":214,"1A01":1,"1A02":0,"1A03":0,"1A04":1,"1A05":0,"1A06":1,"1A07":1,"1A20":0}
   
   buff = "{";
   for (i=0;i<indexNo;i++)
   {
        if (i!=0) buff += ",";
        buff += String(char(34));  
        buff += String(H1_ID[i]) + String(char(34));
        buff += ":";
        buff += String(H1_ValueNow[i]);
   }
   buff += "}";
   //Serial.print ("A");
   server->send(200, "text/plain", buff);
}

//----------------------------------------------------------------------------------------------
void ApiSet() { // Write to one register using URL Query string
  String buff;
  String response = "err";
  float v;
  bool Err=0;
  // API Setting has to be 2 or larger for set access
  buff = "{" + String(char(34)) + "response" + String(char(34)) + ":" + String(char(34));
    
  if(server->args() == 2) // Any arguments?
     {
         //Serial.println ("Arg:" + server->argName(0) + " " + server->arg(0)); 
         if (server->argName(0)=="idx" && server->argName(1)=="val") // http://ip/api/set?idx=2204&val=20
              {
               if (server->arg(0).length() != 4) {buff += "Invalid idx}";Err=1;} 
               if (server->arg(1).length() < 1)  {buff += "Invalid val}";Err=1;}

               if (!Err)
                  {v = server->arg(1).toFloat(); 
                   H1_hp_set(server->arg(0), v);
                   response = "Ok" ;
                   trace ("API: " + H1_Name [H1_IDXtoID(server->arg(0))] + " has been set to " + v, "SER LOG LF");
                  }       
              } 
           else
            response = "Format error";
          
     }           
  buff += response + String(char(34)) + "}";
  server->send(200, "text/plain", buff);
}


//----------------------------------------------------------------------------------------------
void webserver_loop(void) {
  server->handleClient();
}
