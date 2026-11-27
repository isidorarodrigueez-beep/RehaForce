// RehaForce v1.3 (fuerza en Newtons) - Firmware principal (Arduino Nano)
//
//  - Boton (D2): ENCIENDE / APAGA el sistema. Al encender hace tara (cero).
//  - LED (D4): encendido mientras el sistema esta ON.
//  - Buzzer (D7): pitido corto al encender/apagar, y suena mientras
//                 la fuerza llega a la META (umbral).
//  - Pantalla: fuerza actual, maximo, meta y barra de progreso.
//  - Datos por USB en CSV (Serial Plotter / Excel).
//
// Libreria necesaria: U8g2

#include <U8g2lib.h>
#include <Wire.h>

U8G2_SH1106_128X64_NONAME_1_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

const int PIN_FSR1 = A0, PIN_FSR2 = A1;
const int PIN_BTN = 2, PIN_LED = 4, PIN_BUZ = 7;

// ---------- CALIBRACION (reemplazar con tus mediciones) ----------
const int N_CAL = 6;
const int   CAL_CRUDO[N_CAL]  = {0, 100, 250, 450, 700, 1000};
const float CAL_GRAMOS[N_CAL] = {0, 500, 1000, 2000, 3000, 5000};

// ---------- META (en gramos) ----------
const float META_N = 20.0;            // meta en Newtons
const float META = META_N / 9.81 * 1000; // equivalente en gramos (uso interno)

// ---------- MELODIA AL LLEGAR A LA META ----------
// "La Trucha" (Die Forelle) - Schubert, 1817 (dominio publico)
const int MELODIA[] = {
  392, 523, 523, 659, 659, 523, 392,
  392, 440, 494, 523, 587, 523, 0,
  392, 523, 523, 659, 659, 523, 392,
  392, 349, 330, 294, 262
};
const int DURACION[] = {
  200, 300, 150, 300, 150, 300, 400,
  200, 200, 200, 200, 200, 500, 100,
  200, 300, 150, 300, 150, 300, 400,
  200, 200, 200, 200, 700
};
const int N_NOTAS = 26;
bool yaCelebro = false;

void celebrar() {
  for (int i = 0; i < N_NOTAS; i++) {
    if (MELODIA[i] > 0) tone(PIN_BUZ, MELODIA[i], DURACION[i]);
    delay(DURACION[i] + 20);
  }
  noTone(PIN_BUZ);
}

// ---------- Variables ----------
bool encendido = false;
int cero = 0;
float maximo = 0;
bool botonAntes = false;
unsigned long tUltimoCambio = 0;

int leerCrudo() {
  long s = 0;
  for (int i = 0; i < 10; i++) { s += analogRead(PIN_FSR1) + analogRead(PIN_FSR2); delay(1); }
  return s / 10;
}

float aGramos(int crudo) {
  if (crudo <= CAL_CRUDO[0]) return CAL_GRAMOS[0];
  for (int i = 1; i < N_CAL; i++) {
    if (crudo <= CAL_CRUDO[i]) {
      float t = float(crudo - CAL_CRUDO[i-1]) / (CAL_CRUDO[i] - CAL_CRUDO[i-1]);
      return CAL_GRAMOS[i-1] + t * (CAL_GRAMOS[i] - CAL_GRAMOS[i-1]);
    }
  }
  return CAL_GRAMOS[N_CAL - 1];
}

// gramos -> Newtons:  F = m * g = (gramos/1000) * 9.81
float aN(float gramos) { return gramos / 1000.0 * 9.81; }

void beep(int ms) { digitalWrite(PIN_BUZ, HIGH); delay(ms); digitalWrite(PIN_BUZ, LOW); }

void pantallaApagada() {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x12_tr);
    oled.drawStr(20, 30, "RehaForce");
    oled.drawStr(10, 48, "Apagado - pulse");
  } while (oled.nextPage());
}

void revisarBoton() {
  bool apretado = (digitalRead(PIN_BTN) == LOW);
  if (apretado && !botonAntes && millis() - tUltimoCambio > 300) {  // antirrebote
    tUltimoCambio = millis();
    encendido = !encendido;
    if (encendido) {
      digitalWrite(PIN_LED, HIGH);
      beep(120);
      cero = leerCrudo();     // tara al encender
      maximo = 0;
    } else {
      digitalWrite(PIN_LED, LOW);
      beep(60); delay(80); beep(60);
      digitalWrite(PIN_BUZ, LOW);
      pantallaApagada();
    }
  }
  botonAntes = apretado;
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUZ, OUTPUT);
  oled.begin();
  pantallaApagada();
  Serial.println(F("tiempo_ms,crudo,fuerza_N,maximo_N,meta_N"));
}

void loop() {
  revisarBoton();
  if (!encendido) { delay(20); return; }

  int crudo = max(0, leerCrudo() - cero);
  float g = aGramos(crudo);
  if (g > maximo) maximo = g;
  bool alcanzado = (g >= META);

  if (alcanzado && !yaCelebro) { celebrar(); yaCelebro = true; }  // suena 1 vez
  if (g < META * 0.5) yaCelebro = false;                           // al soltar, se rearma

  Serial.print(millis()); Serial.print(',');
  Serial.print(crudo);    Serial.print(',');
  Serial.print(aN(g), 2);      Serial.print(',');
  Serial.print(aN(maximo), 2); Serial.print(',');
  Serial.println(META_N, 1);

  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x12_tr);
    oled.drawStr(0, 10, "RehaForce");
    oled.drawStr(104, 10, "ON");
    oled.setFont(u8g2_font_logisoso16_tr);
    oled.setCursor(0, 34); oled.print(aN(g), 1); oled.print(" N");
    oled.setFont(u8g2_font_6x12_tr);
    oled.setCursor(0, 48); oled.print("Max: "); oled.print(aN(maximo), 1); oled.print(" N");
    oled.setCursor(0, 62); oled.print("Meta: "); oled.print(META_N, 0); oled.print(" N");
    if (alcanzado) oled.drawStr(100, 62, "OK!");
    int ancho = constrain(int(g / META * 40), 0, 40);
    oled.drawFrame(86, 40, 42, 10);
    oled.drawBox(87, 41, ancho, 8);
  } while (oled.nextPage());

  delay(50);
}
