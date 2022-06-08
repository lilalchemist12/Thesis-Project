
#include <ESP8266WiFi.h>     //Include Esp library
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>        //include RFID library
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
//#include <Servo.h>
//#include <SoftwareSerial.h>

#define SDA_PIN D4 //RX slave select
#define RST_PIN D3
#define gLed D2
#define buzzer D8
#define lock D0

MFRC522 mfrc522(SDA_PIN, RST_PIN); // Create MFRC522 instance.
LiquidCrystal_I2C lcdLogin(0x27, 16, 2);
LiquidCrystal_I2C lcdLogout(0x26, 16, 2);

//Servo servo;

/* Set these to your desired credentials. */
const char *ssid = "B310_62121";  //ENTER YOUR WIFI SETTINGS
const char *password = "Password@2127";

//Web/Server address to read/write from
const char *host = "192.168.254.117";   //IP address of server

int dservo = 2200;

String getData , Link;
String CardID = "";
String studentStatus = "";
String studentNumber = "";

ESP8266WebServer server(80);
char temp[400];
void handleRoot()
{
  snprintf(temp, 400,
           "<html>\
  <head>\
  </head>\
    <body>\
       <h1>EMERGENCY CONTROL</h1>\
       <a href =\"/1\"> <button onclick=\"alert('Emergency Unlock ?')\"style=\"/font-family:century gothic;color:White;font-size:20px;background-color:Red;\">LOCK OFF</button></a>\
       <a href =\"/0\"> <button onclick=\"alert('Lock it back ?')\" style=\"/font-family:century gothic;color:White;font-size:20px;background-color:Green;\">LOCK BACK</button></a>\
       <a href=\"/localhost/loginsystem\"><button>Home</a></button></a>\
       </body>\
</html>");
  server.send(200, "text/html", temp);
}


void setup() {
  lcdLogin.init();
  lcdLogout.init();

  pinMode(lock, OUTPUT);
  pinMode(buzzer, OUTPUT);
  noTone(buzzer);
  //servo.attach(16);
  //servo.write(0);
  delay(1000);
  Serial.begin(9600);
  SPI.begin();  // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card

  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot

  WiFi.begin(ssid, password);     //Connect to your WiFi router

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);

  }

  //If connection successful show IP address in serial monitor

  server.on("/", handleRoot);
  server.on("/0", lockBack);
  server.on("/1", lockOff);

  server.begin();


  Wire.begin(D2, D1);

  lcdLogin.backlight();
  lcdLogout.backlight();


  lcdLogin.setCursor(0, 0);
  lcdLogin.print("Good Day! Please");
  lcdLogin.setCursor(1, 1);
  lcdLogin.print("Scan To Login");

  lcdLogout.setCursor(0, 0);
  lcdLogout.print("Good Day! Please");
  lcdLogout.setCursor(1, 1);
  lcdLogout.print("Scan To Logout");

  digitalWrite(lock, HIGH);
  tone(buzzer, 1000);
  delay(300);
  noTone(buzzer);
}

