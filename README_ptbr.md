<p align="center">
  <img src="extras/gingoduino.png" alt="Gingoduino" width="480">
</p>

# 🪇 Gingo[duino]

**Motor de Teoria Musical para Sistemas Embarcados**

<p align="center">
  <a href="https://github.com/sauloverissimo/gingoduino/actions/workflows/ci.yml"><img src="https://github.com/sauloverissimo/gingoduino/actions/workflows/ci.yml/badge.svg" alt="Native Tests"></a>
  <a href="https://www.arduino.cc/reference/en/libraries/gingoduino/"><img src="https://img.shields.io/badge/Arduino-compatible-00878F?logo=arduino&logoColor=white" alt="Arduino"></a>
  <a href="https://registry.platformio.org/libraries/sauloverissimo/Gingoduino"><img src="https://img.shields.io/badge/PlatformIO-compatible-FF7F00?logo=platformio&logoColor=white" alt="PlatformIO"></a>
  <a href="https://github.com/sauloverissimo/gingoduino"><img src="https://img.shields.io/badge/ESP--IDF-compatible-E7352C?logo=espressif&logoColor=white" alt="ESP-IDF"></a>
  <a href="https://github.com/sauloverissimo/gingoduino/blob/main/src/GingoMIDI2.h"><img src="https://img.shields.io/badge/MIDI_2.0-UMP_Flex_Data-8A2BE2" alt="MIDI 2.0"></a>
  <a href="https://github.com/sauloverissimo/gingoduino/blob/main/LICENSE"><img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="License: MIT"></a>
</p>

<p align="center">
  <a href="https://github.com/sponsors/sauloverissimo"><img src="https://img.shields.io/badge/Sponsor-❤-ea4aaa?style=flat-square&logo=github-sponsors&logoColor=white" alt="Sponsor" /></a>
</p>

> English version: [README.md](README.md)

---

## Visão geral

Gingoduino é um motor de teoria musical para sistemas embarcados. Ele cobre o domínio musical (notas, intervalos, acordes, escalas, campos harmônicos, árvores harmônicas, análise de progressão, engine de braço), um monitor harmônico em tempo real, e adaptadores de saída sem estado que traduzem estruturas musicais em formatos serializados (bytes MIDI 1.0 e UMP Flex Data MIDI 2.0).

