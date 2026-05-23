#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

const char* ssid = "ichi ni san";
const char* password = "";

IPAddress apIP(192,168,4,1);   
DNSServer dnsServer;
WebServer webServer(80);

const char* validUser = "admin";
const char* validPass = "1234";

void PortalCaptivo() {
  String html = "<!DOCTYPE html><html><head><title>Portal Captivo</title></head><body>";
  html += "<h1>Identificarse</h1>";
  html += "<form action='/login' method='POST'>";
  html += "Usuario: <input type='text' name='usuario'><br>";
  html += "Contraseña: <input type='password' name='contraseña'><br>";
  html += "<input type='submit' value='Login'>";
  html += "</form></body></html>";
  webServer.send(200, "text/html", html);
}

void Login() {
  if (webServer.hasArg("usuario") && webServer.hasArg("contraseña")) {
    String usuario = webServer.arg("usuario");
    String contra = webServer.arg("contraseña");

    if (usuario == validUser && contra == validPass) {
      webServer.send(200, "text/html", "<h1>Bienvenido administrador</h1><p>ESP32-Access Point.</p>");
    } else {
      webServer.send(200, "text/html", "<h1>Credenciales invalidas</h1><p>Ingresa los datos correctamente.</p><a href='/'>Back</a>");
    }
  } else {
    webServer.send(400, "text/html", "error");
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255,255,255,0));
  Serial.println("AP started");
  Serial.println(WiFi.softAPIP());

  dnsServer.start(53, "*", apIP);

  webServer.on("/", PortalCaptivo);
  webServer.on("/login", HTTP_POST, Login);
  webServer.onNotFound(PortalCaptivo);
  webServer.begin();
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}