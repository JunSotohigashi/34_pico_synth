# Panel Module (pico-synth-panel)

パネルモジュールは、キーボードから受け取った MIDI イベントを Processing し、16 個の Sound ユニット（8台の Pico × 2ボイス）に各ボイスの合成パラメータを配信する中核的な音声制御ユニットです。

## 概要

**主な機能：**
- UART MIDI 受信：Keyboard モジュールからの MIDI Note On/Off イベント受信（115200 bps）
- Voice Allocation：最大16個の同時発音ボイスを管理
- ADC 入力処理：16 個のポテンショメータ（VR）から Synth パラメータ値を読み込み
- スイッチ入力：5種類のモード切替スイッチ（VCO1/VCO2波形、VCFタイプ、LFO波形/ターゲット）
- SPI 出力：8台の Sound モジュールに音声パラメータストリームを配信
- LED 制御：シフトレジスタ経由で各種状態インジケータを点灯

**ハードウェア構成：**
- RP2040 デュアルコア
- UART×1：Keyboard MIDI 受信 (115200 bps)
- SPI×2：Panel用（ADC+LED制御）、Sound用（音声パラメータ配信）
- ADC×2（MCP3008）：合計16チャンネルのアナログ入力
- LED制御×5（74HC595シフトレジスタ）：ユニット状態、モード表示
- スイッチ×10（5種類 × R/L）：各種モード選択

## ハードウェア詳細

### VR（ポテンショメータ）マッピング

既存ハードウェアの16個のVRを2つのMCP3008 ADC（各8チャンネル）で読み込み：

**ADC1（MCP3008 #1）:**

| CH | VR 機能 | 範囲 | 説明 |
|----|---------|------|------|
| 0 | VCF CutOff | 0-1023 | フィルターカットオフ周波数 |
| 1 | VCO Mix | 0-1023 | VCO1とVCO2のミックス比 |
| 2 | VCO Duty | 0-1023 | 矩形波のDuty比（パルス幅） |
| 3 | VCO2 Tune | 0-1023 | サブオシレーター（VCO2）の周波数微調整 |
| 4 | VCF Release | 0-1023 | VCF Envelope Release時間 |
| 5 | VCF Decay | 0-1023 | VCF Envelope Decay時間 |
| 6 | VCF Attack | 0-1023 | VCF Envelope Attack時間 |
| 7 | VCF Resonance | 0-1023 | フィルター共振（Q値） |

**ADC2（MCP3008 #2）:**

| CH | VR 機能 | 範囲 | 説明 |
|----|---------|------|------|
| 0 | LFO Depth | 0-1023 | LFO モジュレーション深度 |
| 1 | LFO Speed | 0-1023 | LFO 周波数 |
| 2 | VCA Gain | 0-1023 | 最終出力ゲイン |
| 3 | VCA Release | 0-1023 | VCA Envelope Release時間 |
| 4 | VCA Sustain | 0-1023 | VCA Envelope Sustain レベル |
| 5 | VCA Decay | 0-1023 | VCA Envelope Decay時間 |
| 6 | VCA Attack | 0-1023 | VCA Envelope Attack時間 |
| 7 | VCF Sustain | 0-1023 | VCF Envelope Sustain レベル |

### SW（スイッチ）マッピング

既存ハードウェアの5種類のスイッチ（各R/Lボタン）：

| スイッチ | R ボタン | L ボタン | 状態数 | 機能 |
|----------|----------|----------|--------|------|
| VCO1 | GPIO29 | GPIO28 | 4 | VCO1 波形選択（のこぎり波/正弦波/三角波/矩形波） |
| VCO2 | GPIO27 | GPIO26 | 4 | VCO2 波形選択（のこぎり波/正弦波/三角波/矩形波） |
| VCF | GPIO25 | GPIO24 | 2 | フィルタータイプ（LPF/HPF） |
| LFO Wave | GPIO23 | GPIO22 | 6 | LFO波形（のこぎり波↑/↓/正弦波/三角波/矩形波/ランダム） |
| LFO Target | GPIO21 | GPIO20 | 6 | LFO適用先（VCO2周波数/VCO1+2周波数/VCO Duty/VCO Mix/VCF CutOff/VCA Gain） |

### LED（シフトレジスタ）構成

5個の74HC595シフトレジスタでLED制御：

