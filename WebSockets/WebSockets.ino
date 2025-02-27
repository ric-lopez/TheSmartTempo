/*
  Universidad Nacional Autónoma de México
  Facultad de Ciencias
  Licenciatura en Ciencias de la Computación
  Proyecto PAPIME: PE1O7O24

    SMART TIMER

  Última modificación: 14-ENERO-2024
*/

#include "WiFi.h"
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include <Ticker.h>

//Para desactivar el watchdog
#include "esp_task_wdt.h"
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"


//Objeto AsyncWebServer, puerto 80-------------
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");


// Hardware Serial 2 pins
#define RXD2 16
#define TXD2 17

int gate[3] = { 33, 32, 35 };

bool stateC1 = false;
bool stateC2 = false;
bool stateC3 = false;
volatile long timeC1 = 0;
volatile long timeC2 = 0;
volatile long timeC3 = 0;
volatile double timeDiff = 0;

//Variables para mejorar el funcionamiento del botón
long timeCounter1 = 0;
long timeCounter2 = 0;
long timeCounter3 = 0;
int timeThreshold = 150;

// Variable para almacenar el tiempo medido
volatile double currentTime = 0;
// Indica si la medición está en progreso
volatile bool measurementInProgress = false;

unsigned long lastWebSocketUpdate = 0;
const unsigned long webSocketInterval = 5000;  // 5 segundos

// Variable para almacenar el tiempo (ejemplo en segundos)
unsigned long tiempoMedido = 0;


String mensaje;
String tiempoMedido2;
String resultado;

// Función que se ejecuta cuando se recibe un evento WebSocket
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("Cliente conectado - ID: %u\n", client->id());
      client->text("id2: Conexión establecida");
      break;

    case WS_EVT_DISCONNECT:
      Serial.printf("id2: Cliente desconectado - ID: %u\n", client->id());
      break;

    case WS_EVT_DATA:
      {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;

        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
          String command = String((char *)data).substring(0, len);
          Serial.printf("Comando recibido: %s\n", command.c_str());

          // Delegar el manejo del comando a la función
          handleWebSocketMessage(client, command);
        }
        break;
      }

    case WS_EVT_PONG:
      Serial.println("PONG recibido del cliente");
      break;

    case WS_EVT_ERROR:
      Serial.printf("Error en WebSocket - Cliente ID: %u\n", client->id());
      break;
  }
}

// Función que enviará el mensaje
void sendCalculating() {
  ws.textAll("id2: Conexión establecida");
  Serial.println("Mensaje 'calculando' enviado");
}


void setup_wifi_ap() {
  const char *ssid_ap = "SmartTempo_06";  // Nombre del AP
  const char *password_ap = "12345678";   // Contraseña del AP (mínimo 8 caracteres)

  // Conectar a Wi-Fi
  WiFi.softAP(ssid_ap, password_ap);

  // Imprimir la dirección IP del AP
  Serial.println("ESP32 configurado como Access Point");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void disableCoreWatchdogs() {
  // Desactiva el watchdog del sistema en el grupo 0
  TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;  // Desbloquea el acceso al registro
  TIMERG0.wdt_config0.en = 0;                  // Desactiva el watchdog
  TIMERG0.wdt_wprotect = 0;                    // Bloquea el acceso al registro

  // Desactiva el watchdog del sistema en el grupo 1
  TIMERG1.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
  TIMERG1.wdt_config0.en = 0;
  TIMERG1.wdt_wprotect = 0;
}


//Acciones a realizar--------------------------
String m12() {
  double tiempo = measureTime("m12");
  return String(tiempo);
}

void sensorTask(void *parameter) {
  String val = *((String *)parameter);  // Convertir el parámetro a String
  double result = measureTimeP(val);    // Llama a tu función de medición
  String tiempo = String(result);
  Serial.println(result);
  ws.textAll("id1:" + tiempo + " segundos");  // Enviar el resultado
  vTaskDelete(NULL);                          // Eliminar la tarea después de completarla
  if (val == "reset") {
    ws.textAll("id1: 0:00");
    stateC1 = false;
    stateC2 = false;
    stateC3 = false;
  }
}

void handleWebSocketMessage(AsyncWebSocketClient *client, String command) {
  if (command == "m12" || command == "m13" || command == "m23") {
    client->text("id1: Medición iniciada para " + command);
    xTaskCreate(sensorTask, "SensorTask", 4096, new String(command), 1, NULL);
  }
  if (command == "reset") {
    client->text("id1: 0:00");
  } else {
    client->text("Comando no reconocido");
  }
}

void setup() {
  Serial.begin(9600);  // Inicializa la comunicación serie a 9600 baudios
  delay(100);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  delay(100);
  pinMode(2, OUTPUT);
  pinMode(gate[0], INPUT);
  pinMode(gate[1], INPUT);
  pinMode(gate[2], INPUT);

  attachInterrupt(digitalPinToInterrupt(gate[0]), int1, FALLING);  //Interrupción 1
  attachInterrupt(digitalPinToInterrupt(gate[1]), int2, FALLING);  //Interrupción 2
  attachInterrupt(digitalPinToInterrupt(gate[2]), int3, FALLING);  //Interrupción 2


  ///***///Serial.begin(9600);
  esp_task_wdt_deinit();
  Serial.println("I");
  Serial2.println("I");
  disableCoreWatchdogs();

  setup_wifi_ap();


  // Iniciar el servidor web
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Servir archivos estáticos
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", String(), false);
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });
  server.on("/scriptP.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/scriptP.js", "application/javascript");
  });

  //Función para error 404
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Error 404");
  });
}



