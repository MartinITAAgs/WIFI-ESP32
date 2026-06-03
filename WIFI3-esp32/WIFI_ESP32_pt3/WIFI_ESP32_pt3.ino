// Código para ESP32 S3 - Conexión a MSSQL vía servidor REST
// Librerías necesarias:
// - WiFi (incluida en Arduino IDE para ESP32)
// - HTTPClient (incluida en Arduino IDE para ESP32)
Librerias necesarias para el funcionamiento de la practica
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

Configuracion necesaria para entrar en al red de la base de datos y la ip del dispositivo donde se encuentra el servidor
// ============ CONFIGURACIÓN ============
const char* ssid = "Totalplay-9CA5";
const char* password = "9CA5C414gV4Qs844";
const char* apiUrl = "http://192.168.100.11:3000/api";
const char* testUrl = "http://192.168.100.11:3000/api/test";
const char* insertUrl = "http://192.168.100.11:3000/api/insert";

// Pines de trabajo del ESP32 S3 |Ajusta estos valores según tu proyecto

// Pines de trabajo del ESP32 S3 |Ajusta estos valores según tu proyecto
  
// ============ FUNCIONES LED INTEGRADO============
Al no contar con un LED RGB integrado se opto por hacer un sistema de blinks
#define LED_PIN 2

void setupLED() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Apagado inicial
}
Cuando la conexion esta en curso hace un blink rapido
void blinkFast() { // Conexion en curso
  digitalWrite(LED_PIN, HIGH);
  delay(200);
  digitalWrite(LED_PIN, LOW);
  delay(200);
}
Cuando la conexion es exitosa hace blink mas lento
void blinkSlow() { // Conexion exitosa
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}
Si hay algun error en la conexión el blink es doble y repetido
void blinkError() { // Error de conexion
  for (int i = 0; i < 2; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(150);
    digitalWrite(LED_PIN, LOW);
    delay(150);
  }
  delay(500); // Pausa entre ciclos
}

// ============ FUNCIONES WiFi ============
Funcion que conecta el ESP32 a la red wifi
void connectToWiFi() {
  Serial.println("\n\nInitializando conexión WiFi...");
  Serial.print("Conectando a: ");
  Serial.println(ssid);
  
Configura el Esp32 como un cliente que se conectara al router con los datos del ssid y contraseña definidos arriba
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
Funcion que cuenta los intentos para conectarse a la red wifi. 
Se tiene un delay de 500 ms entre cada intento a conectarse. Este bucle terminara cuando se conecte a la red wifi o hasta que hayan pasado 20 intentos de conectarse.
  int attemp = 0;
  while (WiFi.status() != WL_CONNECTED && attemp < 20) {
    delay(500);
    Serial.print(".");
    attemp++;
    blinkFast();
  }
  

Si la conexion es exitosa pasara a dar un aviso y cambiara el blink a lento.
de lo contrario avisara del error y parpadeara doble el led
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ Conectado a WiFi");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    blinkSlow();
  } else {
    Serial.println("\n✗ Error: No se pudo conectar a WiFi");
    blinkError();
  }
}

// ============ FUNCIONES HTTP ============

bool testDatabaseConnection() {
mensaje serial avisando de la conexión a la base de datos
  Serial.println("\nProbando conexión a MSSQL...");
  
Se hace un objeto http y se prueba la url almacenada en testUrl. Se especifica con el Content-Type que se trabajaran con datos de tipo json.
  HTTPClient http;
  http.begin(testUrl);
  http.addHeader("Content-Type", "application/json");
  


Se solicita una peticion Get al servidor y se guardara en la variable httpCode que almacenara códigos de la solicitud
  int httpCode = http.GET();
  
Analiza la respuesta de la solicitud GET y da mensajes de la respuesta del servidor
  if (httpCode == 200) {
    String response = http.getString();
    Serial.println("✓ Conexión a BD exitosa");
    Serial.println(response);
    http.end();
    return true;
  } else {
    Serial.print("✗ Error conectando a BD: ");
    Serial.println(httpCode);
    http.end();
    return false;
  }
}

