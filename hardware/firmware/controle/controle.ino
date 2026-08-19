#include <VarSpeedServo.h>
#include <EEPROM.h>

#define espacoMemoria            199
#define tempoPausaEntreMovimentos 500

#define pinServo1 13
#define pinServo2 12
#define pinServo3 11
#define pinServo4 10
#define pinServo5  9
#define pinServo6  8

#define pinBotao1  3
#define pinBotao2  2

#define pinLedA    5
#define pinLedB    4

#define pinPot1   A5
#define pinPot2   A4
#define pinPot3   A3
#define pinPot4   A0
#define pinPot5   A1
#define pinPot6   A2  // knob verde — controla velocidade no modo de reprodução

VarSpeedServo servo1;
VarSpeedServo servo2;
VarSpeedServo servo3;
VarSpeedServo servo4;
VarSpeedServo servo5;
VarSpeedServo servo6;

byte pinBotao1Modo();
bool pinBotao2Retencao();
bool pinBotao2Apertado();
void pinLedAPisca(bool reset = false);
void setMemoria(byte posicao, byte servo, byte valor);
byte readMemoria(byte posicao, byte servo);

int ultMemoria;

void setup() {
  servo1.attach(pinServo1);
  servo2.attach(pinServo2);
  servo3.attach(pinServo3);
  servo4.attach(pinServo4);
  servo5.attach(pinServo5);
  servo6.attach(pinServo6);

  pinMode(pinLedA, OUTPUT);
  pinMode(pinLedB, OUTPUT);
  pinMode(pinBotao1, INPUT_PULLUP);
  pinMode(pinBotao2, INPUT_PULLUP);

  ultMemoria = EEPROM.read(0);
}

void loop() {
  static byte modo = 0;
  static byte modoAnt;
  static byte movimento = 0;
  static byte posMemoria = 0;
  static unsigned long delayPausa;

  // ── Modo Normal (reprodução) ────────────────────────────────────────────────
  if (modo == 0) {
    digitalWrite(pinLedA, LOW);

    if (pinBotao2Retencao()) {
      digitalWrite(pinLedB, HIGH);

      if (movimento == 0) {
        byte velocidade = map(analogRead(pinPot6), 0, 1023, 0, 255);
        servo1.slowmove(readMemoria(posMemoria, 0), velocidade);
        servo2.slowmove(readMemoria(posMemoria, 1), velocidade);
        servo3.slowmove(readMemoria(posMemoria, 2), velocidade);
        servo4.slowmove(readMemoria(posMemoria, 3), velocidade);
        servo5.slowmove(readMemoria(posMemoria, 4), velocidade);
        // servo6 sincronizado inversamente ao servo3
        servo6.slowmove(map(readMemoria(posMemoria, 2), 0, 179, 179, 0), velocidade);
        movimento = 1;
      }

      if (movimento == 1) {
        if ((servo1.read() == readMemoria(posMemoria, 0)) &&
            (servo2.read() == readMemoria(posMemoria, 1)) &&
            (servo3.read() == readMemoria(posMemoria, 2)) &&
            (servo4.read() == readMemoria(posMemoria, 3)) &&
            (servo5.read() == readMemoria(posMemoria, 4))) {
          posMemoria++;
          if (posMemoria > ultMemoria) { posMemoria = 0; }
          delayPausa = millis();
          movimento = 2;
        }
      }

      if (movimento == 2) {
        if ((millis() - delayPausa) > tempoPausaEntreMovimentos) {
          movimento = 0;
        }
      }

    } else {
      digitalWrite(pinLedB, LOW);
      if (pinBotao1Modo() == 2) {
        modo = 1;
      }
    }
  }

  // ── Modo Gravação ───────────────────────────────────────────────────────────
  if (modo == 1) {
    if (modoAnt == 0) {
      pinLedAPisca(true);
      ultMemoria = -1;
      EEPROM.write(0, ultMemoria);
    }

    digitalWrite(pinLedB, LOW);
    pinLedAPisca();

    if (pinBotao1Modo() == 2) {
      modo = 0;
    }

    servo1.write(map(analogRead(pinPot1), 0, 1023, 0, 179));
    servo2.write(map(analogRead(pinPot2), 0, 1023, 0, 179));
    servo3.write(map(analogRead(pinPot3), 0, 1023, 0, 179));
    servo4.write(map(analogRead(pinPot4), 0, 1023, 0, 179));
    servo5.write(map(analogRead(pinPot5), 0, 1023, 0, 179));
    // servo6 sincronizado inversamente ao servo3
    servo6.write(map(analogRead(pinPot3), 0, 1023, 179, 0));

    if (pinBotao2Apertado()) {
      ultMemoria++;
      EEPROM.write(0, ultMemoria);
      setMemoria(ultMemoria, 0, map(analogRead(pinPot1), 0, 1023, 0, 179));
      setMemoria(ultMemoria, 1, map(analogRead(pinPot2), 0, 1023, 0, 179));
      setMemoria(ultMemoria, 2, map(analogRead(pinPot3), 0, 1023, 0, 179));
      setMemoria(ultMemoria, 3, map(analogRead(pinPot4), 0, 1023, 0, 179));
      setMemoria(ultMemoria, 4, map(analogRead(pinPot5), 0, 1023, 0, 179));
      digitalWrite(pinLedB, HIGH);
      delay(250);
      digitalWrite(pinLedB, LOW);

      if (ultMemoria == (espacoMemoria - 1)) {
        modo = 0;
      }
    }
  }

  modoAnt = modo;
}