| シフトレジスタ | CS Pin | 機能 |
|---------------|--------|------|
| Unit Low | GPIO2 | Voice Unit 0-7 状態表示 |
| Unit High | GPIO5 | Voice Unit 8-15 状態表示 |
| LFO High VCF | GPIO3 | LFO波形選択、VCFタイプ表示 |
| LFO Low | GPIO4 | LFOターゲット表示 |
| VCO | GPIO6 | VCO1/VCO2 波形表示 |

**Pin 割り当て（既存ハードウェア）:**
```
UART:
  TX: GPIO0
  RX: GPIO1

SPI Panel (spi0):
  SCK:  GPIO18
  MOSI: GPIO19
  MISO: GPIO16
  CS ADC1: GPIO7
  CS ADC2: GPIO8
  CS SR (Unit Low): GPIO2
  CS SR (LFO High VCF): GPIO3
  CS SR (LFO Low): GPIO4
  CS SR (Unit High): GPIO5
  CS SR (VCO): GPIO6

SPI Sound (spi1):
  SCK:  GPIO14
  MOSI: GPIO15
  MISO: GPIO12
  CS:   GPIO13

Switches:
  VCO1 R/L:    GPIO29/28
  VCO2 R/L:    GPIO27/26
  VCF R/L:     GPIO25/24
  LFO Wave R/L:   GPIO23/22
  LFO Target R/L: GPIO21/20
```

### SPI Sound プロトコル（既存実装）

**通信方式：**
- Clock: 1 MHz（既存ハードウェア）
- Mode: SPI Mode 0 (CPOL=0, CPHA=0)
- Format: 16-bit ワード × 34 = 68 バイト ストリーム
- 周期: メインループ毎に全データ送信（約10ms間隔）

**ストリーム構造（34ワード）:**

```
Word 0:     0xFFFF（ヘッダー）
Word 1-16:  Unit 0-15 状態
            bit 15:    発音中フラグ（1=ON, 0=OFF）
            bit 14-8:  Note番号（0-127）
            bit 7-0:   Velocity（0-127）
Word 17:    モード設定
            bit 11-10: VCO1 波形（0-3）
            bit 9-8:   VCO2 波形（0-3）
            bit 7:     VCF タイプ（0=LPF, 1=HPF）
            bit 6-3:   LFO 波形（0-5）
            bit 2-0:   LFO ターゲット（0-5）
Word 18:    VCO Duty（ADC1 ch2）
Word 19:    VCO2 Tune（ADC1 ch3）
Word 20:    VCO Mix（ADC1 ch1）
Word 21:    VCF CutOff（ADC1 ch0）
Word 22:    VCF Resonance（ADC1 ch7）
Word 23:    VCF Attack（ADC1 ch6）
Word 24:    VCF Decay（ADC1 ch5）
Word 25:    VCF Sustain（ADC2 ch7）
Word 26:    VCF Release（ADC1 ch4）
Word 27:    VCA Attack（ADC2 ch6）
Word 28:    VCA Decay（ADC2 ch5）
Word 29:    VCA Sustain（ADC2 ch4）
Word 30:    VCA Release（ADC2 ch3）
Word 31:    VCA Gain（ADC2 ch2）
Word 32:    LFO Speed（ADC2 ch1）
Word 33:    LFO Depth（ADC2 ch0）
```

## Voice Allocation エンジン

パネルは MIDI 入力に基づいて最大 16 個のユニット（8台 Pico × 2ボイス）に Note イベントを配分します。

**アーキテクチャ：**
- Active Voice Table: 16 エントリ (Unit Index 0-15)
- Round-robin 方式: 空きユニットを順番に割り当て
- Voice Stealing: 全ユニット使用中の場合、最も古いユニットを置き換え
- Sustain Pedal 対応: ペダル押下中はリリースを遅延

**Voice Allocation ロジック（既存実装）:**
```
Note On イベント:
  1. unit_next から順に空きユニット（unit_state bit=0）を探す
  2. 見つかった場合:
     - unit_state |= (1 << unit)  // bit を立てる
     - unit_note[unit] = note
     - unit_velocity[unit] = velocity
     - unit_next = (unit + 1) % 16
     - unit_busy キューに追加
  3. 空きがない場合（Voice Stealing）:
     - unit_busy キューから最も古いユニットを取り出す
     - そのユニットを新しいノートで上書き

Note Off イベント:
  1. Sustain がOFFの場合:
     - unit_busy キューを走査
     - unit_note[unit] == note のユニットを解放
     - unit_state &= ~(1 << unit)
  2. Sustain がONの場合:
     - note_state[note] = RELEASE に遷移
     - Sustain OFF時にまとめて解放

Sustain Pedal (CC 64):
  - Sustain ON: note_state を RELEASE で保持
  - Sustain OFF: RELEASE 状態の全ノートを解放
```

