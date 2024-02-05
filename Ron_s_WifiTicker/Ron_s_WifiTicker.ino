/*
Project: Wifi controlled LED matrix display
NodeMCU pins    -> EasyMatrix pins
MOSI-D7-GPIO13  -> DIN
CLK-D5-GPIO14   -> Clk
GPIO0-D3        -> LOAD

*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <stdio.h>
#include "FS.h"

#define SSID ""                      // insert your SSID
#define PASS ""                    // insert your password
int wait = 60; // In milliseconds
int intensity = 7; //0-15
// ******************* String form to sent to the client-browser ************************************

// String form =
//   "<html>"
//   "<head>"
//   "<style>"
//   "body { background-color: black; color: white; }"
//   "input { font-size: 16px; height: 40px; }"
//   "</style>"
//   "</head>"
//   "<body>"
//   "<center>"
//   "<h1>JACOB's Thick Ticker</h1>"
//   "<form action='msg'><p>Type your message <input type='text' name='msg' style='width: 200px;' autofocus> <input type='submit' value='Submit'></form>"
//   "<form action='speed'><p>Speed 10=fast 1=slow <input type='number' name='speed' style='width: 100px;' autofocus> <input type='submit' value='Submit'></form>"
//   "<form action='intensity'><p>Enter Brightness 0 - 15 <input type='number' name='intensity' style='width: 50px;' autofocus> <input type='submit' value='Submit'></form>"
//   "</center>"
//   "</body>"
//   "</html>";

String form = 
  "<html lang=\"en\">"
  "<head>"
  "<meta charset=\"UTF-8\">"
  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
  "<link href=\"https://cdn.jsdelivr.net/npm/tailwindcss@2.2.19/dist/tailwind.min.css\" rel=\"stylesheet\">"
  "<title>JACOB's Thick Ticker</title>"
  "</head>"
  "<body class=\"bg-black text-white\">"
  "<div class=\"container mx-auto p-4\">"
  "<div class=\"text-center\">"
  "<h1 class=\"text-3xl font-bold mb-8\">JACOB's Thick Ticker</h1>"

  "<form action=\"msg\" class=\"mb-4\">"
  "<label class=\"block mb-2\">Type your message</label>"
  "<input type=\"text\" name=\"msg\" class=\"w-48 px-4 py-2 border rounded focus:outline-none focus:border-blue-500\" autofocus>"
  "</form>"

  "<form action=\"speed\" class=\"mb-4\">"
  "<label class=\"block mb-2\">Speed 10=fast 1=slow</label>"
  "<input type=\"number\" name=\"speed\" class=\"w-24 px-4 py-2 border rounded focus:outline-none focus:border-blue-500\" autofocus>"
  "</form>"

  "<form action=\"intensity\" class=\"mb-4\">"
  "<label class=\"block mb-2\">Enter Brightness 0 - 15</label>"
  "<input type=\"number\" name=\"intensity\" class=\"w-16 px-4 py-2 border rounded focus:outline-none focus:border-blue-500\" autofocus>"
  "</form>"

  "<form action=\"submit\" class=\"mt-4\">"
  "<input type=\"submit\" value=\"Submit\" class=\"px-4 py-2 bg-blue-500 text-white rounded cursor-pointer\">"
  "</form>"

  "</div>"
  "</div>"
  "</body>"
  "</html>";



ESP8266WebServer server(80);                             // HTTP server will listen at port 80
long period;
int offset=1,refresh=0;
int pinCS = D6; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;
int ledRotation = 1;
String decodedMsg;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

String tape = "Arduino";

int spacer = 1;
int width = 5 + spacer; // The font width is 5 pixels

/*
  handles the messages coming from the webbrowser, restores a few special characters and 
  constructs the strings that can be sent to the oled display
*/
void handle_msg() {
                        
  matrix.fillScreen(LOW);
  server.send(200, "text/html", form);    // Send same page so they can send another msg
  refresh=1;
  // Display msg on Oled
  String msg = server.arg("msg");
  int speed = server.arg("speed").toInt();
  Serial.println(msg);
  Serial.println(speed);
  decodedMsg = msg;
  // Restore special characters that are misformed to %char by the client browser
  decodedMsg.replace("+", " ");      
  decodedMsg.replace("%21", "!");  
  decodedMsg.replace("%22", "");  
  decodedMsg.replace("%23", "#");
  decodedMsg.replace("%24", "$");
  decodedMsg.replace("%25", "%");  
  decodedMsg.replace("%26", "&");
  decodedMsg.replace("%27", "'");  
  decodedMsg.replace("%28", "(");
  decodedMsg.replace("%29", ")");
  decodedMsg.replace("%2A", "*");
  decodedMsg.replace("%2B", "+");  
  decodedMsg.replace("%2C", ",");  
  decodedMsg.replace("%2F", "/");   
  decodedMsg.replace("%3A", ":");    
  decodedMsg.replace("%3B", ";");  
  decodedMsg.replace("%3C", "<");  
  decodedMsg.replace("%3D", "=");  
  decodedMsg.replace("%3E", ">");
  decodedMsg.replace("%3F", "?");  
  decodedMsg.replace("%40", "@"); 
  //Serial.println(decodedMsg);                   // print original string to monitor
 
 
    
  //Serial.println(' ');                          // new line in monitor
}

