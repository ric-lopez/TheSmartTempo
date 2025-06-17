#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include <TouchScreen.h>
#define MINPRESSURE 200
#define MAXPRESSURE 1000

#define YP A1
#define XM A2
#define YM 7
#define XP 6

#define TS_LEFT 169
#define TS_BOT 181
#define TS_RT 935
#define TS_TOP 986

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

bool botonesBloqueados = false;  // Controla si los botones están bloqueados


//Arreglo en el que se declaran toods los botones que se usan en la interfaz
Adafruit_GFX_Button bt_btn, manual_btn, menu_btn, m12_btn, m13_btn, m23_btn, m123_btn, reset_btn;

String inData = "INIT";  // Estado inicial
int pixel_x, pixel_y;    // Touch_getXY() actualiza variables globales
/*Para la conexión bluetooth*/
bool isBluetoothConnected = false;  // Para manejar el estado de la conexión Bluetooth

/*sEPARAMOS LOS */
Adafruit_GFX_Button *initial_buttons[] = { &bt_btn, &manual_btn, NULL };
Adafruit_GFX_Button *buttons[] = { &m12_btn, &m13_btn, &m23_btn, &m123_btn, &reset_btn, NULL };
Adafruit_GFX_Button *menu[] = { &menu_btn, NULL };

bool Touch_getXY(void) {
  TSPoint p = ts.getPoint();
  pinMode(YP, OUTPUT);  // restore shared pins
  pinMode(XM, OUTPUT);
  digitalWrite(YP, HIGH);  // porque los pines de control TFT
  digitalWrite(XM, HIGH);
  bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
  if (pressed) {
    pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width());
    pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
  }
  return pressed;
}

// Se definen los colores EN R
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x14FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define AMARILL 0x64962
#define VERDE 0x6DC1
#define AZUL 0x0C77
void setup(void) {

  Serial.begin(9600);
  delay(500);
  uint16_t ID = tft.readID();
  if (ID == 0xD3D3) ID = 0x9486;
  tft.begin(ID);
  tft.setRotation(0);  // PORTRAIT
  tft.fillScreen(BLACK);
  showInitialMenu();
}