## 通信プロトコル

### UART MIDI (Keyboard 受信)

- Baud Rate: 115200 bps
- Format: ASCII hex ("90 3c 7f\n" = Note On, Key 60, Velocity 127)
- Direction: Receive only

**メッセージタイプ：**
- `90 [key] [velocity]` - Note On (velocity=0でNote Off扱い)
- `80 [key] 00` - Note Off
- `B0 40 [value]` - Sustain Pedal (CC 64, value ≥ 64 = On)

## アーキテクチャ（既存実装ベース）

```
┌─────────────────┐
│  Keyboard MIDI  │
│   (UART RX)     │
└────────┬────────┘
         │ 115200 bps ASCII hex
         ▼
    ┌──────────────────────────────────────────────────────────┐
    │   Panel Module (RP2040)                                  │
    │                                                           │
    │  Core 1 (UART MIDI受信スレッド):                          │
    │  ┌────────────────────────────────────────────────────┐  │
    │  │ • UART文字列バッファリング                          │  │
    │  │ • ASCII hex → バイナリ変換                         │  │
    │  │ • Note On/Off → Voice Allocation                   │  │
    │  │ • Sustain CC → note_state 管理                     │  │
    │  │ • unit_state, unit_note[], unit_velocity[] 更新    │  │
    │  │   （セマフォで排他制御）                            │  │
    │  └────────────────────────────────────────────────────┘  │
    │                                                           │
    │  Core 0 (メインループ):                                  │
    │  ┌────────────────────────────────────────────────────┐  │
    │  │ 1. スイッチ読み込み（Selector.update()）           │  │
    │  │    • VCO1/VCO2 波形選択                            │  │
    │  │    • VCF タイプ（LPF/HPF）                        │  │
    │  │    • LFO 波形/ターゲット選択                      │  │
    │  │                                                    │  │
    │  │ 2. LED更新（74HC595 シフトレジスタ）               │  │
    │  │    • Unit 状態表示（16 bit → 2個のSR）           │  │
    │  │    • モード状態表示（VCO/VCF/LFO）                │  │
    │  │                                                    │  │
    │  │ 3. ADC読み込み（MCP3008 × 2）                     │  │
    │  │    • ADC1: VCF, VCO パラメータ（8ch）             │  │
    │  │    • ADC2: VCA, LFO パラメータ（8ch）             │  │
    │  │                                                    │  │
    │  │ 4. ストリーム構築（34 words × 16 bit）             │  │
    │  │    • Word 0: ヘッダー（0xFFFF）                   │  │
    │  │    • Word 1-16: Unit状態（セマフォ保護読み込み）   │  │
    │  │    • Word 17: モード設定                           │  │
    │  │    • Word 18-33: パラメータ値                     │  │
    │  │                                                    │  │
    │  │ 5. SPI Sound送信（spi1）                          │  │
    │  │    • 34回のCS toggle + 16bit転送                  │  │
    │  └────────────────────────────────────────────────────┘  │
    │                                                           │
    └────────────┬──────────────────────────────┬───────────────┘
                 │                              │
                 ▼                              ▼
        ┌─────────────────┐           ┌──────────────────┐
        │ Sound Modules   │           │ ADC/Switch/LED   │
        │  (8 Pico × 2ch) │           │ (Panel Hardware) │
        │  SPI1 16bit     │           │ SPI0 8bit        │
        └─────────────────┘           └──────────────────┘
```
    │         └────────┬─────────────────────┤
    │                  ▼                      │
    │  ┌─────────────────────────────────────┐│
    │  │  Parameter Engine (Core 1)          ││
    │  │  • ADC scan (round-robin)           ││
    │  │  • Parameter smoothing              ││
    │  │  • Generate SPI updates             ││
    │  │  • Queue parameter packets          ││
    │  └─────────────────────────────────────┘│
    │         │                                │
    │         ▼                                │
    │  ┌─────────────────────────────────────┐│
    │  │  SPI Transmitter (Core 0)           ││
    │  │  • Dequeue param packets            ││
    │  │  • Send via SPI to Sound modules    ││
    │  │  • Broadcast to all 8 voices        ││
    │  └─────────────────────────────────────┘│
    └────────┬────────────────┬───────────────┘
             │                │
             ▼                ▼
    ┌─────────────────┐  ┌──────────────┐
    │ Sound Modules   │  │  ADC/Mux     │
    │  (SPI slave)    │  │ (Potentiom.) │
    └─────────────────┘  └──────────────┘