// ── Botão 1: modo de pressionamento longo (> 1500 ms) ──────────────────────
byte pinBotao1Modo() {
  #define tempoDebounce 40
  #define tempoBotao    1500

  bool estadoBotao;
  static bool estadoBotaoAnt;
  static byte estadoRet = 0;
  static unsigned long delayBotao = 0;
  static byte enviado = 0;

  if ((millis() - delayBotao) > tempoDebounce) {
    estadoBotao = digitalRead(pinBotao1);

    if (estadoRet == 0) {
      if (!estadoBotao && (estadoBotao != estadoBotaoAnt)) {
        estadoRet = 1;
        delayBotao = millis();
      }
    }
    if (estadoRet == 1) {
      if (estadoBotao && (estadoBotao != estadoBotaoAnt)) {
        estadoRet = 0;
        delayBotao = millis();
      }
    }
    estadoBotaoAnt = estadoBotao;
  }

  if (estadoRet == 1) {
    if ((millis() - delayBotao) > tempoBotao) {
      estadoRet = 2;
      delayBotao = millis();
    }
  }

  if (estadoRet == 2) {
    enviado++;
    if (enviado >= 2) {
      estadoRet = 0;
      delayBotao = millis();
      enviado = 0;
    }
  }

  return estadoRet;
}

// ── Botão 2: retenção (toggle a cada pressionamento) ───────────────────────
bool pinBotao2Retencao() {
  #define tempoDebounce 40

  bool estadoBotao;
  static bool estadoBotaoAnt;
  static bool estadoRet = false;
  static unsigned long delayBotao = 0;

  if ((millis() - delayBotao) > tempoDebounce) {
    estadoBotao = digitalRead(pinBotao2);

    if (!estadoBotao && (estadoBotao != estadoBotaoAnt)) {
      estadoRet = !estadoRet;
      delayBotao = millis();
    }
    if (estadoBotao && (estadoBotao != estadoBotaoAnt)) {
      delayBotao = millis();
    }
    estadoBotaoAnt = estadoBotao;
  }

  return estadoRet;
}

// ── Botão 2: detecta borda de descida (one-shot por pressionamento) ─────────
bool pinBotao2Apertado() {
  #define tempoDebounce 40

  bool estadoBotao;
  static bool estadoBotaoAnt;
  static bool estadoRet;
  static unsigned long delayBotao = 0;

  estadoRet = false;

  if ((millis() - delayBotao) > tempoDebounce) {
    estadoBotao = digitalRead(pinBotao2);

    if (!estadoBotao && (estadoBotao != estadoBotaoAnt)) {
      estadoRet = true;
      delayBotao = millis();
    }
    if (estadoBotao && (estadoBotao != estadoBotaoAnt)) {
      delayBotao = millis();
    }
    estadoBotaoAnt = estadoBotao;
  }

  return estadoRet;
}

// ── LED A: pisca com período de 1 s (500 ms LOW / 500 ms HIGH) ─────────────
void pinLedAPisca(bool reset) {
  static unsigned long delayPisca = 0;

  if (reset) {
    delayPisca = millis();
  } else {
    if ((millis() - delayPisca) < 500) {
      digitalWrite(pinLedA, LOW);
    } else {
      digitalWrite(pinLedA, HIGH);
    }
    if ((millis() - delayPisca) >= 1000) {
      delayPisca = millis();
    }
  }
}

// ── EEPROM helpers ──────────────────────────────────────────────────────────
// Layout: byte 0 = índice do último movimento; bytes 1..N = posições (5 bytes cada)
void setMemoria(byte posicao, byte servo, byte valor) {
  int posMem = ((posicao * 5) + servo) + 1;
  EEPROM.write(posMem, valor);
}

byte readMemoria(byte posicao, byte servo) {
  int posMem = ((posicao * 5) + servo) + 1;
  return EEPROM.read(posMem);
}