void loop() {
  if (Serial.available() != 0) {
    inData = Serial.readStringUntil('\n');
    Serial.print("Received data: ");  // Para ver que es lo que se recibe
    Serial.println(inData);
  }
  if (inData.charAt(0) < 48  || inData.charAt(0) > 90) {
    inData = "INIT";
  }
  //Menú para poder interactuar con los botones.

  switch (inData.charAt(0)) {
    case 'I':                               // Caso con la pantalla inicial
      update_button_list(initial_buttons);  //Actualiza los botones "Bluetooth" y "Manual" para activarlos

      //Condición para cuando apretamos el botón wifi
      if (bt_btn.justPressed()) {
        isBluetoothConnected = true;
        fondoBluetooth();
        Serial.println("b");
        inData = "B";
      } else if (manual_btn.justPressed()) {  //Cuando presionamos el botón Manual
        showMeasurementButtons();             // Pinta la pantalla manual
        inData = "R";
      }
      break;

    case 'B':                    // Caso Wifi
      update_button_list(menu);  //use helper function
      if (menu_btn.justPressed()) {
        Serial.println("c"); //se manda señal de reinicio para bloquear las lecturas de los sensores
        tft.fillScreen(BLACK);  //Pinta la pantalla de negro
        showInitialMenu();      // Función para mostrar el menú inicial
        inData = "I";
      }

      break;

    case 'R': //Caso Manualpara la medición
      update_button_list(menu);
      update_button_list(buttons);
      /*
        Para evitar que se presionen un botón antes de presionar reset usaremos una bandera
        Si los botones no estan bloqueados podras acceder a cualquiera
        Pero al presionar uno de los botones en automático vamos a cambiar la bandera para bloquearlo

      */
      if (!botonesBloqueados) {
        if (m12_btn.justPressed()) {
          Serial.println("m12");
          tft.fillRect(15, 42, 210, 65, BLACK);
          m12_btn.initButton(&tft, 60, 145, 100, 50, WHITE, GREEN, BLACK, "1-2", 2);
          inData = "T";
          botonesBloqueados = true; //Cambiar la bandera
        } else if (m13_btn.justPressed()) {
          Serial.println("m13");
          tft.fillRect(15, 42, 210, 65, BLACK);
          m13_btn.initButton(&tft, 60, 216, 100, 50, WHITE, GREEN, BLACK, "1-3", 2);
          inData = "D";
          botonesBloqueados = true;  // Cambiar la bandera
        } else if (m23_btn.justPressed()) {
          Serial.println("m23");
          tft.fillRect(15, 42, 210, 65, BLACK);
          m23_btn.initButton(&tft, 180, 145, 100, 50, WHITE, GREEN, BLACK, "2-3", 2);
          inData = "E";
          botonesBloqueados = true;  // Cambiar la bandera
        } else if (m123_btn.justPressed()) {
          Serial.println("m123");
          tft.fillRect(15, 42, 210, 65, BLACK);
          m123_btn.initButton(&tft, 180, 216, 100, 50, WHITE, GREEN, BLACK, "1-2-3", 2);
          inData = "F";
          botonesBloqueados = true;  // Cambiar la bandera
        }
      }

      if (reset_btn.justPressed()) {
        tft.fillRect(15, 42, 210, 65, BLACK);
        m12_btn.initButton(&tft, 60, 145, 100, 50, WHITE, CYAN, BLACK, "1-2", 2);
        m13_btn.initButton(&tft, 60, 216, 100, 50, WHITE, CYAN, BLACK, "1-3", 2);
        m23_btn.initButton(&tft, 180, 145, 100, 50, WHITE, CYAN, BLACK, "2-3", 2);
        m123_btn.initButton(&tft, 180, 216, 100, 50, WHITE, CYAN, BLACK, "1-2-3", 2);
        Serial.println("r");
        inData = "R";
        m12_btn.drawButton(false);
        m13_btn.drawButton(false);
        m23_btn.drawButton(false);
        m123_btn.drawButton(false);
        reset_btn.drawButton(false);
        menu_btn.drawButton(false);
        botonesBloqueados = false;  // Desbloquear botones
      }

      if (menu_btn.justPressed()) {
        tft.fillScreen(BLACK);
        showInitialMenu();
        inData = "I";
        botonesBloqueados = false;  // Asegurar desbloqueo al regresar al menú
      }
      break;

    case 'T':
      update_button_list(buttons);

      if ((inData == "T" || inData == "D" || inData == "E" || inData == "F") && Serial.available() > 0) {
        String tiempo = Serial.readStringUntil('\n');  // Leer tiempo del sensor
        textTime(tiempo);  // Mostrar tiempo en la pantalla
      }

      if (reset_btn.justPressed()) {
        tft.fillRect(15, 42, 210, 65, BLACK);
        m12_btn.initButton(&tft, 60, 145, 100, 50, WHITE, CYAN, BLACK, "1-2", 2);
        m13_btn.initButton(&tft, 60, 216, 100, 50, WHITE, CYAN, BLACK, "1-3", 2);
        m23_btn.initButton(&tft, 180, 145, 100, 50, WHITE, CYAN, BLACK, "2-3", 2);
        m123_btn.initButton(&tft, 180, 216, 100, 50, WHITE, CYAN, BLACK, "1-2-3", 2);
        Serial.println("r");
        inData = "R";
        m12_btn.drawButton(false);
        m13_btn.drawButton(false);
        m23_btn.drawButton(false);
        m123_btn.drawButton(false);
        reset_btn.drawButton(false);
        menu_btn.drawButton(false);
        botonesBloqueados = false;
      }
      break;
    case 'D':
      update_button_list(buttons);

      if ((inData == "T" || inData == "D" || inData == "E" || inData == "F") && Serial.available() > 0) {
        String tiempo = Serial.readStringUntil('\n');  // Leer tiempo del sensor
        Serial.println(tiempo);
        textTime(tiempo);  // Mostrar tiempo en la pantalla
      }

      if (reset_btn.justPressed()) {
        tft.fillRect(15, 42, 210, 65, BLACK);
        m12_btn.initButton(&tft, 60, 145, 100, 50, WHITE, CYAN, BLACK, "1-2", 2);
        m13_btn.initButton(&tft, 60, 216, 100, 50, WHITE, CYAN, BLACK, "1-3", 2);
        m23_btn.initButton(&tft, 180, 145, 100, 50, WHITE, CYAN, BLACK, "2-3", 2);
        m123_btn.initButton(&tft, 180, 216, 100, 50, WHITE, CYAN, BLACK, "1-2-3", 2);
        Serial.println("r");
        inData = "R";
        m12_btn.drawButton(false);
        m13_btn.drawButton(false);
        m23_btn.drawButton(false);
        m123_btn.drawButton(false);
        reset_btn.drawButton(false);
        menu_btn.drawButton(false);
        botonesBloqueados = false;
      }
      break;
    case 'E':
      update_button_list(buttons);

      if ((inData == "T" || inData == "D" || inData == "E" || inData == "F") && Serial.available() > 0) {
        String tiempo = Serial.readStringUntil('\n');  // Leer tiempo del sensor
        tiempo.trim();
        textTime(tiempo);  // Mostrar tiempo en la pantalla
      }

      if (reset_btn.justPressed()) {  // Solo se puede salir con Reset
        tft.fillRect(15, 42, 210, 65, BLACK);
        m12_btn.initButton(&tft, 60, 145, 100, 50, WHITE, CYAN, BLACK, "1-2", 2);
        m13_btn.initButton(&tft, 60, 216, 100, 50, WHITE, CYAN, BLACK, "1-3", 2);
        m23_btn.initButton(&tft, 180, 145, 100, 50, WHITE, CYAN, BLACK, "2-3", 2);
        m123_btn.initButton(&tft, 180, 216, 100, 50, WHITE, CYAN, BLACK, "1-2-3", 2);
        Serial.println("r");
        m12_btn.drawButton(false);
        m13_btn.drawButton(false);
        m23_btn.drawButton(false);
        m123_btn.drawButton(false);
        reset_btn.drawButton(false);
        menu_btn.drawButton(false);
        inData = "R";  // Volver a estado de selección
        botonesBloqueados = false;
      }

      break;
    case 'F':
      update_button_list(buttons);

      if ((inData == "T" || inData == "D" || inData == "E" || inData == "F") && Serial.available() > 0) {
        String tiempo = Serial.readStringUntil('\n');  // Leer tiempo del sensor
        Serial.println(tiempo);
        Serial.println(tiempo.substring(1, 2));
        if (tiempo.substring(1, 2).equals(",")) {
          textTime2(tiempo);  // Mostrar tiempo en la pantalla
          
        }else{
        textTime(tiempo);  // Mostrar tiempo en la pantalla
        }
      }

      if (reset_btn.justPressed()) {
        tft.fillRect(15, 42, 210, 65, BLACK);
        m12_btn.initButton(&tft, 60, 145, 100, 50, WHITE, CYAN, BLACK, "1-2", 2);
        m13_btn.initButton(&tft, 60, 216, 100, 50, WHITE, CYAN, BLACK, "1-3", 2);
        m23_btn.initButton(&tft, 180, 145, 100, 50, WHITE, CYAN, BLACK, "2-3", 2);
        m123_btn.initButton(&tft, 180, 216, 100, 50, WHITE, CYAN, BLACK, "1-2-3", 2);
        Serial.println("r");
        inData = "R";
        m12_btn.drawButton(false);
        m13_btn.drawButton(false);
        m23_btn.drawButton(false);
        m123_btn.drawButton(false);
        reset_btn.drawButton(false);
        menu_btn.drawButton(false);
        botonesBloqueados = false;
      }
      break;

    default:  //Imprime el tiempo medido en el TFT
      textTime(inData);
      inData = "R";
      break;
  }
}