```

## 実装詳細（既存実装ベース）

### Core 1: UART MIDI 受信スレッド

```cpp
constexpr uint8_t N_UNIT = 16;  // 16 ボイス（8 Pico × 2ch）

// 共有変数（セマフォで排他制御）
semaphore_t sem;
uint16_t unit_state = 0;           // 各ビットが Unit 0-15 の ON/OFF
uint8_t unit_note[N_UNIT] = {0};   // 各 Unit の Note 番号
uint8_t unit_velocity[N_UNIT] = {0};  // 各 Unit の Velocity

enum class KEY { OFF, PUSH, RELEASE };

void main_core1() {
  char buf[256] = {0};
  char buf_msg[12] = {0};
  uint8_t buf_index = 0;
  
  uint8_t unit_next = 0;
  queue_t unit_busy;  // 使用中 Unit の管理キュー
  queue_init(&unit_busy, sizeof(uint8_t), N_UNIT);
  KEY note_state[128] = {KEY::OFF};
  bool sustain = false;
  
  while (true) {
    if (uart_is_readable(UART_PORT)) {
      buf[buf_index] = uart_getc(UART_PORT);
      
      if (buf[buf_index] == '\n') {  // メッセージ終端
        // 最後の9文字を抽出："90 3c 7f\n"
        for (uint16_t i = 0; i <= 8; i++) {
          buf_msg[i] = buf[(buf_index + i - 8) & 0xFF];
        }
        
        // ASCII hex → バイナリ変換
        uint8_t msg[3];
        sscanf(buf_msg, "%hhx %hhx %hhx", &msg[0], &msg[1], &msg[2]);
        
        sem_acquire_blocking(&sem);
        
        if (msg[0] == 0x90 && msg[2] == 0) {  // Note Off
          if (sustain) {
            note_state[msg[1]] = KEY::RELEASE;
          } else {
            note_state[msg[1]] = KEY::OFF;
            // unit_busy から該当 Unit を解放
            uint8_t i_max = queue_get_level(&unit_busy);
            for (uint8_t i = 0; i < i_max; i++) {
              uint8_t unit;
              queue_remove_blocking(&unit_busy, &unit);
              if ((unit_state & (1 << unit)) && (unit_note[unit] == msg[1])) {
                unit_state &= ~(1 << unit);
                unit_note[unit] = 0;
                unit_velocity[unit] = 0;
              } else {
                queue_add_blocking(&unit_busy, &unit);
              }
            }
          }
        } 
        else if (msg[0] == 0x90 && msg[2] != 0) {  // Note On
          note_state[msg[1]] = KEY::PUSH;
          bool found_free_unit = false;
          
          // 空き Unit を Round-robin で探す
          for (uint8_t i = 0; i < N_UNIT; i++) {
            uint8_t unit = (i + unit_next) % N_UNIT;
            if (!(unit_state & 1 << unit)) {
              unit_state |= 1 << unit;
              unit_note[unit] = msg[1];
              unit_velocity[unit] = msg[2];
              unit_next = (unit + 1) % N_UNIT;
              queue_add_blocking(&unit_busy, &unit);
              found_free_unit = true;
              break;
            }
          }
          
          // 空きがない場合：Voice Stealing
          if (!found_free_unit) {
            uint8_t unit;
            queue_remove_blocking(&unit_busy, &unit);
            unit_note[unit] = msg[1];
            unit_velocity[unit] = msg[2];
            unit_next = (unit + 1) % N_UNIT;
            queue_add_blocking(&unit_busy, &unit);
          }
        }
        else if (msg[0] == 0xB0 && msg[1] == 0x40) {  // Sustain
          sustain = (bool)msg[2];
          
          // Sustain OFF 時：RELEASE 状態のノートを解放
          if (!sustain) {
            for (uint8_t note = 0; note < 128; note++) {
              if (note_state[note] == KEY::RELEASE) {
                note_state[note] = KEY::OFF;
                // 該当 Unit を解放（Note Off と同じ処理）
                // ...
              }
            }
          }
        }
        
        sem_release(&sem);
      }
      buf_index++;
    }
  }
}
```

### Core 0: メインループ

```cpp
int main() {
  stdio_init_all();
  sem_init(&sem, 1, 1);
  
  // 初期化
  uart_init(UART_PORT, 115200);
  spi_init(SPI_PORT_PANEL, 1000 * 1000);   // Panel SPI: 1 MHz
  spi_init(SPI_PORT_SOUND, 1000 * 1000);   // Sound SPI: 1 MHz
  
  // ADC (MCP3008 × 2)
  SPIMCP3008 adc1(SPI_PORT_PANEL, PIN_CS_ADC1);
  SPIMCP3008 adc2(SPI_PORT_PANEL, PIN_CS_ADC2);
  
  // LED (74HC595 × 5)
  SPI74HC595 led_unit_low(SPI_PORT_PANEL, PIN_CS_SR_UNIT_LOW);
  SPI74HC595 led_unit_high(SPI_PORT_PANEL, PIN_CS_SR_UNIT_HIGH);
  SPI74HC595 led_lfo_high_vcf(SPI_PORT_PANEL, PIN_CS_SR_LFO_HIGH_VCF);
  SPI74HC595 led_lfo_low(SPI_PORT_PANEL, PIN_CS_SR_LFO_LOW);
  SPI74HC595 led_vco(SPI_PORT_PANEL, PIN_CS_SR_VCO);
  
  // Switch Selector (Selector クラス)
  Selector sel_vco1(PIN_SW_VCO1_R, PIN_SW_VCO1_L, 4, ...);
  Selector sel_vco2(PIN_SW_VCO2_R, PIN_SW_VCO2_L, 4, ...);
  Selector sel_vcf(PIN_SW_VCF_R, PIN_SW_VCF_L, 2, ...);
  Selector sel_lfo_wave(PIN_SW_LFO_WAVE_R, PIN_SW_LFO_WAVE_L, 6, ...);
  Selector sel_lfo_target(PIN_SW_LFO_TARGET_R, PIN_SW_LFO_TARGET_L, 6, ...);
  
  multicore_launch_core1(main_core1);
  
  while (true) {
    // 1. スイッチ更新
    sel_vco1.update();
    sel_vco2.update();
    sel_vcf.update();
    sel_lfo_wave.update();
    sel_lfo_target.update();
    
    // 2. LED 更新（Unit 状態表示）
    led_unit_high.put_8bit((unit_state & 0xFF) << 1 | (unit_state & 0xFF) >> 7);
    led_unit_low.put_8bit((unit_state >> 8 & 0xFF) << 1 | (unit_state >> 8 & 0xFF) >> 7);
    
    // 3. ストリーム構築
    uint16_t stream[34];
    stream[0] = 0xFFFF;  // ヘッダー
    
    sem_acquire_blocking(&sem);
    for (uint8_t ch = 0; ch < N_UNIT; ch++) {
      stream[ch + 1] = ((unit_state >> ch) & 1) << 15 
                     | unit_note[ch] << 8 
                     | unit_velocity[ch];
    }
    sem_release(&sem);
    
    // モード設定（Word 17）
    stream[17] = sel_vco1.get_state() << 10 
               | sel_vco2.get_state() << 8 
               | sel_vcf.get_state() << 7 
               | sel_lfo_wave.get_state() << 3 
               | sel_lfo_target.get_state();
    
    // パラメータ（Word 18-33）
    stream[18] = adc1.read(2);  // VCO Duty
    stream[19] = adc1.read(3);  // VCO2 Tune
    stream[20] = adc1.read(1);  // VCO Mix
    stream[21] = adc1.read(0);  // VCF CutOff
    stream[22] = adc1.read(7);  // VCF Resonance
    stream[23] = adc1.read(6);  // VCF Attack
    stream[24] = adc1.read(5);  // VCF Decay
    stream[25] = adc2.read(7);  // VCF Sustain
    stream[26] = adc1.read(4);  // VCF Release
    stream[27] = adc2.read(6);  // VCA Attack
    stream[28] = adc2.read(5);  // VCA Decay
    stream[29] = adc2.read(4);  // VCA Sustain
    stream[30] = adc2.read(3);  // VCA Release
    stream[31] = adc2.read(2);  // VCA Gain
    stream[32] = adc2.read(1);  // LFO Speed
    stream[33] = adc2.read(0);  // LFO Depth
    
    // 4. SPI Sound 送信（34 words × 16 bit）
    for (uint8_t i = 0; i < 34; i++) {
      gpio_put(PIN_CS_SOUND, false);
      sleep_us(10);
      spi_write16_blocking(SPI_PORT_SOUND, &stream[i], 1);
      sleep_us(10);
      gpio_put(PIN_CS_SOUND, true);
    }
  }
}
```
  
  Voice* find_and_release(uint8_t key, uint8_t channel) {
    for (int i = 0; i < 8; i++) {
      if (voices[i].active && voices[i].key == key && voices[i].midi_channel == channel) {
        voices[i].active = false;
        return &voices[i];
      }
    }
    return nullptr;
  }
};
```