void loop() {

  server.handleClient();

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);

    }

  }

  //look for new card
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;//got to start of loop if there is no card present
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;//if read card serial(0) returns 1, the uid struct contians the ID of the read card.
  }

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    CardID += mfrc522.uid.uidByte[i];
  }



  WiFiClient client;

  HTTPClient http;    //Declare object of class HTTPClient

  //GET Data
  getData = "?CardID=" + CardID;  //Note "?" added at front
  Link = "http://192.168.254.117/loginsystem/postdemo.php" + getData;

  http.begin(client, Link);

  int httpCode = http.GET();            //Send the request
  delay(10);
  String payload = http.getString();    //Get the response payload


  separatePayload(payload);
  if (studentStatus == "login") {
    lcdLogin.clear();

    lcdLogin.setCursor(4, 0);
    lcdLogin.print("Welcome!");
    lcdLogin.setCursor(0, 1);
    lcdLogin.print("Have A Nice Day");
    //servo.write(180);
    digitalWrite(lock, LOW);
    tone(buzzer, 800);
    delay(300);
    noTone(buzzer);
    //delay(500);  //Post Data at every 5 seconds
    delay(dservo);
    studentNumber += "-student has logged in";
    Serial.print(studentNumber);
    studentNumber = "";

  }
  else if (studentStatus == "logout") {
    lcdLogout.clear();

    lcdLogout.setCursor(4, 0);
    lcdLogout.print("Goodbye!");
    lcdLogout.setCursor(0, 1);
    lcdLogout.print("Have A Nice Day");
    //servo.write(180);
    digitalWrite(lock, LOW);
    tone(buzzer, 600);
    delay(300);
    noTone(buzzer);
    //delay(500);  //Post Data at every 5 seconds
    delay(dservo);
    studentNumber += "-student has logged out";
    Serial.print(studentNumber);
    studentNumber = "";
  }
  else if (studentStatus == "succesful" || payload == "Cardavailable") {
    lcdLogin.clear();
    lcdLogout.clear();

    lcdLogin.print("Please Register");
    lcdLogin.setCursor(3, 1);
    lcdLogin.print("Your Card");

    lcdLogout.print("Please Register");
    lcdLogout.setCursor(3, 1);
    lcdLogout.print("Your Card");

    digitalWrite(lock, HIGH);
    tone(buzzer, 1200);
    delay(200);
    noTone(buzzer);
    delay(100);
    tone(buzzer, 800);
    delay(200);
    noTone(buzzer);
    delay(100);
    tone(buzzer, 800);
    delay(200);
    noTone(buzzer);
    delay(100);
    tone(buzzer, 800);
    delay(200);
    noTone(buzzer);
    delay(700);
  }

  delay(500);

  CardID = "";
  getData = "";
  Link = "";
  http.end();  //Close connection

  lcdLogin.clear();
  lcdLogout.clear();

  lcdLogin.print("Good Day! Please");
  lcdLogin.setCursor(1, 1);
  lcdLogin.print("Scan To Login");

  lcdLogout.print("Good Day! Please");
  lcdLogout.setCursor(1, 1);
  lcdLogout.print("Scan To Logout");

  tone(buzzer, 1000);
  //servo.write(0);
  digitalWrite(lock, HIGH);
  delay(300);
  noTone(buzzer);

}

void lockBack()
{
  digitalWrite(lock, HIGH);

  lcdLogin.clear();
  lcdLogout.clear();
  delay(100);

  lcdLogin.setCursor(5, 0);
  lcdLogin.print("Back To");
  lcdLogin.setCursor(2, 1);
  lcdLogin.print("Being Locked");

  lcdLogout.setCursor(5, 0);
  lcdLogout.print("Back To");
  lcdLogout.setCursor(2, 1);
  lcdLogout.print("Being Locked");

  tone(buzzer, 1000);
  delay(300);
  noTone(buzzer);
  delay(600);
  server.send(200, "text/html", temp);
  delay(1500);


  lcdLogin.clear();
  lcdLogout.clear();

  lcdLogin.print("Good Day! Please");
  lcdLogin.setCursor(1, 1);
  lcdLogin.print("Scan To Login");

  lcdLogout.print("Good Day! Please");
  lcdLogout.setCursor(1, 1);
  lcdLogout.print("Scan To Logout");
  tone(buzzer, 1000);
  digitalWrite(lock, HIGH);
  delay(300);
  noTone(buzzer);
}

void lockOff()
{
  digitalWrite(lock, LOW);

  lcdLogin.clear();
  lcdLogout.clear();
  delay(100);

  lcdLogin.setCursor(3, 0);
  lcdLogin.print("EMERGENCY!");
  lcdLogin.setCursor(4, 1);
  lcdLogin.print("Unlocked");

  lcdLogout.setCursor(3, 0);
  lcdLogout.print("EMERGENCY!");
  lcdLogout.setCursor(4, 1);
  lcdLogout.print("Unlocked");

  tone(buzzer, 1500);
  delay(5000);
  noTone(buzzer);
  server.send(200, "text/html", temp);
}

void separatePayload(String payload) {
  studentStatus = payload.substring(0, payload.indexOf('-'));
  studentNumber = payload.substring(payload.indexOf('-') + 1);
  
}
//=======================================================================
