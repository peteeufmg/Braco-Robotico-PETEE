# Braço Robótico — PETEE UFMG

TESTE

Projeto de braço robótico desenvolvido originalmente na disciplina de **Laboratório de Circuitos Elétricos 1** e doado ao grupo **PETEE (Programa de Educação Tutorial da Engenharia Elétrica) da UFMG**. Utilizado em atividades de extensão como a **Engenharia na Escola (ENE)** e a **Mostra de Profissões da UFMG** para demonstrar conceitos básicos de robótica e programação.

Inspirado no projeto [FoamArmDS da EasyDS](https://www.youtube.com/watch?v=vVOyWQZ25M8).

---

## Estrutura do Repositório

```
Braco-Robotico-PETEE/
├── BackEnd/
│   └── firmware/
│       └── controle/
│           └── controle.ino   # Firmware Arduino (código principal)
├── FrontEnd/                  # Interface de controle (a definir)
└── README.md
```

---

## Hardware

### Componentes

| Quantidade | Componente |
|---|---|
| 6x | Micro Servo 9g |
| 6x | Potenciômetro Linear Mini 1K |
| 2x | Chave Tátil 12×12×4,3 mm (4 terminais) |
| 2x | Chave Tátil 12×12×12 mm (4 terminais) |
| 5x | LED Vermelho Difuso 3mm |
| 5x | LED Amarelo Difuso 3mm |
| 5x | LED Verde Difuso 3mm |
| 1x | Arduino Uno |
| 2x | Placa CI Fenolite Virgem 10×20 |
| 1x | Jack DC-002 |
| 1x | Jack DC-005 2,1mm × 5,5mm (para placa) |
| 1x | Jack DC-005 2,5mm × 5,5mm (para placa) |
| 1x | Suporte para 4 pilhas |
| 4x | Pilha 1,2 V |
| — | Jumpers macho/fêmea |
| — | Spumapaper (estrutura física) |
| 1x | Knob AD-110 Azul (eixo estriado) |
| 1x | Knob AD-110 Verde (eixo estriado) |
| 1x | Knob AD-110 Branco (eixo estriado) |

### Mapeamento de Pinos — Arduino Uno

| Pino | Função |
|---|---|
| D13 | Servo 1 |
| D12 | Servo 2 |
| D11 | Servo 3 |
| D10 | Servo 4 |
| D9  | Servo 5 |
| D8  | Servo 6 (sincronizado inversamente ao Servo 3) |
| D5  | LED A (vermelho — pisca no modo gravação) |
| D4  | LED B (amarelo — acende no modo reprodução) |
| D3  | Botão 1 (INPUT_PULLUP) |
| D2  | Botão 2 (INPUT_PULLUP) |
| A5  | Potenciômetro 1 |
| A4  | Potenciômetro 2 |
| A3  | Potenciômetro 3 |
| A0  | Potenciômetro 4 |
| A1  | Potenciômetro 5 |
| A2  | Potenciômetro 6 (knob verde — velocidade) |

**Alimentação:** 5 V contínuos (4 pilhas de 1,2 V em série).

---

## Firmware — BackEnd

### Dependências (Arduino IDE)

- [VarSpeedServo](https://github.com/netlabtoolkit/VarSpeedServo) — controle de servo com velocidade variável
- `EEPROM.h` — biblioteca nativa do Arduino

Instale `VarSpeedServo` pelo **Gerenciador de Bibliotecas** do Arduino IDE ou manualmente na pasta `libraries/`.

### Como compilar e gravar

1. Abra `BackEnd/firmware/controle/controle.ino` no Arduino IDE.
2. Selecione a placa **Arduino Uno** em *Ferramentas > Placa*.
3. Selecione a porta COM correta em *Ferramentas > Porta*.
4. Clique em **Carregar** (Upload).

---

## Modos de Operação

### Modo Manual (padrão na inicialização)

- Cada potenciômetro controla diretamente o servo correspondente.
- LED A apagado, LED B apagado.
- Para entrar no **Modo Gravação**: pressione e segure o **Botão 1** por mais de 1,5 s.

### Modo Gravação

- LED A pisca (período 1 s) enquanto o modo está ativo.
- Os potenciômetros ainda movem os servos em tempo real.
- Para **gravar uma posição**: pressione rapidamente o **Botão 2** — o LED B piscará brevemente confirmando.
- Capacidade: até **199 posições** armazenadas na EEPROM.
- Para sair do modo gravação: pressione e segure o **Botão 1** por mais de 1,5 s.

### Modo Reprodução

- Ativado pelo **Botão 2** (toggle — pressione para ligar/desligar).
- O braço executa em loop as posições gravadas.
- O **knob verde** (Pot6 / A2) controla a velocidade de reprodução.
- LED B aceso enquanto reproduz.

---

## EEPROM — Layout de Memória

```
Byte 0          → índice do último movimento gravado
Bytes 1 a 995   → posições dos servos (5 bytes por posição: servos 1–5)
```

Cada posição armazena o ângulo (0–179°) dos servos 1 a 5. O Servo 6 é calculado em tempo real como o inverso do Servo 3.

---

## Pontos de Atenção para Manutenção

| Item | Descrição |
|---|---|
| **Alimentação** | Manter exatamente 5 V. Pilhas descarregadas causam comportamento imprevisível nos servos. Verificar carga antes das apresentações. |
| **Estrutura física** | As peças de spumapaper e o encaixe dos servos são frágeis. Inspecionar antes de cada uso e reforçar com cola se necessário. |
| **EEPROM** | A EEPROM do Arduino Uno suporta ~100.000 ciclos de escrita por endereço. Evitar gravações excessivas desnecessárias. |
| **Servo 6** | Sincronizado *inversamente* ao Servo 3 por software — não há potenciômetro dedicado. |

---

## FrontEnd (A Definir)

Interface de controle a ser desenvolvida. Possibilidades:
- Aplicativo mobile (Bluetooth/Wi-Fi)
- Interface web (com módulo ESP8266/ESP32)
- Controle por joystick via serial

---

## Melhorias Planejadas

> **Status atual:** desmontado — remontagem necessária antes de qualquer teste de firmware.

| # | Melhoria | Situação Atual | Proposta | Impacto | Observações |
|---|---|---|---|---|---|
| 1 | **Controle por IMU** | 6 potenciômetros físicos controlam os servos manualmente | Substituir por módulo **MPU-6050** (acelerômetro + giroscópio 6 DOF via I²C), espelhando movimentos do operador em tempo real | Firmware: trocar `analogRead(pinPotX)` por leitura I²C (`Wire.h` + `MPU6050.h`); mapear roll/pitch/yaw para ângulos 0–179° de cada servo | I²C usa SDA/A4 e SCL/A5 — conflito com Pot1 e Pot2 atuais; requer remapeamento de pinos e resistores pull-up 4,7 kΩ |
| 2 | **Estrutura em Impressão 3D** | Peças em spumapaper — frágeis, encaixes imprecisos, servos se soltam | Modelar e imprimir peças em PLA/PETG compatíveis com os servos 9g existentes | Estrutura mais robusta e reproduzível; fixação por parafusos M2/M3 em vez de cola | Tolerância 0,2 mm nos encaixes; paredes ≥ 2 mm nas articulações; servo 9g: 22,8 × 12,5 × 22,7 mm, eixo Ø 4,7 mm; salvar `.stl` em `docs/3d-models/` |
| 3 | **Remontagem** | Braço desmontado | Remontar seguindo checklist de validação por etapas | Necessária antes de qualquer teste de firmware ou demonstração | Ver checklist abaixo |

### Checklist de Remontagem

- [ ] Testar cada servo individualmente com sketch básico (`Servo.write(90)`)
- [ ] Verificar estado da placa de circuito impresso (trilhas, soldas frias)
- [ ] Confirmar alimentação estável de 5 V antes de conectar os servos
- [ ] Gravar o firmware `controle.ino` no Arduino Uno antes de fixar na estrutura
- [ ] Calibrar posição zero de cada servo antes de montar as peças mecânicas
- [ ] Testar modo manual → modo gravação → modo reprodução após montagem completa

---

## Equipe

Projeto original: Lorran Pires Venetillo Dutra, Michael Cassemiro Oliveira, Vítor Gabriel Reis Caitité, Willian Braga da Silva.

Mantido pelo grupo **PETEE — UFMG** | [petee.cpdee.ufmg.br](http://www.petee.cpdee.ufmg.br/)