## マルチコア調整（既存実装）

**Core 1（UART MIDI受信スレッド）：**
- 周期：UART入力駆動（ブロッキング）
- タスク：ASCII hex MIDI パース、Voice Allocation
- 共有変数：`unit_state`, `unit_note[]`, `unit_velocity[]`
- 排他制御：セマフォ（`sem`）による共有変数保護
- メモリ：スタック使用（ローカル変数：バッファ、キュー）

**Core 0（メインループ）：**
- 周期：連続（可能な限り高速）
- タスク：
  1. スイッチ読み込み（Selector.update()）
  2. LED更新（74HC595 × 5個）
  3. ADC読み込み（MCP3008 × 2個、合計16ch）
  4. ストリーム構築（34 words）
  5. SPI送信（Sound modules）
- 共有変数読み込み：セマフォで保護された `unit_state` 等
- 実効レート：約10ms/loop（ADC読み込み + SPI送信時間）

**IPC（Inter-Core Communication）：**
- セマフォ：`sem`（排他制御用）
- 共有メモリ：グローバル変数
  - `unit_state` (16 bit ビットマップ)
  - `unit_note[16]` (各Unitのノート番号)
  - `unit_velocity[16]` (各Unitのベロシティ)
- キュー：`unit_busy`（Core 1内ローカル、Voice Stealing管理用）

