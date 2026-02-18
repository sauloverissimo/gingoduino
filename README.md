<p align="center">
  <img src="extras/gingoduino.png" alt="Gingoduino" width="480">
</p>

# 🪇 Gingo [duino]

**Music Theory Library for Embedded Systems**
**Biblioteca de Teoria Musical para Sistemas Embarcados**

---

## English Version

### Overview

Gingoduino is a comprehensive music theory library for embedded systems, microcontrollers, and creative hardware. It brings expressive music theory concepts directly to Arduino, ESP32, Teensy, Daisy Seed, Raspberry Pi Pico, and other platforms.

Built as a port of the [gingo C++17 library](https://github.com/sauloverissimo/gingo), Gingoduino provides access to:

- **Notes** — Chromatic notes with frequency, MIDI, and transposition
- **Intervals** — Musical intervals with naming and calculation
- **Chords** — Common and extended chord types with note extraction
- **Scales** — Major, minor, modal, and exotic scales
- **Harmonic Fields** — Triads, sevenths, and harmonic functions (Tonic/Subdominant/Dominant)
- **Rhythm** — Time signatures, durations, and tempo

### Features

✨ **Rich Music Theory Support**
- 12-note chromatic system with enharmonic equivalents
- 20+ chord types (major, minor, seventh, diminished, augmented, etc.)
- 40+ scale types and modes (major, minor, blues, whole tone, etc.)
- Harmonic field analysis with functional harmony
- Frequency calculations in Hz and MIDI note numbers

⚡ **Optimized for Embedded Systems**
- Minimal memory footprint with configurable feature tiers
- Fixed-size arrays (no dynamic allocation)
- PROGMEM support for Arduino RAM efficiency
- Compiles on AVR, ESP8266, ESP32, RP2040, Teensy, and more

🎛️ **Tiered Architecture**
- **Tier 1**: Notes, Intervals, Chords (minimal footprint)
- **Tier 2**: + Scales, Fields, Durations, Tempo, Time Signatures
- **Tier 3**: + Sequences, Events, Trees, Progressions (full features)
- Tiers auto-select based on platform, or override manually

🔌 **Hardware Integration**
- Works with displays (TFT_eSPI, Adafruit GFX, etc.)
- MIDI output ready
- Serial debugging included in examples

### Quick Start

#### Installation

1. **Via Arduino IDE Library Manager:**
   - Open Arduino IDE → Sketch → Include Library → Manage Libraries
   - Search for `Gingoduino`
   - Click Install

2. **Manual Installation:**
   - Download this repository
   - Copy the `gingoduino/` folder to your Arduino libraries directory:
     - macOS: `~/Documents/Arduino/libraries/`
     - Linux: `~/Arduino/libraries/`
     - Windows: `Documents\Arduino\libraries\`

3. **For ESP32 users (if needed):**
   - Ensure your board package is installed
   - Library auto-detects platform and loads appropriate tier

#### Basic Usage

```cpp
#include <Gingoduino.h>

using namespace gingoduino;

void setup() {
    Serial.begin(9600);

    // Create a note
    GingoNote note("C");

    // Get properties
    Serial.println(note.name());           // "C"
    Serial.println(note.semitone());       // 0
    Serial.println(note.midiNumber(4));    // 60 (Middle C)
    Serial.println(note.frequency(4), 2);  // 261.63 Hz

    // Transpose
    GingoNote fifth = note.transpose(7);   // C + 7 semitones = G
}

void loop() {}
```

#### Examples Included

1. **BasicNote** — Note creation, transposition, MIDI, frequency
2. **ChordNotes** — Extract notes from chords, interval analysis
3. **ScaleExplorer** — Browse scales and modes
4. **HarmonicField** — Triads, sevenths, and harmonic functions
5. **TDisplayS3Explorer** — Interactive GUI on LilyGo T-Display S3 (170×320 TFT)

Run any example via Arduino IDE: File → Examples → Gingoduino → [Example Name]

### API Documentation

#### GingoNote

```cpp
GingoNote note("C#");

// Properties
note.name();              // "C#"
note.natural();           // "C"
note.semitone();          // 1 (0-11)
note.isSharp();           // true
note.isFlat();            // false
note.isNatural();         // false

// Frequency & MIDI
note.frequency(octave);   // Hz (float)
note.midiNumber(octave);  // 0-127
note.isEnharmonic(other); // Check if equivalent note

// Transposition
note.transpose(semitones);// Returns new GingoNote

// Distance
note.distance(other);     // Semitones between notes
```

#### GingoChord

```cpp
GingoChord chord("Cm7");

// Properties
chord.name();             // "Cm7"
chord.root();             // GingoNote("C")
chord.type();             // "m7" or full name
chord.quality();          // "minor seventh"

// Extract notes
GingoNote notes[7];
uint8_t count = chord.notes(notes, 7);  // Fill array with notes

// Interval labels
LabelStr labels[7];
uint8_t count = chord.intervalLabels(labels, 7);  // "R", "3m", "5", "7m"
```

#### GingoScale

```cpp
GingoScale scale("A", SCALE_NATURAL_MINOR);

// Properties
scale.modeName(buf, size);  // "Natural Minor" (Aeolian)
scale.quality();            // "minor"

// Extract notes
GingoNote notes[12];
uint8_t count = scale.notes(notes, 12);

// Pentatonic variant
GingoScale penta = scale.pentatonic();
uint8_t pentaCount = penta.notes(pentaNotes, 12);
```

#### GingoField

```cpp
GingoField field("C", SCALE_MAJOR);

// Get triads (I, II, III, IV, V, VI, VII)
GingoChord triads[7];
uint8_t count = field.chords(triads, 7);

// Get seventh chords
GingoChord sevenths[7];
uint8_t count = field.sevenths(sevenths, 7);

// Get harmonic function
uint8_t function = field.function(degree);  // FUNC_TONIC, FUNC_SUBDOMINANT, FUNC_DOMINANT
```

### Supported Chord Types

- **Triads**: M, m, dim, aug
- **Sevenths**: 7, M7, m7, m7(b5), dim7
- **Extended**: 9, 11, 13 (partial support)
- **Alterations**: sus2, sus4, add9, etc.

### Supported Scale Types

```cpp
SCALE_MAJOR
SCALE_NATURAL_MINOR
SCALE_HARMONIC_MINOR
SCALE_MELODIC_MINOR
SCALE_IONIAN
SCALE_DORIAN
SCALE_PHRYGIAN
SCALE_LYDIAN
SCALE_MIXOLYDIAN
SCALE_AEOLIAN
SCALE_LOCRIAN
SCALE_BLUES
SCALE_PENTATONIC_MAJOR
SCALE_PENTATONIC_MINOR
SCALE_WHOLE_TONE
SCALE_DIMINISHED
// ... and more
```

### Configuration

Override default settings **before** including the library:

```cpp
// Limit memory usage
#define GINGODUINO_MAX_CHORD_NOTES 5
#define GINGODUINO_MAX_SCALE_NOTES 8

// Force a specific tier (1, 2, or 3)
#define GINGODUINO_TIER 1

#include <Gingoduino.h>
```

**Platform Auto-Detection:**
- **AVR** (Arduino Uno, Nano): Tier 1
- **ESP8266**: Tier 2
- **ESP32, RP2040, Teensy, Daisy Seed**: Tier 3

### Memory Usage

Approximate memory footprint (in bytes):

| Platform | Tier 1 | Tier 2 | Tier 3 |
|----------|--------|--------|--------|
| AVR      | ~2KB   | ~4KB   | N/A    |
| ESP8266  | ~3KB   | ~6KB   | N/A    |
| ESP32    | ~4KB   | ~8KB   | ~15KB  |

*Actual usage depends on configuration and included features.*

### Supported Platforms

| Platform | Tested | Supported |
|----------|--------|-----------|
| Arduino Uno/Nano (AVR) | ✓ | ✓ |
| Arduino Mega | ✓ | ✓ |
| Arduino Leonardo | ✓ | ✓ |
| ESP32 | ✓ | ✓ |
| ESP8266 | ✓ | ✓ |
| Raspberry Pi Pico | ✓ | ✓ |
| Teensy 3.x / 4.x | ✓ | ✓ |
| Daisy Seed | ✓ | ✓ |
| Arduino MKR series | ✓ | ✓ |

### T-Display S3 Explorer Example

The included **TDisplayS3Explorer** example demonstrates a full interactive application on the LilyGo T-Display S3 (ESP32-S3 with 170×320 TFT display):

**Hardware Required:**
- LilyGo T-Display S3 board
- USB-C cable for programming

**Setup:**
1. Install TFT_eSPI library via Arduino IDE
2. Configure TFT_eSPI for T-Display S3:
   - Edit `TFT_eSPI/User_Setup_Select.h`
   - Comment out: `#include <User_Setup.h>`
   - Uncomment: `#include <User_Setups/Setup206_LilyGo_T_Display_S3.h>`
3. Upload TDisplayS3Explorer sketch

**Navigation:**
- **LEFT button (BOOT)**: Switch pages
- **RIGHT button (KEY)**: Cycle items within page

**Pages:**
1. **Note Explorer** — 12 chromatic notes with MIDI and frequency
2. **Chord Explorer** — Common chords with interval analysis
3. **Scale Explorer** — Scales, modes, and pentatonic variants
4. **Harmonic Field** — Triads/sevenths with harmonic functions (T/S/D)

### Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### License

MIT License — See [LICENSE](LICENSE) file for details

### Author

**Saulo Verissimo**
sauloverissimo@gmail.com
https://github.com/sauloverissimo

### Acknowledgments

- Original [gingo library](https://github.com/sauloverissimo/gingo) (C++17)
- Arduino community and ecosystem
- Inspired by music theory pedagogy and creative coding

---

## Versão em Português

### Visão Geral

Gingoduino é uma biblioteca completa de teoria musical para sistemas embarcados e microcontroladores. Traz conceitos expressivos de teoria musical diretamente para Arduino, ESP32, Teensy, Daisy Seed, Raspberry Pi Pico e outras plataformas.

Desenvolvida como port da [biblioteca gingo C++17](https://github.com/sauloverissimo/gingo), Gingoduino fornece acesso a:

- **Notas** — Notas cromáticas com frequência, MIDI e transposição
- **Intervalos** — Intervalos musicais com nomes e cálculos
- **Acordes** — Tipos de acordes comuns e estendidos com extração de notas
- **Escalas** — Escalas maiores, menores, modais e exóticas
- **Campos Harmônicos** — Tríades, tétrades e funções harmônicas (Tônica/Subdominante/Dominante)
- **Ritmo** — Fórmulas de compasso, durações e tempo

### Características

✨ **Suporte Rico em Teoria Musical**
- Sistema cromático com 12 notas e enarmônicos
- 20+ tipos de acordes (maior, menor, sétima, diminuto, aumentado, etc.)
- 40+ tipos de escalas e modos (maior, menor, blues, tons inteiros, etc.)
- Análise de campo harmônico com harmonia funcional
- Cálculos de frequência em Hz e números MIDI

⚡ **Otimizado para Sistemas Embarcados**
- Pegada mínima de memória com camadas de funcionalidade configuráveis
- Arrays de tamanho fixo (sem alocação dinâmica)
- Suporte a PROGMEM para eficiência de RAM no Arduino
- Compila em AVR, ESP8266, ESP32, RP2040, Teensy e mais

🎛️ **Arquitetura em Camadas**
- **Camada 1**: Notas, Intervalos, Acordes (mínima)
- **Camada 2**: + Escalas, Campos, Durações, Tempo, Fórmulas de Compasso
- **Camada 3**: + Sequências, Eventos, Árvores, Progressões (completa)
- Camadas auto-selecionadas por plataforma, ou override manual

🔌 **Integração com Hardware**
- Funciona com displays (TFT_eSPI, Adafruit GFX, etc.)
- Pronto para saída MIDI
- Debug por Serial incluído nos exemplos

### Início Rápido

#### Instalação

1. **Via Arduino IDE Library Manager:**
   - Abra Arduino IDE → Sketch → Include Library → Manage Libraries
   - Procure por `Gingoduino`
   - Clique em Install

2. **Instalação Manual:**
   - Faça download deste repositório
   - Copie a pasta `gingoduino/` para sua pasta de bibliotecas Arduino:
     - macOS: `~/Documents/Arduino/libraries/`
     - Linux: `~/Arduino/libraries/`
     - Windows: `Documents\Arduino\libraries\`

3. **Para usuários de ESP32 (se necessário):**
   - Certifique-se de que o pacote da placa está instalado
   - Biblioteca auto-detecta plataforma e carrega camada apropriada

#### Uso Básico

```cpp
#include <Gingoduino.h>

using namespace gingoduino;

void setup() {
    Serial.begin(9600);

    // Criar uma nota
    GingoNote nota("C");

    // Obter propriedades
    Serial.println(nota.name());           // "C"
    Serial.println(nota.semitone());       // 0
    Serial.println(nota.midiNumber(4));    // 60 (Dó médio)
    Serial.println(nota.frequency(4), 2);  // 261,63 Hz

    // Transpor
    GingoNote quinta = nota.transpose(7);  // C + 7 semitons = G
}

void loop() {}
```

#### Exemplos Inclusos

1. **BasicNote** — Criação de notas, transposição, MIDI, frequência
2. **ChordNotes** — Extração de notas de acordes, análise de intervalos
3. **ScaleExplorer** — Navegação em escalas e modos
4. **HarmonicField** — Tríades, tétrades e funções harmônicas
5. **TDisplayS3Explorer** — GUI interativa no LilyGo T-Display S3 (170×320 TFT)

Execute qualquer exemplo via Arduino IDE: File → Examples → Gingoduino → [Nome do Exemplo]

### Documentação da API

#### GingoNote

```cpp
GingoNote nota("C#");

// Propriedades
nota.name();              // "C#"
nota.natural();           // "C"
nota.semitone();          // 1 (0-11)
nota.isSharp();           // true
nota.isFlat();            // false
nota.isNatural();         // false

// Frequência & MIDI
nota.frequency(oitava);   // Hz (float)
nota.midiNumber(oitava);  // 0-127
nota.isEnharmonic(outra); // Verifica se nota equivalente

// Transposição
nota.transpose(semitons); // Retorna novo GingoNote

// Distância
nota.distance(outra);     // Semitons entre notas
```

#### GingoChord

```cpp
GingoChord acorde("Cm7");

// Propriedades
acorde.name();             // "Cm7"
acorde.root();             // GingoNote("C")
acorde.type();             // "m7" ou nome completo
acorde.quality();          // "minor seventh"

// Extrair notas
GingoNote notas[7];
uint8_t contador = acorde.notes(notas, 7);  // Preenche array

// Rótulos de intervalos
LabelStr rotulos[7];
uint8_t contador = acorde.intervalLabels(rotulos, 7);  // "R", "3m", "5", "7m"
```

#### GingoScale

```cpp
GingoScale escala("A", SCALE_NATURAL_MINOR);

// Propriedades
escala.modeName(buf, tam);  // "Natural Minor" (Aeolian)
escala.quality();           // "minor"

// Extrair notas
GingoNote notas[12];
uint8_t contador = escala.notes(notas, 12);

// Variante pentatônica
GingoScale penta = escala.pentatonic();
uint8_t contPenta = penta.notes(notasPenta, 12);
```

#### GingoField

```cpp
GingoField campo("C", SCALE_MAJOR);

// Obter tríades (I, II, III, IV, V, VI, VII)
GingoChord triades[7];
uint8_t contador = campo.chords(triades, 7);

// Obter acordes com sétima
GingoChord setimas[7];
uint8_t contador = campo.sevenths(setimas, 7);

// Obter função harmônica
uint8_t funcao = campo.function(grau);  // FUNC_TONIC, FUNC_SUBDOMINANT, FUNC_DOMINANT
```

### Tipos de Acordes Suportados

- **Tríades**: M, m, dim, aug
- **Sétimas**: 7, M7, m7, m7(b5), dim7
- **Estendidos**: 9, 11, 13 (suporte parcial)
- **Alterações**: sus2, sus4, add9, etc.

### Tipos de Escalas Suportadas

```cpp
SCALE_MAJOR
SCALE_NATURAL_MINOR
SCALE_HARMONIC_MINOR
SCALE_MELODIC_MINOR
SCALE_IONIAN
SCALE_DORIAN
SCALE_PHRYGIAN
SCALE_LYDIAN
SCALE_MIXOLYDIAN
SCALE_AEOLIAN
SCALE_LOCRIAN
SCALE_BLUES
SCALE_PENTATONIC_MAJOR
SCALE_PENTATONIC_MINOR
SCALE_WHOLE_TONE
SCALE_DIMINISHED
// ... e mais
```

### Configuração

Sobrescreva configurações padrão **antes** de incluir a biblioteca:

```cpp
// Limitar uso de memória
#define GINGODUINO_MAX_CHORD_NOTES 5
#define GINGODUINO_MAX_SCALE_NOTES 8

// Forçar uma camada específica (1, 2 ou 3)
#define GINGODUINO_TIER 1

#include <Gingoduino.h>
```

**Auto-Detecção de Plataforma:**
- **AVR** (Arduino Uno, Nano): Camada 1
- **ESP8266**: Camada 2
- **ESP32, RP2040, Teensy, Daisy Seed**: Camada 3

### Uso de Memória

Pegada aproximada de memória (em bytes):

| Plataforma | Camada 1 | Camada 2 | Camada 3 |
|-----------|----------|----------|----------|
| AVR       | ~2KB     | ~4KB     | N/A      |
| ESP8266   | ~3KB     | ~6KB     | N/A      |
| ESP32     | ~4KB     | ~8KB     | ~15KB    |

*O uso real depende da configuração e recursos inclusos.*

### Plataformas Suportadas

| Plataforma | Testado | Suportado |
|-----------|---------|-----------|
| Arduino Uno/Nano (AVR) | ✓ | ✓ |
| Arduino Mega | ✓ | ✓ |
| Arduino Leonardo | ✓ | ✓ |
| ESP32 | ✓ | ✓ |
| ESP8266 | ✓ | ✓ |
| Raspberry Pi Pico | ✓ | ✓ |
| Teensy 3.x / 4.x | ✓ | ✓ |
| Daisy Seed | ✓ | ✓ |
| Arduino MKR series | ✓ | ✓ |

### Exemplo T-Display S3 Explorer

O exemplo incluído **TDisplayS3Explorer** demonstra uma aplicação interativa completa no LilyGo T-Display S3 (ESP32-S3 com display TFT 170×320):

**Hardware Necessário:**
- Placa LilyGo T-Display S3
- Cabo USB-C para programação

**Configuração:**
1. Instale biblioteca TFT_eSPI via Arduino IDE
2. Configure TFT_eSPI para T-Display S3:
   - Edite `TFT_eSPI/User_Setup_Select.h`
   - Comente: `#include <User_Setup.h>`
   - Descomente: `#include <User_Setups/Setup206_LilyGo_T_Display_S3.h>`
3. Faça upload do sketch TDisplayS3Explorer

**Navegação:**
- **Botão ESQUERDO (BOOT)**: Trocar páginas
- **Botão DIREITO (KEY)**: Navegar itens dentro da página

**Páginas:**
1. **Note Explorer** — 12 notas cromáticas com MIDI e frequência
2. **Chord Explorer** — Acordes comuns com análise de intervalos
3. **Scale Explorer** — Escalas, modos e variantes pentatônicas
4. **Harmonic Field** — Tríades/sétimas com funções harmônicas (T/S/D)

### Contribuindo

Contribuições são bem-vindas! Por favor:

1. Faça fork do repositório
2. Crie uma branch de feature (`git checkout -b feature/minha-feature`)
3. Faça commit de suas mudanças (`git commit -m 'Adiciona minha feature'`)
4. Faça push para a branch (`git push origin feature/minha-feature`)
5. Abra um Pull Request

### Licença

Licença MIT — Veja arquivo [LICENSE](LICENSE) para detalhes

### Autor

**Saulo Verissimo**
sauloverissimo@gmail.com
https://github.com/sauloverissimo

### Agradecimentos

- Biblioteca original [gingo](https://github.com/sauloverissimo/gingo) (C++17)
- Comunidade e ecossistema Arduino
- Inspirado em pedagogia de teoria musical e programação criativa

---

## Changelog

### v0.1.0 (Initial Release)
- Port from gingo C++17 library
- Tier 1: Notes, Intervals, Chords
- Tier 2: Scales, Fields, Rhythm
- Tier 3: Sequences, Events, Progressions
- TDisplayS3Explorer interactive example
- Multiple Arduino platforms support