Não cobre o fio. Parsing de byte stream, dispatch de UMP recebido, e descoberta MIDI-CI com Property Exchange são responsabilidades de protocolo delegadas a bibliotecas dedicadas: `midi2_cpp` para UMP, Arduino MIDI Library ou parser próprio para byte stream MIDI 1.0, biblioteca [`midi2`](https://github.com/sauloverissimo/midi2) C99 para responders MIDI-CI.

Port da [biblioteca gingopy C++17](https://github.com/sauloverissimo/gingopy). Arquitetura zero-heap, tabelas em PROGMEM, compatível com C++11.

> **0.4.0 é uma release com breaking changes.** Veja [CHANGELOG.md](CHANGELOG.md) para o guia de migração se você está atualizando da 0.3.x.

## Arquitetura

```
┌─────────────────┐    evento musical     ┌──────────────────┐
│   Transporte    │ ──────────────────▶   │   GingoMonitor   │
│  (midi2_cpp,    │ noteOn / noteOff /    │  acorde, campo,  │
│   Arduino MIDI, │ sustainOn / Off       │  contexto/nota   │
│   ESP32_Host_   │                       └────────┬─────────┘
│   MIDI, ...)    │                                │
└─────────────────┘                                │
        ▲                                          ▼
        │            ┌─────────────────────────────────┐
        │            │    Teoria + Análise              │
        │            │  Note · Interval · Chord ·       │
        │            │  Scale · Field · Tree ·          │
        │            │  Progression · Comparison ·      │
        │            │  Fretboard · Event · Sequence    │
        │            └─────────────────┬────────────────┘
        │                              │
        │                              ▼
        │            ┌─────────────────────────────────┐
        │            │     Adaptadores de saída         │
        │  bytes /   │  GingoMIDI1::fromEvent           │
        └────────────┤  GingoMIDI1::fromSequence        │
            UMP      │  GingoMIDI2::chordName           │
                     │  GingoMIDI2::keySignature        │
                     │  GingoMIDI2::perNoteController   │
                     └─────────────────────────────────┘
```

O transporte vive fora da biblioteca. Ele decodifica bytes ou pacotes UMP e alimenta o Monitor pela API de eventos musicais. Tudo o que Gingoduino calcula pode ser convertido de volta em bytes ou UMP através dos adaptadores de saída em namespaces dedicados.

## Módulos

| Tier | Módulos | Plataformas |
|------|---------|-------------|
| 1 | Note, Interval, Chord | AVR (Uno, Nano), best-effort, sem CI |
| 2 | + Scale, Field, Duration, Tempo, TimeSig, Fretboard, NoteContext, Monitor, adaptadores MIDI1 | ESP8266 |
| 3 | + Event, Sequence, Tree, Progression, ChordComparison, adaptadores MIDI2 | ESP32, RP2040, Teensy, Daisy Seed |

Os tiers são selecionados automaticamente pela plataforma. Para forçar um tier, defina `GINGODUINO_TIER N` antes de incluir `Gingoduino.h`.

## Características

- Sistema cromático de 12 notas com equivalentes enarmônicos
- 42 fórmulas de acordes com lookup reverso (identify)
- Mais de 40 tipos de escalas e modos com armadura, brilho, escala relativa e paralela
- Análise de campo harmônico com funções T/S/D e roles, mais dedução a partir de notas e acordes
- Árvore harmônica (grafo dirigido, maior e menor, tradições clássica e jazz)
- Análise de progressão: identify, deduce (ranqueado), predict (próximo branch)
- Engine de braço: guitar, violão, cavaquinho, mandolim/bandolim, ukulele; afinações alternativas (Drop D, Open G, DADGAD); acordes comuns e digitações em primeira posição
- Eventos musicais (nota, acorde, pausa) e sequências com tempo e fórmula de compasso
- Monitor harmônico em tempo real com detecção de acordes e campos e contexto por nota
- Adaptadores de saída MIDI 1.0: `GingoMIDI1::fromEvent`, `GingoMIDI1::fromSequence`
- Adaptadores de saída MIDI 2.0 UMP Flex Data: `GingoMIDI2::chordName`, `keySignature`, `perNoteController`
- Comparação de acordes em 17 dimensões, incluindo transformações Neo-Riemannianas e vetores Forte
- Arrays de tamanho fixo, sem alocação dinâmica, suporte a PROGMEM
- Compatível com Arduino IDE, PlatformIO e ESP-IDF
- 399 testes nativos passando com `-Wall -Wextra -Werror`

## Instalação

**Arduino IDE Library Manager:**
- Sketch > Include Library > Manage Libraries > buscar `Gingoduino` > Install

**PlatformIO:**
```ini
; platformio.ini
lib_deps = sauloverissimo/Gingoduino
```

**ESP-IDF Component:**
```bash
idf.py add-dependency "sauloverissimo/gingoduino"
```

**Manual:**
- Baixe e copie para sua pasta de bibliotecas Arduino (`~/Arduino/libraries/`).

## Uso rápido

```cpp
#include <Gingoduino.h>

using namespace gingoduino;

void setup() {
    Serial.begin(9600);

    GingoNote nota("C");
    Serial.println(nota.name());           // "C"
    Serial.println(nota.midiNumber(4));    // 60
    Serial.println(nota.frequency(4), 1);  // 261.6

    GingoChord acorde("Dm7");
    GingoNote notas[7];
    acorde.notes(notas, 7);                // D, F, A, C

    GingoScale escala("C", SCALE_MAJOR);
    GingoField campo("C", SCALE_MAJOR);
    GingoChord triades[7];
    campo.chords(triades, 7);              // CM, Dm, Em, FM, GM, Am, Bdim
}

void loop() {}
```

## Referência da API

### GingoNote
```cpp
GingoNote note("C#");
note.name();              // "C#"
note.natural();           // "C#" (forma canônica em sustenidos: Bb -> A#, Eb -> D#)
note.semitone();          // 1 (0-11)
note.frequency(4);        // Hz (float)
note.midiNumber(4);       // 0-127
note.transpose(7);        // GingoNote
note.distance(other);     // distância mínima no ciclo de quintas (0-6)
note.isEnharmonic(other); // bool
GingoNote::fromMIDI(60);  // "C"
GingoNote::octaveFromMIDI(60); // 4
```

### GingoInterval
```cpp
GingoInterval iv("5J");          // ou GingoInterval(7) ou GingoInterval(noteA, noteB)
char buf[32];
iv.label(buf, sizeof(buf));      // "5J"
iv.semitones();                  // 7
iv.degree();                     // 5
iv.consonance(buf, sizeof(buf)); // "perfect", "imperfect" ou "dissonant"
iv.fullName(buf, sizeof(buf));   // "Perfect Fifth"
iv.fullNamePt(buf, sizeof(buf)); // "Quinta Justa"
iv.simple();                     // reduz composto para simples
iv.invert();                     // complemento dentro da oitava
```

### GingoChord
```cpp
GingoChord chord("Dm7");
chord.name();                          // "Dm7"
chord.root();                          // GingoNote("D")
chord.type();                          // "m7"
chord.size();                          // 4

GingoNote notes[7];
chord.notes(notes, 7);

GingoNote arr[3] = {GingoNote("C"), GingoNote("E"), GingoNote("G")};
char name[16];
GingoChord::identify(arr, 3, name, 16); // "CM"
```

### GingoScale
```cpp
GingoScale scale("C", SCALE_MAJOR);    // ou GingoScale("C", "dorian")
char buf[22];
scale.modeName(buf, sizeof(buf));      // "Ionian"
scale.quality();                       // "major" ou "minor"
scale.signature();                     // 0 (sustenidos > 0, bemóis < 0)
scale.brightness();                    // 1-7 (maior = mais brilhante)

GingoNote notes[12];
scale.notes(notes, 12);
scale.mode(2);                         // Dorian
scale.pentatonic();
scale.relative();                      // relativa maior/menor
scale.parallel();                      // paralela maior/menor
```

### GingoField
```cpp
GingoField field("C", SCALE_MAJOR);
GingoChord triads[7];  field.chords(triads, 7);    // CM, Dm, Em, FM, GM, Am, Bdim
GingoChord sevs[7];    field.sevenths(sevs, 7);    // C7M, Dm7, Em7, F7M, G7, Am7, Bm7(b5)

field.function(5);                     // FUNC_DOMINANT
field.functionOf(GingoChord("GM"));    // FUNC_DOMINANT
char buf[12];
field.role(1, buf, sizeof(buf));       // "primary"

GingoNoteContext ctx = field.noteContext(GingoNote("E"));
ctx.degree;                            // 3
ctx.function;                          // FUNC_TONIC
ctx.inScale;                           // true
ctx.interval.semitones();              // 4
```

### GingoFretboard
```cpp
GingoFretboard guitar = GingoFretboard::guitar();
// Também: ::violao(), ::cavaquinho(), ::mandolin(), ::bandolim(), ::ukulele()
// Afinações alternativas: ::dropD(), ::openG(), ::dadgad()

guitar.noteAt(0, 5);                  // GingoNote("A")
guitar.midiAt(0, 0);                  // 40 (E2)

GingoFingering fgs[5];
guitar.fingerings(GingoChord("CM"), fgs, 5);

GingoFingering opens[5];
guitar.openFingerings(GingoChord("GM"), opens, 5);

GingoFingering ccs[7];
guitar.commonChords(GingoScale("G", SCALE_MAJOR), ccs, 7);

GingoFretboard capo2 = guitar.capo(2);
```

### GingoEvent e GingoSequence (Tier 3)
```cpp
GingoEvent ne = GingoEvent::noteEvent(GingoNote("C"), GingoDuration("quarter"), 4);
GingoEvent ce = GingoEvent::chordEvent(GingoChord("CM"), GingoDuration("half"));
GingoEvent re = GingoEvent::rest(GingoDuration("quarter"));

GingoSequence seq(GingoTempo(120), GingoTimeSig(4, 4));
seq.add(ne);
seq.totalBeats();   // 1.0
seq.totalSeconds(); // 0.5 a 120 BPM
seq.transpose(5);
```

### GingoMonitor (Tier 2+)

O Monitor recebe eventos musicais e mantém notas seguradas, pedal de sustain, acorde detectado, campo deduzido e contexto por nota. As entradas chegam de qualquer transporte externo. O Monitor em si não decodifica MIDI.

```cpp
GingoMonitor monitor;

monitor.setChannel(0xFF);   // aceita todos os channels (default), ou 0-15 pra filtrar

// Alimentar a partir dos callbacks do transporte:
monitor.noteOn(0, 60, 100);   // channel 0, C4, velocity 100
monitor.noteOn(0, 64, 100);   // channel 0, E4
monitor.noteOn(0, 67, 100);   // channel 0, G4
monitor.sustainOn();
monitor.sustainOff();
monitor.reset();              // all notes off

// Consultar estado:
monitor.hasChord();           // true
monitor.currentChord();       // GingoChord("CM")
monitor.currentField();       // GingoField

// Callbacks (Tier 3 suporta lambdas std::function):
monitor.onChordDetected([](const GingoChord& c)            { /* ... */ });
monitor.onFieldChanged([](const GingoField& f)             { /* ... */ });
monitor.onNoteOn      ([](const GingoNoteContext& ctx)     { /* ... */ });
```

### GingoMIDI1, adaptadores de saída (Tier 2+)
```cpp
// Evento único -> bytes MIDI 1.0 (NoteOn + NoteOff, 6 bytes pra eventos de nota).
uint8_t buf[6];
uint8_t n = GingoMIDI1::fromEvent(noteEvent, buf, sizeof(buf));

// Sequência -> stream de bytes MIDI 1.0. O default mantém o canal de
// cada evento; passe um 0-15 explícito pra forçar override em todos.
uint8_t out[256];
uint16_t total = GingoMIDI1::fromSequence(seq, out, sizeof(out));
// Ou com override explícito:
uint16_t total2 = GingoMIDI1::fromSequence(seq, out, sizeof(out), 5);
```

A entrada de byte stream MIDI 1.0 fica intencionalmente fora do escopo. Use qualquer parser externo (Arduino MIDI Library, parser próprio, etc.) e chame `GingoMonitor` direto. Veja [examples/MIDI2_Monitor/](examples/MIDI2_Monitor/) para um parser inline auto-contido.

### GingoMIDI2, adaptadores UMP Flex Data (Tier 3)
```cpp
GingoUMP ump = GingoMIDI2::chordName(GingoChord("CM"));
GingoUMP ump = GingoMIDI2::keySignature(scale);
GingoUMP ump = GingoMIDI2::keySignature(scale, group, channel);

GingoNoteContext ctx = field.noteContext(GingoNote("E"));
GingoUMP ump = GingoMIDI2::perNoteController(ctx, /*midiNote=*/64);

ump.wordCount;     // 4 (Flex Data 128-bit) ou 2 (per-note CC 64-bit)
ump.byteCount();   // total de bytes
uint8_t bytes[16];
ump.toBytesBE(bytes, sizeof(bytes));   // serialização big-endian pro fio
```

Dispatch de UMP recebido e MIDI-CI estão fora do escopo. Use `midi2_cpp` (ou outra lib UMP) para callbacks de recepção, e a biblioteca [`midi2`](https://github.com/sauloverissimo/midi2) C99 para fluxos de responder/initiator MIDI-CI.

### GingoChordComparison (Tier 3)
```cpp
GingoChordComparison cmp(GingoChord("CM"), GingoChord("Am"));
cmp.common_count;          // 2 (C e E em comum)
cmp.root_distance;         // 3 semitons
cmp.same_quality;          // false
cmp.voice_leading;         // movimento mínimo em semitons
cmp.transformation;        // NEO_R (Relative)
cmp.interval_vector_a[6];  // vetor intervalar de Forte
```

## Integração MIDI

O Monitor é o ponto único de entrada para eventos musicais. A cola entre o transporte externo e o Monitor leva poucas linhas e vive no seu sketch.

### MIDI 2.0 UMP via midi2_cpp

```cpp
midi2_cpp::Endpoint ep(...);
ep.on_note_on ([&](uint8_t g, uint8_t ch, uint8_t n, uint8_t v) {
    monitor.noteOn(ch, n, v);
});
ep.on_note_off([&](uint8_t g, uint8_t ch, uint8_t n, uint8_t v) {
    monitor.noteOff(ch, n);
});
ep.on_control_change([&](uint8_t g, uint8_t ch, uint8_t cc, uint32_t val) {
    if (cc == 64)  { (val >= 0x80000000U) ? monitor.sustainOn() : monitor.sustainOff(); }
    else if (cc == 123) { monitor.reset(); }
});

monitor.onChordDetected([&](const GingoChord& c) {
    GingoUMP ump = GingoMIDI2::chordName(c);
    ep.send_ump(ump.words, ump.wordCount);
});
```

### MIDI 1.0 via Arduino MIDI Library

```cpp
MIDI.setHandleNoteOn ([](byte ch, byte note, byte vel) {
    monitor.noteOn(ch - 1, note, vel);   // 1-16 -> 0-15 (convenção UMP)
});
MIDI.setHandleNoteOff([](byte ch, byte note, byte vel) {
    monitor.noteOff(ch - 1, note);
});
MIDI.setHandleControlChange([](byte ch, byte cc, byte val) {
    if (cc == 64)  { (val >= 64) ? monitor.sustainOn() : monitor.sustainOff(); }
    else if (cc == 123) { monitor.reset(); }
});
```

### DIN MIDI bruta na UART

Veja [examples/MIDI2_Monitor/MIDI2_Monitor.ino](examples/MIDI2_Monitor/MIDI2_Monitor.ino). O sketch traz um parser inline de cerca de 30 linhas que lida com running status, absorção de SysEx e bytes de real-time, e encaminha eventos musicais ao Monitor.

## Exemplos

| Exemplo | Descrição | Tier |
|---------|-----------|------|
| BasicNote | Criação de notas, transposição, MIDI, frequência | 1 |
| ChordNotes | Notas do acorde, intervalos, identify | 1 |
| ScaleExplorer | Escalas, modos, pentatônica | 2 |
| HarmonicField | Tríades, tétrades, funções harmônicas | 2 |
| TDisplayS3Explorer | GUI interativa de 7 páginas no LilyGo T-Display S3 | 3 |
| T-Display-S3-Piano | Visualizador piano de 25 teclas com análise teórica e síntese onboard | 3 |
| MIDI_to_Gingoduino | Recebe MIDI USB/BLE via ESP32_Host_MIDI e analisa com Gingoduino | 3 |
| RealtimeChordIdentifier | Identifica acordes a partir de notas simultâneas (entrada USB/BLE) | 3 |
| MIDI2_Monitor | UART MIDI 1.0 in (parser inline), análise no Monitor, UMP Flex Data out | 3 |
| Gingoduino_to_MIDI | Constrói uma sequência e serializa via `GingoMIDI1::fromSequence` | 3 |

## Testes nativos

```bash
g++ -std=c++11 -DGINGODUINO_TIER=3 -I. -Wall -Wextra -Werror \
    -o extras/tests/test_native extras/tests/test_native.cpp \
    && ./extras/tests/test_native
```

399 testes, 0 falhas. Sem o framework Arduino.

## Licença

MIT License. Veja [LICENSE](LICENSE).

## Autor

**Saulo Veríssimo**
- https://github.com/sauloverissimo
- sauloverissimo@gmail.com
