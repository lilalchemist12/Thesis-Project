#include <SoftwareSerial.h>

SoftwareSerial gsmSerial(10, 9);
SoftwareSerial mcuSerial(5, 6);
char msg;

String fromSerial = "";
String studentNumber = "";
String message = "";
void setup() {
  gsmSerial.begin(9600);
  mcuSerial.begin(9600);
  Serial.begin(9600);

}

void loop() {
  
  if (gsmSerial.available () > 0) {
    Serial.write(gsmSerial.read());
  }
  if (mcuSerial.available () > 0) {

    while (mcuSerial.available() > 0) {
      char buffer = (char)mcuSerial.read();
      String letter = String(buffer);
      fromSerial += letter;
    }

    delay(1000);

    separatePayload(fromSerial);
    SendMessage();
    Serial.println("message sent");
  }
  




  delay(1000);
  studentNumber = "+63";
}

void SendMessage ()
{
  gsmSerial.println("AT+CMGF=1");
  delay(1000);
  gsmSerial.println(contactNumber(studentNumber));
  delay(1000);
  gsmSerial.println(message);
  delay(100);
  gsmSerial.println((char)26);
  delay(1000);
  fromSerial = "" ;
  studentNumber = "";
  message = "";
}

String contactNumber(String studentNumber) {
  String firstString = "AT+CMGS=\"";
  String secondString = "\"\r";
  String finalString = firstString + studentNumber + secondString;
  return finalString;
}

void separatePayload(String payload){
  studentNumber = payload.substring(0, payload.indexOf('-'));
  message = payload.substring(payload.indexOf('-') + 1);
  Serial.print("Student Number:");
  Serial.println(studentNumber);
  Serial.print("message: ");
  Serial.println(message);
  
}
