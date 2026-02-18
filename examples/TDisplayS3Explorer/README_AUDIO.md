# T-Display-S3 Explorer + Audio Synthesis

## 🎶 Visão Geral

Este exemplo combina a interface musical do Gingoduino (teoria musical pura) com **síntese de áudio em tempo real** via I2S.

A música é gerada matematicamente (sine wave synthesis) e enviada ao DAC PCM5102, que converte para áudio analógico na saída 3.5mm.

## 🔊 Hardware Necessário

### Obrigatório
- **LilyGo T-Display-S3** (ESP32-S3 + tela ST7789)
- **T-Display-S3 MIDI Shield v1.1** com módulo DAC PCM5102
- Fone de ouvido ou caixa de som (entrada 3.5mm)

### Pinagem I2S
```
T-Display-S3         PCM5102
GPIO_17 (DIN)  ——→  pino 14 (DIN)
GPIO_18 (BCK)  ——→  pino 13 (BCK)
GPIO_44 (LRCK) ——→  pino 15 (LRCK)
GPIO_43 (MCLK) ——→  pino 12 (SCLK)
GND            ——→  GND
```

## 🎮 Controles

| Botão | Ação |
|-------|------|
| **BOOT (LEFT)** | Muda página, para áudio |
| **KEY (RIGHT)** | Cicla itens + toca áudio<br/>Na página Sequence: PLAY/STOP |

## 🎵 Página por Página

### 1. Note Explorer (C C# D D# E F F# G G# A A# B)
- **Ao cyclar**: Toca a nota atual (500ms)
- **Resultado**: Sine wave na frequência da nota (octava 4)

### 2. Interval Explorer
- **Ao cyclar**: Toca intervalo como 2 notas simultâneas
- **Resultado**: C4 + intervalo, exemplo: C4 + E4 (M3)

### 3. Chord Explorer (CM, Cm, C7, Dm7, etc)
- **Ao cyclar**: Toca o acorde completo
- **Resultado**: Polifonia (até 4 vozes)

### 4. Scale Explorer
- **Ao cyclar**: Toca a escala como arpeggio (200ms por nota)
- **Resultado**: Sequência ascendente: C D E F G A B C

### 5. Harmonic Field
- **Ao cyclar**: Toca os 7 acordes do campo como progressão
- **Resultado**: I-ii-iii-IV-V-vi-vii°, exemplo em C Major: C-Dm-Em-F-G-Am-Bdim

### 6. Fretboard
- Sem áudio (foco no diagrama de acordes)

### 7. Sequence
- **Sequências predefinidas**: I-IV-V-I, ii-V-I Jazz, Simple Melody, Rests & Notes, Bossa
- **Botão PLAY**: Pressione RIGHT para executar a sequência
- **Resultado**: Executa cada evento da sequência com sua duração

## 🎛️ Arquitetura de Áudio

### Síntese
```
FreeRTOS audioTask (core 1)
    ↓
Sine wave oscillator (phase accumulation)
    ↓
ADSR envelope (attack/release)
    ↓
16-bit PCM samples (44.1kHz)
    ↓
I2S driver → PCM5102 DAC
    ↓
🔊 Saída 3.5mm (estéreo)
```

### Características
- **Frequência**: 44.1kHz (padrão CD)
- **Bit depth**: 16-bit
- **Polifonia**: até 4 vozes (soma de sines)
- **Envelope**: Attack rápido (instantâneo), Release 200ms

### Não-bloqueante
- Síntese roda em **core 1** (dedicado)
- Display roda em **core 0** (não afetado)
- Áudio tem prioridade baixa para não congelar UI

## 📝 Exemplo de Código

```cpp
// Tocar uma nota
GingoNote note("C");
playNote(note, octave=4, durationMs=500);

// Tocar um acorde (polifonia)
GingoChord chord("Cm7");
playChord(chord, octave=4, durationMs=500);

// Tocar escala como arpeggio
GingoScale scale("C", SCALE_MAJOR);
playScaleArpeggio(scale, octave=4);
```

## ⚠️ Limitações & Notas

### Bloqueio durante playback
- `playScaleArpeggio()` tem delays internos que **bloqueiam o loop()** por ~1.4s
- `playSequence()` bloqueia durante toda a duração da sequência
- **Solução futura**: usar task assíncrona para playback (v0.3+)

### Qualidade de áudio
- Síntese simples (sine wave pura) — sem harmonics
- Sem filtros, reverb, ou efectos
- Click/pop ao mudar frequências rapidamente (sem glide)
- **Solução futura**: adicionar oscillador com wavetable (v0.3+)

### Volume
- Pode soar baixo dependendo do fone/caixa
- Ajustar em `audioTask()`: mude `0.7f` em `sample * 0.7f` para valores entre 0.5 e 1.0
- **Solução futura**: botão de volume (v0.3+)

### Latência
- Latência: ~50-100ms (DMA I2S buffer)
- Aceitável para exploração educacional
- **Não recomendado** para aplicações profissionais de síntese em tempo real

## 🔧 Troubleshooting

### Sem som
1. ✓ Conferir conexões I2S (GPIO 17/18/44/43)
2. ✓ Verificar se fone/caixa está conectado
3. ✓ Serial monitor: verificar mensagem "Gingoduino T-Display-S3 Explorer + Audio Synthesis"
4. ✓ Tentar aumentar volume em `audioTask()` (0.7f → 0.9f)

### Som muito baixo
- Aumentar ganho em `audioTask()`: `sample * 0.7f` → `sample * 0.9f`
- Usar amplificador externo se disponível

### Engasgo/freeze ao tocar escala
- Normal! `playScaleArpeggio()` bloqueia por alguns segundos
- Solução: implementar task assíncrona (roadmap v0.3)

### Ruído/clicks
- I2S buffer pequeno? Aumentar `I2S_BUFFER_SIZE` de 512 para 1024
- DMA não configurado? Verificar `i2s_config_t` no código

## 🎯 Roadmap (v0.3+)

- [ ] Playback assíncrono (não bloqueia UI)
- [ ] Múltiplos tipos de oscilador (sawtooth, square, triangle)
- [ ] Filtro low-pass (ADSR)
- [ ] Glide/portamento entre notas
- [ ] Controle de volume (potenciômetro ou slider)
- [ ] Gravação de sequências custom
- [ ] MIDI input (receber via USB/Serial)

## 📚 Referências

- ESP32 I2S: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2s.html
- PCM5102 Datasheet: https://www.ti.com/lit/ds/symlink/pcm5102a.pdf
- T-Display-S3 Schematic: schema fornecido pelo LilyGo
- Gingoduino: https://github.com/sauloverissimo/gingoduino

---

**Versão**: 0.1.0
**Data**: 2026-02-18
**Autor**: Saulo Verissimo
**Licença**: MIT
