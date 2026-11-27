// RehaForce - PRUEBA POR BLOQUES (Arduino Nano)
// Carga esto primero para revisar que cada parte de la placa funciona.
// Abre el Monitor Serie a 115200 baudios.
//
// Conexiones de la PCB:
//   FSR1 -> A0 | FSR2 -> A1 | OLED SDA -> A4, SCK -> A5
//   Boton -> D2 (a GND) | LED -> D4 | Buzzer -> D7
// Libreria necesaria: U8g2 (Herramientas > Administrar bibliotecas > "U8g2")

#include <U8g2lib.h>
#include <Wire.h>

U8G2_SH1106_128X64_NONAME_1_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

const int PIN_FSR1 = A0, PIN_FSR2 = A1;
const int PIN_BTN = 2, PIN_LED = 4, PIN_BUZ = 7;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUZ, OUTPUT);

  Serial.println(F("== Prueba RehaForce =="));

  // 1) LED y buzzer
  Serial.println(F("1) LED y buzzer: deben encender 3 veces"));
  for (int i = 0; i < 3; i++) {
    digitalWrite(PIN_LED, HIGH); digitalWrite(PIN_BUZ, HIGH); delay(200);
    digitalWrite(PIN_LED, LOW);  digitalWrite(PIN_BUZ, LOW);  delay(200);
  }

  // 2) Pantalla
  Serial.println(F("2) Pantalla: debe decir 'Hola RehaForce'"));
  oled.begin();
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x12_tr);
    oled.drawStr(0, 20, "Hola RehaForce");
    oled.drawStr(0, 40, "Pantalla OK");
  } while (oled.nextPage());

  Serial.println(F("3) Aprieta los FSR y el boton. Se muestran abajo:"));
}

void loop() {
  int f1 = analogRead(PIN_FSR1);
  int f2 = analogRead(PIN_FSR2);
  bool boton = (digitalRead(PIN_BTN) == LOW);

  // El LED se enciende mientras aprietas el boton
  digitalWrite(PIN_LED, boton);

  Serial.print(F("FSR1:")); Serial.print(f1);
  Serial.print(F(",FSR2:")); Serial.print(f2);
  Serial.print(F(",Boton:")); Serial.println(boton ? 500 : 0);

  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x12_tr);
    oled.drawStr(0, 12, "Prueba de bloques");
    oled.setCursor(0, 30); oled.print("FSR1: "); oled.print(f1);
    oled.setCursor(0, 44); oled.print("FSR2: "); oled.print(f2);
    oled.setCursor(0, 58); oled.print("Boton: "); oled.print(boton ? "APRETADO" : "-");
  } while (oled.nextPage());

  delay(100);
}
