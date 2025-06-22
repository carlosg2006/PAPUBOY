// PAPUBOY 
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
const int buzzer1 = 10;
const int buzzer2 = 9;
const int boton1 = 12;
const int boton2 = 11;
const int joyX = A1;
const int joyY = A0;
byte naveChar[8] = {
  B00100,
  B01110,
  B11111,
  B11111,
  B11111,
  B01110,
  B10101,
  B00100
};

byte enemigoChar[8] = {
  B11111,
  B10101,
  B11111,
  B11111,
  B01110,
  B00100,
  B00100,
  B01010
};

byte escudoChar[8] = {
  B01110,
  B10001,
  B10101,
  B10001,
  B10001,
  B10101,
  B10001,
  B01110
};

int naveY = 0;
int puntaje = 0;
int record = 0;
bool jugando = false;
bool mostrarAnimacion = true;
unsigned long ultimoMovimiento = 0;
int enemigos[2] = {16, 20};

int escudos = 3;
bool escudoActivo = false;
unsigned long tiempoEscudo = 0;

void melodiaMenu() {
  static unsigned long ultimaNota = 0;
  static int notaActual = 0;
  static int notas[] = {440, 392, 440, 349};
  static int duraciones[] = {300, 300, 300, 400};
  static int totalNotas = sizeof(notas) / sizeof(notas[0]);

  if (millis() - ultimaNota > duraciones[notaActual]) {
    tone(buzzer2, notas[notaActual]);
    ultimaNota = millis();
    notaActual = (notaActual + 1) % totalNotas;
  }
}

void reproducirMelodia(int pin, int *notas, int *duraciones, int tamanio) {
  for (int i = 0; i < tamanio; i++) {
    tone(pin, notas[i], duraciones[i]);
    delay(duraciones[i] * 1.3);
  }
}

void melodiaInicio() {
  int notas[] = {262, 330, 392, 523};
  int duraciones[] = {200, 200, 200, 400};
  reproducirMelodia(buzzer1, notas, duraciones, 4);
}

void melodiaJuego() {
  tone(buzzer1, 200);
  delay(50);
  noTone(buzzer1);
}

void sonidoEscudo() {
  int notas[] = {880, 660};
  int duraciones[] = {100, 200};
  reproducirMelodia(buzzer2, notas, duraciones, 2);
}

void sonidoPerder() {
  int notas[] = {300, 200, 150};
  int duraciones[] = {200, 200, 400};
  reproducirMelodia(buzzer1, notas, duraciones, 3);
}

void setup() {
  pinMode(boton1, INPUT_PULLUP);
  pinMode(boton2, INPUT_PULLUP);
  pinMode(buzzer1, OUTPUT);
  pinMode(buzzer2, OUTPUT);
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, naveChar);
  lcd.createChar(1, enemigoChar);
  lcd.createChar(2, escudoChar);
  Serial.begin(9600);
  record = EEPROM.read(0);
  if (mostrarAnimacion) {
    animacionInicio();
    mostrarAnimacion = false;
  }
  pantallaInicio();
}

void loop() {
  if (!jugando) {
    melodiaMenu();
    if (digitalRead(boton1) == LOW) {
      delay(200);
      noTone(buzzer2);
      iniciarJuego();
    }
  } else {
    jugar();
  }
}

void animacionInicio() {
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("PAPUBOY");
  melodiaInicio();
  delay(1000);
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    lcd.print("=");
    delay(50);
  }
  delay(800);
}

void pantallaInicio() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SkyEscape");
  lcd.setCursor(0, 1);
  lcd.print("pulse para jugar");
}

void iniciarJuego() {
  jugando = true;
  puntaje = 0;
  naveY = 0;
  enemigos[0] = 16;
  enemigos[1] = 20;
  escudos = 3;
  escudoActivo = false;
  lcd.clear();
  melodiaJuego();
}

void jugar() {
  if (digitalRead(boton2) == LOW && escudos > 0 && !escudoActivo) {
    escudoActivo = true;
    tiempoEscudo = millis();
    escudos--;
    puntaje = max(0, puntaje - 2);
    sonidoEscudo();
  }

  if (escudoActivo && millis() - tiempoEscudo > 2000) {
    escudoActivo = false;
  }

  if (millis() - ultimoMovimiento >= 200) {
    ultimoMovimiento = millis();

    int yVal = analogRead(joyY);
    if (yVal < 400 && naveY <1) naveY++; 
    else if (yVal > 600 && naveY >0) naveY--; 
    moverEnemigos();
    detectarColision();
    mostrarJuego();
  }
}

void moverEnemigos() {
  for (int i = 0; i < 2; i++) {
    enemigos[i]--;
    if (enemigos[i] == 0 && naveY != i) puntaje++;
    if (enemigos[i] < 0) enemigos[i] = random(16, 25);
  }
}

void detectarColision() {
  for (int i = 0; i < 2; i++) {
    if (enemigos[i] == 1 && naveY == i && !escudoActivo) {
      sonidoPerder();
      perder();
      return;
    }
  }
}

void mostrarJuego() {
  lcd.clear();
  for (int i = 0; i < 2; i++) {
    lcd.setCursor(0, i);
    if (i == naveY) {
      if (escudoActivo) lcd.write(2);
      else lcd.write(0);
    } else lcd.print(" ");
  }
  for (int i = 0; i < 2; i++) {
    if (enemigos[i] >= 0 && enemigos[i] < 16) {
      lcd.setCursor(enemigos[i], i);
      lcd.write(1);
    }
  }
  lcd.setCursor(5, 0);
  lcd.print("P:");
  lcd.print(puntaje);
  lcd.setCursor(11, 0);
  lcd.print("S:");
  lcd.print(escudos);
}

void perder() {
  jugando = false;
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("PERDISTE");
  lcd.setCursor(0, 1);
  lcd.print("P:");
  lcd.print(puntaje);
  lcd.print(" R:");
  if (puntaje > record) {
    record = puntaje;
    EEPROM.write(0, record);
  }
  lcd.print(record);
  delay(5000);
  pantallaInicio();
}