void loop() {
  // Código principal
  if (Serial2.available()) {
    // Lee los datos recibidos del Arduino Uno
    String mensaje = Serial2.readStringUntil('\n');
    mensaje.trim();
    Serial.print("Received data: ");
    Serial.println(mensaje);

    if (mensaje.equals("b")) {
      setup_wifi_ap();
      if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
      }
      ws.onEvent(onWebSocketEvent);
      server.addHandler(&ws);
      server.begin();
      Serial.println(mensaje);
    }
    if (mensaje.equals("r")) {
      double tiempo = 0;
      Serial2.println(tiempo);
      Serial.print("r mensaje  enviado: ");
      Serial.println(mensaje);
      stateC1 = false;
      stateC2 = false;
      stateC3 = false;
    }
    if (mensaje.equals("m12") || mensaje.equals("m13") || mensaje.equals("m23")) {
      Serial.println(mensaje);
      double tiempo = measureTime(mensaje);
      Serial2.println(tiempo);
      Serial.print("m12m13m23 mensaje enviado: ");
      Serial.println(tiempo);
    }
  }
}


void int1() {
  if ((millis() > timeCounter1 + timeThreshold)) {
    timeC1 = micros();
    stateC1 = true;
    timeCounter1 = millis();
  }
}

void int2() {
  if ((millis() > timeCounter2 + timeThreshold)) {
    timeC2 = micros();
    stateC2 = true;
    timeCounter2 = millis();
  }
}

void int3() {
  if ((millis() > timeCounter3 + timeThreshold)) {
    timeC3 = micros();
    stateC3 = true;
    timeCounter3 = millis();
  }
}

/*
 * Lee el monitor serial cuando se recibe un mensaje de reset.
 * Reinicia las banderas que permiten la lectura de los sensores.
 */
void readReset(){
  String mensaje = Serial2.readStringUntil('\n');
  Serial.print("readReset mensaje recibido: ");
  Serial.println(mensaje);
  stateC1 = false;
  stateC2 = false;
  stateC3 = false;
}


/*
  Función que calcula el tiempo que tarda entre dos sensores
*/
double measureTime(String val) {
  bool isReset = false; // Bandera que indica si se recibe un mensaje de reset
  if (val == "m12") {
    while (stateC1 == false && isReset == false) {
      if(Serial2.available()){ // Verifica si se recibe un mensaje de reset mientras se espera la lectura de los sensores
          readReset();
          isReset = true; // Se activa la bandera de mensaje reset recibido
          break;
      }else{
        Serial.println("A1");
      }
    }
    while (stateC2 == false && isReset == false) {
      if(Serial2.available()){
          readReset();
          isReset = true;
          break;
      }else{
        Serial.println("A2");
      }
    }
    if(isReset){ // Valida si se recibio mensaje de reset
      timeDiff = 0; 
      isReset = false; // Cambia la bandera para restablecer las lecturas de los sensores 
    }else{
      timeDiff = abs(timeC2 - timeC1) / 1000000.0;
    }
    stateC1 = false;
    stateC2 = false;
    stateC3 = false;
    return timeDiff;
    
  } else if (val == "m13") {
    while (stateC1 == false && isReset == false) {
      if(Serial2.available()){
          readReset();
          isReset = true;
          break;
      }else{
        Serial.println("B1");
      }
    }
    while (stateC3 == false && isReset == false) {
      if(Serial2.available()){
          readReset();
          isReset = true;
          break;
      }else{
        Serial.println("B3");
      }
    }
    if(isReset){
      timeDiff = 0;
      isReset = false;
    }else{
      timeDiff = abs(timeC3 - timeC1) / 1000000.0;
    }
    stateC1 = false;
    stateC2 = false;
    stateC3 = false;
    return timeDiff;
  } else if (val == "m23" && isReset == false) {
    while (stateC2 == false) {
      if(Serial2.available()){
          readReset();
          isReset = true;
          break;
      }else{
        Serial.println("C2");
      }
    }
    while (stateC3 == false && isReset == false) {
      if(Serial2.available()){
          readReset();
          isReset = true;
          break;
      }else{
        Serial.println("C3");
      }
    }
    if(isReset){
      timeDiff = 0;
      isReset = false;
    }else{
      timeDiff = abs(timeC3 - timeC2) / 1000000.0;
    }
    stateC1 = false;
    stateC2 = false;
    stateC3 = false;
    return timeDiff;
  } else {
    stateC1 = false;
    stateC2 = false;
    stateC3 = false;
    timeDiff = 0;
    return timeDiff;
  }
}

