# Braço Robótico — PETEE UFMG

Projeto de braço robótico desenvolvido originalmente na disciplina de **Laboratório de Circuitos Elétricos 1** e doado ao grupo **PETEE (Programa de Educação Tutorial da Engenharia Elétrica) da UFMG**. Utilizado em atividades de extensão como a **Engenharia na Escola (ENE)** e a **Mostra de Profissões da UFMG** para demonstrar conceitos básicos de robótica e programação.

Inspirado no projeto [FoamArmDS da EasyDS](https://www.youtube.com/watch?v=vVOyWQZ25M8).

---

## Visão Geral da Arquitetura

O sistema é composto por dois elementos que se comunicam sem fio:

```
┌─────────────────────────────┐          ┌──────────────────────────────────┐
│         CELULAR (App)       │          │          BRAÇO ROBÓTICO          │
│                             │          │                                  │
│  Giroscópio + Acelerômetro  │          │  ESP32                           │
│  Slider de garra            │──────────│    ├── Servo 1 (Base - MG996R)   │
│  Botão: Sincronizar         │  Wi-Fi   │    ├── Servo 2 (Ombro - MG996R)  │
│  Botão: Gravar Posição      │  (UDP)   │    ├── Servo 3 (Cotovelo - SG92R)│
│  Botão: Reproduzir          │          │    ├── Servo 4 (Punho V - SG92R) │
│                             │          │    ├── Servo 5 (Punho R - SG92R) │
│  ► Todos os cálculos de     │          │    └── Servo 6 (Garra - SG92R)   │
│    posição e interpolação   │          │                                  │
│    são feitos aqui e        │          │  ► Executa as instruções         │
│    repassados ao ESP32      │          │    recebidas                     │
└─────────────────────────────┘          └──────────────────────────────────┘
```

**Princípio central:** O celular é o "cérebro" — ele lê os sensores, calcula as posições e envia apenas as instruções finais ao ESP32. O ESP32 atua como um executor, movendo os servos conforme os comandos recebidos. Isso simplifica o firmware e permite que toda a lógica de controle seja atualizada pelo app, sem regravar o hardware. A comunicação via **UDP** foi escolhida para garantir baixíssima latência e compatibilidade perfeita com atualizações OTA (Over-The-Air) do ESP32.

---

## Funcionalidades

### Modo Sincronia
- Ao ativar, o app passa a ler o **giroscópio e acelerômetro** do celular continuamente.
- O app mapeia o deslocamento angular do celular (roll, pitch, yaw) para os ângulos dos servos correspondentes do braço.
- Os ângulos calculados são enviados em **fluxo contínuo (UDP)** ao ESP32, que os executa em tempo real — o braço espelha o movimento do celular.
- Um **slider** na tela controla independentemente o ângulo de fechamento da garra (Servo 6).

### Modo Gravação
- Com o braço em qualquer posição (inclusive em sincronia), o usuário pressiona **"Gravar Posição"** no app.
- O app captura o estado atual dos 6 ângulos e os armazena localmente em uma lista ordenada.
- Podem ser gravadas múltiplas posições em sequência, formando uma "coreografia".

### Modo Reprodução
- O app percorre a lista de posições gravadas em loop.
- A **transição entre uma posição e a próxima é calculada pelo app** (interpolação), que envia o fluxo de ângulos intermediários ao ESP32 — garantindo movimentos suaves, sem solavcos.
- O ESP32 executa cada instrução recebida sem precisar conhecer a lógica de transição.

---

## Estrutura do Repositório

```
Braco-Robotico-PETEE/
│
├── hardware/                          # Projeto físico embarcado
│   ├── firmware/                      # Projeto PlatformIO (ESP32)
│   │   └── src/
│   │       └── main.cpp               # Firmware principal
│   ├── electronics/                   # KiCad: esquemáticos e PCB
│   │   ├── symbols/                   # Símbolos customizados
│   │   ├── footprints/                # Footprints customizados
│   │   └── datasheets/                # PDFs dos datasheets dos componentes
│   └── mechanical/                    # Modelagem 3D e impressão
│       ├── freecad/                   # Arquivos fonte .FCStd
│       └── stl/                       # Arquivos exportados para impressão
│
├── app/                               # Aplicativo mobile de controle (em desenvolvimento)
│
├── docs/                              # Documentação e relatórios
│   └── Relatorio_BracoRobotico.pdf    # Relatório da versão original
│
└── README.md
```

---

## Hardware

### Componentes

| Quantidade | Componente | Observação |
|---|---|---|
| 4× | Servo Motor SG92R 180° | Para as juntas mais leves (Cotovelo, Punhos e Garra) |
| 2× | Servo Motor MG996R 180° | Para as juntas pesadas (Base e Ombro), garantindo alto torque e estabilidade |
| 1× | ESP32 (30 pinos) | Microcontrolador principal (Wi-Fi) |
| 1× | Bateria LiPo 2S 7.4V 2200mAh 30C | Alta capacidade de descarga (66A contínuos), elimina risco de queda de tensão (brownout) |
| 1× | Módulo Buck Converter XL4015 | Rebaixador step-down de 5A |
| — | PCB customizada (KiCad) | Esquemático e layout em `hardware/electronics/` |

### Mapeamento de Pinos — ESP32

Foram escolhidos apenas pinos "100% seguros" (que não interferem no boot do ESP32) e agrupados fisicamente no mesmo lado da placa para facilitar o roteamento da PCB no KiCad.

| Pino ESP32 | Função |
|---|---|
| `GPIO 13` | Servo 1 — Base (MG996R) |
| `GPIO 14` | Servo 2 — Ombro (MG996R) |
| `GPIO 25` | Servo 3 — Cotovelo (SG92R) |
| `GPIO 26` | Servo 4 — Punho Vertical (SG92R) |
| `GPIO 27` | Servo 5 — Punho Rotacional (SG92R) |
| `GPIO 33` | Servo 6 — Garra (SG92R) |

### Alimentação (Arquitetura Híbrida)

O sistema pode exigir picos de corrente de até **8 Amperes** se todos os servos sofrerem carga simultaneamente. Para não sobrecarregar o Buck Converter de 5A (XL4015), a alimentação foi dividida na PCB:
1. **Os 2× MG996R** são alimentados **diretamente** pela bateria LiPo 2S (7.4V nominal), operando no seu torque máximo.
2. **O Buck Converter XL4015** rebaixa os 7.4V para **5V** para alimentar apenas o **ESP32** (pelo pino VIN) e os **4× SG92R**.

---

## Mecânica

Modelagem 3D desenvolvida no **FreeCAD**. As peças são projetadas para impressão em **PLA ou PETG**.

### Referências para Modelagem

| Parâmetro | Valor |
|---|---|
| Dimensões do SG92R | 22,8 × 12,5 × 22,7 mm (Eixo Ø 4,7 mm) |
| Dimensões do MG996R | 40,7 × 19,7 × 42,9 mm |
| Tolerância recomendada nos encaixes | 0,2 mm |
| Espessura mínima das paredes | 2 mm nas juntas |
| Fixação | Parafusos M2/M3 |

### Arquivos
- Fonte: `hardware/mechanical/freecad/`
- Exportados para impressão: `hardware/mechanical/stl/`

---

## Firmware

O firmware é desenvolvido com **PlatformIO** (VS Code) para o **ESP32**.
O firmware tem responsabilidade única: **receber pacotes UDP de ângulos via Wi-Fi e mover os servos correspondentes**.

### Dependências

| Biblioteca | Uso |
|---|---|
| `ESP32Servo` | Controle de servos via canais PWM por hardware do ESP32 |
| `WiFi.h` & `WiFiUdp.h` | Conexão de rede e recepção de pacotes (AP ou Cliente) |

### Formato do Pacote Recebido (Proposta Inicial)

Exemplo de pacote UDP (String):
```
S1:090,S2:045,S3:120,S4:090,S5:000,S6:030\n
```
Onde cada valor é o ângulo alvo (0–180°) do servo correspondente.

### Como Compilar e Gravar

1. Instale o [VS Code](https://code.visualstudio.com/) com a extensão **PlatformIO**.
2. Abra a pasta `hardware/firmware/` no VS Code.
3. Conecte o ESP32 via USB.
4. Clique em **Upload** (ícone de seta) na barra inferior do PlatformIO.

---

## App — Interface de Controle

O aplicativo mobile é o "cérebro" do sistema. Como é considerado um projeto de natureza transitória (focado na execução rápida e prova de conceito), a plataforma escolhida prioriza o desenvolvimento rápido e simples (ex: **React Native com Expo** ou até ferramentas low-code para prototipagem rápida).

Responsabilidades do App:
- Ler o **giroscópio e acelerômetro** do celular.
- Calcular **interpolações de movimento** entre posições gravadas.
- Enviar as instruções via **Wi-Fi (UDP)** ao ESP32.
- Gerenciar a lista de posições gravadas localmente.

---

## Pontos de Atenção para Manutenção

| Item | Descrição |
|---|---|
| **Alimentação** | Verificar carga da bateria antes de apresentações. Tensão insuficiente causa comportamento imprevisível nos servos (brownout no ESP32). A LiPo não deve baixar de 3.3V por célula. |
| **Estrutura física** | Inspecionar encaixes das peças 3D e fixação dos servos antes de cada uso. |
| **Servos** | Calibrar a posição zero de cada servo (com um Servo Tester) antes de montar as peças mecânicas. Um servo descalibrado pode forçar e quebrar as peças impressas. |

---

## Equipe

**Projeto original:** Lorran Pires Venetillo Dutra, Michael Cassemiro Oliveira, Vítor Gabriel Reis Caitité, Willian Braga da Silva.

**Versão atual (PETEE):** Hugo e equipe do PETEE.

Mantido pelo grupo **PETEE — UFMG** | [petee.cpdee.ufmg.br](http://www.petee.cpdee.ufmg.br/)