/* Se crea la pantalla de medición de tiempos y dibuja los botones de la pantalla*/
void showMeasurementButtons() {
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.setCursor(5, 10);
  tft.print("Medir tiempos");

  //buttonSetup(); // Configurar y dibujar botones de medición
  m12_btn.initButton(&tft, 60, 145, 100, 50, WHITE, CYAN, BLACK, "1-2", 2);
  m13_btn.initButton(&tft, 60, 216, 100, 50, WHITE, CYAN, BLACK, "1-3", 2);
  m23_btn.initButton(&tft, 180, 145, 100, 50, WHITE, CYAN, BLACK, "2-3", 2);
  m123_btn.initButton(&tft, 180, 216, 100, 50, WHITE, CYAN, BLACK, "1-2-3", 2);
  reset_btn.initButton(&tft, 180, 287, 100, 50, WHITE, RED, BLACK, "Reset", 2);
  menu_btn.initButton(&tft, 60, 287, 100, 50, WHITE, WHITE, BLACK, "Menu", 2);

  m12_btn.drawButton(false);
  m13_btn.drawButton(false);
  m23_btn.drawButton(false);
  m123_btn.drawButton(false);
  reset_btn.drawButton(false);
  menu_btn.drawButton(false);
}

/* Se dibuja la pantalla de Bluetooth y el botón menú */
void fondoBluetooth() {
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.setCursor(30, 20);
  tft.print("Conectate a");
  tft.setCursor(70, 55);
  tft.setTextColor(WHITE);
  tft.print("ssid:");
  tft.setCursor(14, 85);
  tft.setTextColor(RED);
  tft.print("SmartTempo_00");
  tft.setCursor(30, 130);
  tft.setTextColor(WHITE);
  tft.print("Passsword:");
  tft.setCursor(45, 160);
  tft.setTextColor(RED);
  tft.print("12345678");
  tft.setCursor(90, 200);
  tft.setTextColor(WHITE);
  tft.print("IP");
  tft.setCursor(25, 225);
  tft.setTextColor(RED);
  tft.print("192.168.4.1");

  menu_btn.initButton(&tft, 120, 295, 100, 30, WHITE, CYAN, BLACK, "Menu", 2);
  menu_btn.drawButton(false);
}