double measureTimeP(String val) {
  if (val == "m12") {
    unsigned long startTime = millis();  // Tiempo inicial
    unsigned long timeout = 300000;      // Tiempo máximo en milisegundos (30 segundos)
    // Reinicia los estados y tiempos de los sensores
    stateC1 = stateC2 = stateC3 = false;
    timeC1 = timeC2 = timeC3 = 0;

    // Esperar a que ambos sensores estén listos
    while (!(stateC1 && stateC2)) {
      if (stateC1 == false) {
        Serial.println("Esperando a stateC1...");
      }
      if (stateC2 == false) {
        Serial.println("Esperando a stateC2...");
      }

      delay(10);  // Pausa para evitar bloquear completamente la CPU
      if (millis() - lastWebSocketUpdate > webSocketInterval) {
        ws.textAll("id2: Conectado");
        lastWebSocketUpdate = millis();
      }
      // Verificar timeout
      if (millis() - startTime > timeout) {
        Serial.println("Timeout alcanzado esperando a los sensores.");
        return -1;  // Retornar un valor especial en caso de timeout
      }
    }

    // Calcular la diferencia de tiempo si ambos sensores respondieron a tiempo
    timeDiff = abs(timeC2 - timeC1) / 1000000.0;
    stateC1 = stateC2 = stateC3 = false;

    return timeDiff;
  } else if (val == "m13") {
    unsigned long startTime = millis();  // Tiempo inicial
    unsigned long timeout = 300000;      // Tiempo máximo en milisegundos (30 segundos)
    // Reinicia los estados y tiempos de los sensores
    stateC1 = stateC2 = stateC3 = false;
    timeC1 = timeC2 = timeC3 = 0;

    // Esperar a que ambos sensores estén listos
    while (!(stateC1 && stateC3)) {
      if (stateC1 == false) {
        Serial.println("Esperando a stateC1...");
      }
      if (stateC3 == false) {
        Serial.println("Esperando a stateC3...");
      }
      delay(10);  // Pausa para evitar bloquear completamente la CPU

      // Verificar timeout
      if (millis() - startTime > timeout) {
        Serial.println("Timeout alcanzado esperando a los sensores.");
        return -1;  // Retornar un valor especial en caso de timeout
      }
    }

    // Calcular la diferencia de tiempo si ambos sensores respondieron a tiempo
    timeDiff = abs(timeC3 - timeC1) / 1000000.0;
    stateC1 = stateC2 = stateC3 = false;

    return timeDiff;
  } else if (val == "m23") {
    // Reinicia los estados y tiempos de los sensores
    stateC1 = stateC2 = stateC3 = false;
    timeC1 = timeC2 = timeC3 = 0;

    unsigned long startTime = millis();  // Tiempo inicial
    unsigned long timeout = 300000;      // Tiempo máximo en milisegundos (30 segundos)

    // Esperar a que ambos sensores estén listos
    while (!(stateC2 && stateC3)) {
      if (stateC2 == false) {
        Serial.println("Esperando a stateC2...");
      }
      if (stateC3 == false) {
        Serial.println("Esperando a stateC3...");
      }

      delay(10);  // Pausa para evitar bloquear completamente la CPU

      // Verificar timeout
      if (millis() - startTime > timeout) {
        Serial.println("Timeout alcanzado esperando a los sensores.");
        return -1;  // Retornar un valor especial en caso de timeout
      }
    }

    // Calcular la diferencia de tiempo si ambos sensores respondieron a tiempo
    timeDiff = abs(timeC3 - timeC2) / 1000000.0;
    stateC1 = stateC2 = stateC3 = false;

    return timeDiff;
  } else {
    stateC1 = stateC2 = stateC3 = false;
    timeDiff = 0;
    return timeDiff;
  }
}