void handle_speed() {
  server.send(200, "text/html", form);    // Send same page so they can send another msg
  refresh=1;
  // Display msg on Oled
  String msg = server.arg("msg");

  wait = server.arg("speed").toInt();
  // Assuming speedMin = 40 and speedMax = 15
  int speedMin = 1;
  int speedMax = 10;

  // The new range you want (1 to 10)
  int newRangeMin = 15;
  int newRangeMax = 40;

  // Map the original speed to the new range
  int mappedSpeed = map(wait, speedMax, speedMin, newRangeMin, newRangeMax);
  wait = mappedSpeed;
  // Now 'mappedSpeed' will be the speed in the range of 1 to 10
  Serial.println(mappedSpeed);
  Serial.println(msg);
  Serial.println(wait);
}

void handle_intensity() {
  server.send(200, "text/html", form);    // Send same page so they can send another msg
  refresh=1;
  // Display msg on Oled
  intensity = server.arg("intensity").toInt();
  matrix.setIntensity(intensity); // Use a value between 0 and 15 for brightness
  Serial.println(intensity);
}

void setup(void) {
matrix.setIntensity(1); // Use a value between 0 and 15 for brightness

// Adjust to your own needsmatrix.setIntensity(15); // Use a value between 0 and 15 for brightness
  matrix.setRotation(0, ledRotation);
  matrix.setRotation(1, ledRotation);
  matrix.setRotation(2, ledRotation);
  matrix.setRotation(3, ledRotation);

//  matrix.setPosition(0, 1, 0); // The first display is at <0, 0>
//  matrix.setPosition(1, 0, 0); // The second display is at <1, 0>

// Adjust to your own needs
//  matrix.setPosition(0, 7, 0); // The first display is at <0, 7>
//  matrix.setPosition(1, 6, 0); // The second display is at <1, 0>
//  matrix.setPosition(2, 5, 0); // The third display is at <2, 0>
//  matrix.setPosition(3, 4, 0); // And the last display is at <3, 0>
//  matrix.setPosition(4, 3, 0); // The first display is at <0, 0>
//  matrix.setPosition(5, 2, 0); // The second display is at <1, 0>
//  matrix.setPosition(6, 1, 0); // The third display is at <2, 0>
//  matrix.setPosition(7, 0, 0); // And the last display is at <3, 0>
  matrix.setPosition(0, 0, 0); // The first display is at <0, 0>
  matrix.setPosition(1, 1, 0); // The second display is at <1, 0>
  matrix.setPosition(2, 2, 0); // The third display is at <2, 0>
  matrix.setPosition(3, 3, 0); // And the last display is at <3, 0>

//ESP.wdtDisable();                               // used to debug, disable wachdog timer, 
  Serial.begin(115200);                           // full speed to monitor
                               
  WiFi.begin(SSID, PASS);                         // Connect to WiFi network
  while (WiFi.status() != WL_CONNECTED) {         // Wait for connection
    delay(500);
    Serial.print(".");
  }
  // Set up the endpoints for HTTP server,  Endpoints can be written as inline functions:
  server.on("/", []() {
    server.send(200, "text/html", form);
  });
  server.on("/msg", handle_msg);                  // change message
  server.on("/speed", handle_speed);                  // change speed
  server.on("/intensity", handle_intensity);      // change brightness
  server.begin();                                 // Start the server 


  Serial.print("SSID : ");                        // prints SSID in monitor
  Serial.println(SSID);                           // to monitor             
 
  char result[16];
  sprintf(result, "%3d.%3d.%1d.%3d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  Serial.println();
  Serial.println(result);
  decodedMsg = result;
  Serial.println("WebServer ready!   ");

  Serial.println(WiFi.localIP());                 // Serial monitor prints localIP
  Serial.print(analogRead(A0));
  
}


void loop(void) {

  for ( int i = 0 ; i < width * decodedMsg.length() + matrix.width() - 1 - spacer; i++ ) {
    server.handleClient();                        // checks for incoming messages
    if (refresh==1) i=0;
    refresh=0;
    matrix.fillScreen(LOW);

    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically
 
    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < decodedMsg.length() ) {
        // matrix.drawChar(x, y, decodedMsg[letter], HIGH, LOW, 1);

      }

      letter--;
      x -= width;
    }

    matrix.write(); // Send bitmap to display

    delay(wait);
  }
}