Esta function es la encargada de hacer los inserts a la base de datos con los valores de nombre, valor y temperatura.
bool insertDataToDatabase(const char* nombre, int valor, float temperatura) {
  Serial.println("\nEnviando INSERT a MSSQL...");
  
 
Se crea un cliente http con la url almacenada en insertUrl e igualmente se especifica que se usaran datos json
  HTTPClient http;
  http.begin(insertUrl);
  http.addHeader("Content-Type", "application/json");
  
Se crea un documento json con los campos que se insertaran en la base de datos
  // Crear JSON con los datos
  DynamicJsonDocument doc(200);
  doc["nombre"] = nombre;
  doc["valor"] = valor;
  doc["temperatura"] = temperatura;
  
Esta function convierte los datos json en un string para poder ser enviados por http
  String jsonString;
  serializeJson(doc, jsonString);
  
  Serial.print("Datos enviados: ");
  Serial.println(jsonString);
  
Envia los datos al servidor con una petición POST
  int httpCode = http.POST(jsonString);
  
Codigos de respuesta que avisan del insert correcto o un error
  if (httpCode == 200) {
    String response = http.getString();
    Serial.println("✓ INSERT exitoso");
    Serial.println(response);
    http.end();
    return true;
  } else {
    Serial.print("✗ Error en INSERT: ");
    Serial.println(httpCode);
    String response = http.getString();
    Serial.println(response);
    http.end();
    return false;
  }
}

// ============ SETUP ============

void setup() {
  
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=== ESP32 S3 MSSQL DEBUG ===");
 



  Inicia la function del led integrado para avisar del estado.
  setupLED();
  
  // Conectar a WiFi
Usa la funcion para conectar a wifi. Si no se puede conectar a wifi avisa del error y vuelve a intentar la conexion
  connectToWiFi();
  
  if (WiFi.status() != WL_CONNECTED) {
    blinkError();
    Serial.println("No se pudo conectar a WiFi. Reiniciando en 5 segundos...");
    delay(5000);
    ESP.restart();
  }
  
Una vez hecha la conexión, procede a conectarse a la base de datos mediante un bucle con limite de 10 intentos cada 500ms.
  Serial.println("\nProbando conexión a base de datos...");
  bool connectionOk = false;
  
  for (int i = 0; i < 10; i++) {
    blinkFast();
    Si la conexion es correcta se rompe el bucle
    if (testDatabaseConnection()) {
      connectionOk = true;
      break;
    }
    delay(500);
  }
  Si la conexion no es correcta avisa del error y reinicia el esp32
  if (!connectionOk) {
    blinkError();
    Serial.println("No se pudo conectar a la BD. Verifique la configuración.");
    delay(5000);
    ESP.restart();
  }
  
Se intenta hacer el insert a la base de datos
  // Hacer un INSERT de prueba
  Serial.println("\nRealizando INSERT de prueba...");
  
Se manda a llamar la funcion insertDataToDatabase con unos valores predeterminados. El estado de la inserción es dado a conocer en la terminal serial
  if (insertDataToDatabase("ESP32_S3_Test", 42, 23.5)) {
    blinkSlow();
    Serial.println("\n✓✓✓ TODO CORRECTO - LED VERDE ✓✓✓");
  } else {
    blinkError();
    Serial.println("\n✗✗✗ ERROR EN INSERT - LED ROJO ✗✗✗");
  }
}

// ============ LOOP ============

void loop() {
  // Mantener el LED en el estado actual
  
  // Cada 30 segundos, hacer otro INSERT para demostración
Una vez hecho el insert predeterminado, se realizara un insert cada 30 segundos
  static unsigned long lastInsert = 0;
  if (millis() - lastInsert > 30000) {
    lastInsert = millis();
    
    Serial.println("\n\n=== Enviando INSERT periódico ===");

    Se crea un valor aleatorio para las variables que se enviaran: valor y temperatura
    int valorRandom = random(0, 100);
    float temperaturaRandom = 20.0 + (random(0, 50) / 10.0);
    
Se Vuelve a llamar a la funcion para insertar los datos a la base de datos con el nombre predeterminado, y las variables que acaban de generarse de manera aleatoria. Posteriormente da el estatus de la inserción mediante la terminal Serial
    if (insertDataToDatabase("ESP32_Periodic", valorRandom, temperaturaRandom)) {
      blinkFast();
      Serial.println("✓ INSERT periódico exitoso");
      

    } else {
      blinkError();
      Serial.println("✗ INSERT periódico fallido");
     
    }
  }
  
  delay(100);
}