## 既知の制限事項

1. **ADC解像度：** MCP3008は10ビット（0-1023）、量子化ノイズあり
2. **Voice Stealing：** 16同時発音を超えると最も古いUnitを強制的に置き換え
3. **Sustain実装：** ペダルOFF時の一括解放処理、リアルタイム性に課題
4. **SPI送信時間：** 68バイト（34 words × 16 bit）送信に約 680µs（1 MHz時）
5. **ADC読み込みオーバーヘッド：** 16ch読み込みで約 16ms（1ch = 1ms想定）

## トラブルシューティング

### 症状：音が出ない

**原因：**
1. Sound ModuleがSPIストリームを正しく受信していない
2. unit_stateのビットが立っていない（Voice Allocationが機能していない）
3. UART MIDIメッセージが届いていない

**確認方法：**
- `printf` でストリーム内容を出力（既存コードに実装済み）
- オシロスコープでSPI信号確認
- UART経由でMIDIメッセージをエコーバック

### 症状：スイッチが反応しない

**原因：**
1. Selector クラ スのGPIOプルアップ設定ミス
2. LEDシフトレジスタのSPI通信エラー

**確認方法：**
- `sel_xxx.get_state()` の値をprintfで出力
- LEDの点灯状態を目視確認

### 症状：パラメータ変更が反映されない

**原因：**
1. MCP3008のSPI通信エラー（CS pin設定ミス）
2. Sound Moduleが該当パラメータを正しく解釈していない

**確認方法：**
- `adc1.read(ch)`, `adc2.read(ch)` の値をprintfで出力
- オシロスコープでパネルSPI信号確認

## 新実装への移行計画

既存実装（old/）を維持しつつ、新しいC++実装（panel/）を段階的に開発：

### Phase 1: 既存プロトコル互換実装（優先）
- [ ] 既存SPIストリーム形式（34 words）を完全再現
- [ ] MCP3008 ADCクラス実装
- [ ] 74HC595シフトレジスタクラス実装
- [ ] Selectorクラス移植
- [ ] Core 1 UART MIDI受信スレッド
- [ ] Core 0 メインループ

### Phase 2: 動作確認
- [ ] 実機でのハード ウェアテスト
- [ ] 既存実装と新実装のストリーム比較
- [ ] 全16unitの同時発音確認
- [ ] 全16VRの動作確認
- [ ] 全5スイッチの動作確認

### Phase 3: リファクタリング（後回し）
- [ ] プロトコル最適化（可能であれば）
- [ ] パフォーマンス改善
- [ ] コード整理

## 参考資料

- 既存実装：`old/pico-synth-panel/`
- MCP3008 Datasheet：10-bit ADC with SPI
- 74HC595 Datasheet：8-bit Shift Register
- Sound Module Interface：`old/pico-synth-sound/`
- Keyboard Module：`keyboard/README.md`

