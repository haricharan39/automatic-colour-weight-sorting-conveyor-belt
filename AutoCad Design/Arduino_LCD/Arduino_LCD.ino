#include <LiquidCrystal.h>
LiquidCrystal lcd(2, 3, 4, 8, 12, 13);
String incoming = "";
void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting...");
}
void loop() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      processMessage(incoming);
      incoming = "";
    } else if (c != '\r') {
      incoming += c;
    }
  }
}
void processMessage(String msg) {
  msg.trim();
  if (msg == "STATUS:READY") {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Ready");
    return;
  }
  if (msg == "STATUS:WAIT") {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scanning...");
    return;
  }
  int wStart = msg.indexOf("W:") + 2;
  int wEnd   = msg.indexOf(",C:");
  int cStart = msg.indexOf(",C:") + 3;
  if (wStart < 2 || wEnd < 0 || cStart < 3) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Parse error");
    return;
  }
  String wLabel = msg.substring(wStart, wEnd);
  String cLabel = msg.substring(cStart);
  wLabel.trim();
  cLabel.trim();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wt: ");
  lcd.print(wLabel);
  lcd.setCursor(0, 1);
  lcd.print("Color: ");
  lcd.print(cLabel);
}