/* Se muestra el menú inicial */
void showInitialMenu() {
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.setCursor(30, 20);
  tft.print("Selecciona");
  tft.setCursor(95, 50);
  tft.print("modo");

  //bt_btn.initButton(&tft, 120, 140, 200, 60, WHITE, CYAN, BLACK, "Bluetooth", 3);
  bt_btn.initButton(&tft, 120, 140, 200, 60, WHITE, CYAN, BLACK, "Wifi", 3);
  manual_btn.initButton(&tft, 120, 240, 200, 60, WHITE, CYAN, BLACK, "Manual", 3);

  bt_btn.drawButton(false);
  manual_btn.drawButton(false);
}

/* Actualiza el estado de un botón y lo redibuja según sea necesario */
bool update_button(Adafruit_GFX_Button *b, bool down) {
  b->press(down && b->contains(pixel_x, pixel_y));
  if (b->justReleased())
    b->drawButton(false);
  if (b->justPressed())
    b->drawButton(true);
  return down;
}

/* La mayoría de las pantallas tienen diferentes conjuntos de botones */
bool update_button_list(Adafruit_GFX_Button **pb) {
  bool down = Touch_getXY();
  for (int i = 0; pb[i] != NULL; i++) {
    update_button(pb[i], down);
  }
  return down;
}

/* Desactivar todos los botones del arreglo buttons*/
void disableAllButtons() {
  for (int i = 0; buttons[i] != NULL; i++) {
    buttons[i]->press(false);
    buttons[i]->drawButton(true);
  }
}

/* Mostrar el tiempo en pantalla*/
void textTime(String val) {
  Serial.println(val);
  val.trim();
  if (val.equals("ERROR")) {
    tft.fillRect(30, 80, 200, 30, BLACK);
    tft.setTextColor(WHITE);
    tft.setTextSize(2.5);
    tft.setCursor(20, 65);
    tft.print("ERROR DE MEDICION");
  } else {
    tft.fillRect(30, 80, 600, 40, BLACK);
    tft.setTextColor(WHITE);
    tft.setTextSize(2.5);
    tft.setCursor(60, 65);
    tft.print("t = " + val + " s.");
  }
}

void textTime2(String values) {
  values.trim();
  String direccion = values.substring(0, 1) ;
  Serial.println(direccion);
  String t1 = values.substring(2, 6);
  String t2 = values.substring(7, 11);
  String t3 = values.substring(12);

  if (direccion.equals("1") ) {
    tft.fillRect(30, 80, 200, 30, BLACK);
    tft.setTextColor(WHITE);
    tft.setTextSize(2.5);
    tft.setCursor(60, 43);
    tft.print("1a2 = " + t1 + " s.");
    tft.setCursor(60, 65);
    tft.print("2a3 = " + t2 + " s.");
    tft.setCursor(60, 88);
    tft.print("1a3 = " + t3 + " s.");
  } else if (direccion.equals("2") ) {
    tft.fillRect(30, 80, 200, 30, BLACK);
    tft.setTextColor(WHITE);
    tft.setTextSize(2.5);
    tft.setCursor(60, 43);
    tft.print("3a2 = " + t1 + " s.");
    tft.setCursor(60, 65);
    tft.print("2a1 = " + t2 + " s.");
    tft.setCursor(60, 88);
    tft.print("3a1 = " + t3 + " s.");

  }
}
