# Panel Module (pico-synth-panel)

Panel モジュールは、Keyboard から受け取った MIDI イベントをもとに 16 ボイス（8 台の Sound Pico × 2 ボイス）の割り当てを管理し、Sound 全台へ音声パラメータを SPI ブロードキャストする中核的な制御ユニットです。


---

## 概要

**主な機能：**
- **UART MIDI 受信**: Keyboard からの MIDI Note On/Off イベント受信（115200 bps）、ノンブロッキング処理
- **Voice Allocation**: 最大 16 ボイスの割り当て管理、Round-robin 割り当て、Voice stealing、Sustain Pedal 対応
- **UI 入力**: 5 種類のセレクタスイッチ（VCO1/VCO2 波形、VCF タイプ、LFO 波形/ターゲット）
- **ADC 入力**: 16 個のポテンショメータから Synth パラメータ値を読み込み（MCP3008 × 2）
- **LED 制御**: 74HC595 シフトレジスタでボイス状態とモード表示
- **SPI 送信**: 8 台の Sound モジュールにパラメータストリームをブロードキャスト

---

## ハードウェア

### プロセッサ
- RP2040（シングルコア実装）

### 通信インターフェース
- **UART**: Keyboard MIDI 受信（115200 bps）
- **SPI Panel (spi0)**: ADC/LED 制御（1 MHz）
- **SPI Sound (spi1)**: Sound ブロードキャスト（1 MHz）

### アナログ入力
- **MCP3008 × 2**: 合計 16 チャンネルのアナログ入力（ポテンショメータ）

### デジタル出力
- **74HC595 × 5**: シフトレジスタ LED 制御

### ユーザーインターフェース
- **スイッチ**: 5 種類 × R/L ボタン = 10 個（GPIO）

### ピン配置

```
UART (MIDI 受信):
  TX: GPIO0 (出力、使用せず)
  RX: GPIO1 (入力：Keyboard から)

SPI Panel (spi0) - ADC/LED 制御:
  SCK:  GPIO18
  MOSI: GPIO19
  MISO: GPIO16
  CS ADC1:         GPIO7
  CS ADC2:         GPIO8
  CS SR Unit Low:  GPIO2
  CS SR LFO VCF:   GPIO3
  CS SR LFO Low:   GPIO4
  CS SR Unit High: GPIO5
  CS SR VCO:       GPIO6

SPI Sound (spi1) - ブロードキャスト:
  SCK:  GPIO14
  MOSI: GPIO15
  MISO: GPIO12
  CS:   GPIO13

UI スイッチ:
  VCO1:      GPIO29 (R), GPIO28 (L)
  VCO2:      GPIO27 (R), GPIO26 (L)
  VCF:       GPIO25 (R), GPIO24 (L)
  LFO Wave:  GPIO23 (R), GPIO22 (L)
  LFO Target:GPIO21 (R), GPIO20 (L)
```

---

### ハードウェア割り当て詳細

#### スイッチ (SW) - UI セレクタ

| スイッチ | GPIO | 機能 | 状態数 | 説明 |
|---------|------|------|--------|------|
| **VCO1** | 29(R), 28(L) | VCO1 波形選択 | 4 | Saw, Sin, Tri, Square |
| **VCO2** | 27(R), 26(L) | VCO2 波形選択 | 4 | Saw, Sin, Tri, Square |
| **VCF** | 25(R), 24(L) | フィルタタイプ | 2 | LPF, HPF |
| **LFO Wave** | 23(R), 22(L) | LFO 波形選択 | 6 | Saw↑, Saw↓, Sin, Tri, Square, Random |
| **LFO Target** | 21(R), 20(L) | LFO 変調ターゲット | 6 | VCO1Pitch, VCO2Duty, VCOMix, VCFCutoff, VCAGain, VCAPan |

**操作**: 右ボタン (R) で選択肢を進む、左ボタン (L) で選択肢を戻す


#### ポテンショメータ (VR) - ADC 入力割り当て

##### ADC1 (MCP3008 #1) - 8チャンネル

| CH | GPIO CS | 機能 | 説明 |
|----|---------|------|------|
| 0 | 7 | **VCF CutOff** | フィルタ遮断周波数（20Hz～20kHz） |
| 1 | 7 | **VCO Mix** | VCO1/VCO2 ミックスバランス |
| 2 | 7 | **VCO Duty** | PWM デューティ（パルス幅調整） |
| 3 | 7 | **VCO2 Tune** | VCO2 チューニング（±1 octave） |
| 4 | 7 | **VCF Release** | フィルタ EG リリースタイム |
| 5 | 7 | **VCF Decay** | フィルタ EG デケイタイム |
| 6 | 7 | **VCF Attack** | フィルタ EG アタックタイム |
| 7 | 7 | **VCF Resonance** | フィルタレゾナンス (Q)「ピーク」 |

##### ADC2 (MCP3008 #2) - 8チャンネル

| CH | GPIO CS | 機能 | 説明 |
|----|---------|------|------|
| 0 | 8 | **LFO Depth** | LFO変調デプス（変調深さ） |
| 1 | 8 | **LFO Speed** | LFO速度（周波数） |
| 2 | 8 | **VCA Gain** | 出力ゲイン（マスターボリューム） |
| 3 | 8 | **VCA Release** | 音量 EG リリースタイム |
| 4 | 8 | **VCA Sustain** | 音量 EG サスティンレベル |
| 5 | 8 | **VCA Decay** | 音量 EG デケイタイム |
| 6 | 8 | **VCA Attack** | 音量 EG アタックタイム |
| 7 | 8 | **VCF Sustain** | フィルタ EG サスティンレベル |


#### LED - シフトレジスタ (74HC595) 割り当て

| シフトレジスタ | GPIO CS | 機能 | LED数 | 説明 |
|---------------|---------|------|-------|------|
| **SR_UNIT_LOW** | 2 | ユニット 0-7 状態表示 | 8 | Unit 0-7 が発音中かを表示 |
| **SR_UNIT_HIGH** | 5 | ユニット 8-15 状態表示 | 8 | Unit 8-15 が発音中かを表示 |
| **SR_VCO** | 6 | VCO1/VCO2 波形 LED | 8 | 選択中の VCO1/VCO2 波形 を表示 |
| **SR_LFO_HIGH_VCF** | 3 | LFO波形・VCFモード LED | 8 | LFO 波形・VCF タイプを表示 |
| **SR_LFO_LOW** | 4 | LFO ターゲット LED | 8 | LFO 変調ターゲットを表示 |

##### 詳細ビット割り当て

###### SR_UNIT_LOW (Pin: CS2, Unit 8-15 状態)
| Bit | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
|-----|---|---|---|---|---|---|---|---|
| Unit| 15| 14| 13| 12| 11| 10| 9 | 8 |

###### SR_UNIT_HIGH (Pin: CS5, Unit 0-7 状態)
| Bit | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
|-----|---|---|---|---|---|---|---|---|
| Unit| 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |

###### SR_VCO (Pin: CS6, VCO1/VCO2 波形選択)
| Bit | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
|-----|---|---|---|---|---|---|---|---|
| 機能| VCO2 Tri | VCO2 Sin | VCO2 Saw | VCO1 Square | VCO1 Tri | VCO1 Sin | VCO1 Saw | VCO2 Square |
| LED Pin| 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |

###### SR_LFO_HIGH_VCF (Pin: CS3, VCF + LFO 波形)
| Bit | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
|-----|---|---|---|---|---|---|---|---|
| 機能| VCF LPF | LFO Random | LFO Square | LFO Tri | LFO Sin | LFO Saw↓ | LFO Saw↑ | VCF HPF |
| 波形| LPF | Random | Square | Tri | Sin | Saw↓ | Saw↑ | HPF |
| LED Pin| 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |

###### SR_LFO_LOW (Pin: CS4, LFO ターゲット)
| Bit | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
|-----|---|---|---|---|---|---|---|---|
| 機能| 予備 | VCAPan | VCAGain | VCFCutoff | VCOMix | VCO2Duty | VCO1Pitch | 予備 |
| ターゲット| - | Pan | Gain | Cutoff | Mix | Duty | Pitch | - |
| LED Pin| 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |

---


## UART MIDI プロトコル
keyboardモジュールからの鍵盤情報の受信は，UART経由のMIDIで行う．

### フォーマット

- **Baud Rate**: 115200 bps
- **形式**: ASCII hex ("90 3c 7f\n")
- **方向**: Receive only

### メッセージタイプ

| メッセージ | Status | Data1 | Data2 | 説明 |
|-----------|--------|-------|-------|------|
| Note On | 0x90 | Note (0-127) | Velocity (1-127) | 鍵盤押下 |
| Note Off | 0x80 | Note (0-127) | 0 | 鍵盤リリース |
| Note Off | 0x90 | Note (0-127) | 0 | Note On with V=0 |
| Sustain Pedal | 0xB0 | 0x40 | 0-127 | CC 64 (≥64 = ON) |

---

## ファイル構成

```
panel/
├── CMakeLists.txt
├── README.md (このファイル)
├── include/
│   ├── config.hpp                # ピン定義、定数
│   ├── spi_device.hpp            # SPI 基底クラス
│   ├── spi_mcp3008.hpp           # ADC (MCP3008) クラス
│   ├── spi_74hc595.hpp           # シフトレジスタ (74HC595) クラス
│   └── selector.hpp              # UI セレクタロジック
└── src/
    └── main.cpp                  # メイン処理（シングルコア）
```

---

## アーキテクチャ
### クラス構造

```mermaid
classDiagram
    direction BT
    
    %% ===== エンベロープジェネレータクラス =====
    class EG {
        -value: float
        -state: EGState
        -attack: float
        -decay: float
        -sustain: float
        -release: float
        -tau: float
        -cycle: uint16_t
        +get_value() float
        +gate_trigger() void
        +gate_off() void
        +set_adsr(a, d, s, r: float) void
    }

    %% ===== パラメータクラス: 機能別データ構造 =====
    class VCO {
        <<enumeration>> VCOWaveType
        -freq: float
        -wavetype1: VCOWaveType
        -wavetype2: VCOWaveType
        -duty: uint8_t
        -pitch_offset: int16_t
        -mix: uint8_t
        +get_freq() float
        +get_wavetype1() WaveType
        +get_wavetype2() WaveType
        +get_duty() uint8_t
        +get_mix() uint8_t
        +set_freq(f: float) void
        +set_waveforms(w1, w2: WaveType) void
    }

    class VCF {
        -filtertype: uint8_t
        -cutoff: float
        -resonance: float
        -eg_int: float
        +get_filtertype() uint8_t
        +get_cutoff() float
        +get_resonance() float
        +get_eg_intensity() float
        +set_filtertype(f: uint8_t) void
        +set_cutoff(c: float) void
    }

    class VCA {
        -gain: float
        +get_gain() float
        +set_gain(g: float) void
    }

    class LFO {
        <<enumeration>> LFOWaveType
        <<enumeration>> LFOTarget
        -wavetype: LFOWaveType
        -target: LFOTarget
        -rate: float
        -depth: float
        -phase: float
        +get_value() float
        +update() void
        +reset() void
        +set_waveform(w: uint8_t) void
        +set_target(t: uint8_t) void
        +set_rate_depth(r, d: float) void
    }

    class SoundUnit {
        -vco: VCO*
        -vcf: VCF*
        -vcf_eg: EG*
        -vca: VCA*
        -vca_eg: EG*
        -lfo: LFO*
        -portamento_tau: float
        -target_freq: float
        -current_freq: float
        -velocity: uint8_t
        +gate_trigger(note: uint8_t, vel: uint8_t) void
        +gate_off() void
        +set_note(note: uint8_t) void
        +set_velocity(v: uint8_t) void
        +update() void
        +serialize(buf: uint16_t*) void
    }

    class SoundModuleManager {
        -units: SoundUnit[16]
        -adc_mgr: ADCManager*
        -sel_vco1: TriStateToggle*
        -sel_vco2: TriStateToggle*
        -sel_vcf: TriStateToggle*
        -sel_lfo_wave: TriStateToggle*
        -sel_lfo_target: TriStateToggle*
        -led_mgr: LEDManager*
        +SoundModuleManager(adc: ADCManager*, toggles[], leds: LEDManager*)
        +update() void
        +get_unit(idx: uint8_t) SoundUnit*
        +serialize(buf: uint16_t*) void
    }

    %% ===== 入力管理クラス =====
    class ADCManager {
        -adc1: SPIMCP3008
        -adc2: SPIMCP3008
        +read_vco(vco: VCO*) void
        +read_vcf(vcf: VCF*) void
        +read_vca(vca: VCA*) void
        +read_lfo(lfo: LFO*) void
    }

    %% ===== 出力管理クラス =====
    class LEDManager {
        -led_unit_low: SPI74HC595
        -led_unit_high: SPI74HC595
        -led_lfo_high_vcf: SPI74HC595
        -led_lfo_low: SPI74HC595
        -led_vco: SPI74HC595
        +update_unit_leds(units: SoundUnit[16]) void
        +update_mode_leds(sel_idx: uint8_t*) void
        +update_all(mgr: SoundModuleManager*) void
    }

    %% ===== 音声処理クラス =====
    class VoiceAllocator {
        -note_to_unit[128]: uint8_t
        -unit_note[16]: uint8_t
        -unit_velocity[16]: uint8_t
        -unit_state: uint16_t
        -unit_sustain: uint16_t
        +handle_note_on(note, velocity: uint8_t) void
        +handle_note_off(note: uint8_t) void
        +apply_to_units(units: SoundUnit[16]) void
    }

    %% ===== ストリーム管理クラス =====
    class ParameterStreamer {
        -sound: SoundModuleManager*
        +build_stream(stream: uint16_t[34]) void
        +send_stream(stream: uint16_t[34]) void
    }

    %% ===== 元のハードウェア抽象化クラス =====
    class SPIDevice {
        #spi_inst_t* spi_
        #uint pin_cs_
        +write_bytes(src: uint8_t*, len: size_t) int
        +write_read_bytes(src: uint8_t*, dst: uint8_t*, len: size_t) int
        +read_bytes(dst: uint8_t*, len: size_t) int
    }

    class SPIMCP3008 {
        +read(channel: uint8_t) uint16_t
    }

    class SPI74HC595 {
        -value_8bit_: uint8_t
        +put_8bit(value: uint8_t) void
        +put(pin: uint8_t, value: bool) void
    }

    class TriStateToggle {
        -pin_sw_r_: uint
        -pin_sw_l_: uint
        -state_: uint8_t
        +update() void
        +get_state() uint8_t
    }

    %% ===== 関係定義 =====
    %% パラメータ集約
    SoundUnit *-- VCO
    SoundUnit *-- VCF
    SoundUnit *-- VCA
    SoundUnit *-- LFO
    SoundUnit *-- EG : vcf_eg
    SoundUnit *-- EG : vca_eg
    SoundModuleManager *-- SoundUnit

    %% SoundModuleManager が各マネージャを統合
    SoundModuleManager --> ADCManager : uses
    SoundModuleManager --> TriStateToggle : reads all 5 toggles
    SoundModuleManager --> LEDManager : controls
    SoundModuleManager --> VoiceAllocator : integrates voice state

    %% ADCManager の依存
    ADCManager --> SPIMCP3008

    %% LEDManager の依存
    LEDManager --> SPI74HC595

    %% ストリーマが SoundModuleManager を使用
    ParameterStreamer --> SoundModuleManager

    %% ハードウェア継承
    SPIDevice <|-- SPIMCP3008
    SPIDevice <|-- SPI74HC595
```

---


### クラス詳細
1. **SoundModuleManager** - 中心的な統合マネージャ
   - SoundUnit[16] を保持・管理
   - ADCManager への参照で ADC値を読み込み
   - 5個の TriStateToggle を直接管理（VCO1/VCO2/VCF/LFO波形/LFOターゲット）
   - LEDManager を制御
   - `update()` メソッドで全入出力を一括処理

2. **SoundUnit** - 各音声ユニット
   - VCO, VCF, VCA, LFO の各パラメータクラスを保持
   - EG (vcf_eg, vca_eg) を内部保持し、2系統のエンベロープを管理
   - gate_trigger() でノート開始、LFOリセット、EGトリガー
   - gate_off() でノート終了、EGリリース
   - update() で LFO更新、ポルタメント処理、EG更新を実行
   - serialize() で SPI送信用データを生成

3. **EG (EnvelopeGenerator)** - ADSR エンベロープジェネレータ
   - SoundUnit が vcf_eg/vca_eg として内部保持
   - gate_trigger() / gate_off() で状態遷移（Ready/Attack/Decay/Sustain/Release）
   - get_value() で現在のエンベロープ値を取得
   - 値域は 0.0-1.0 (float)

4. **VCO** - オシレータ
   - freq（周波数）で管理
   - VCOWaveType Enum で波形を管理 (Saw, Sin, Tri, Square)
   - gain（VCOレベル）、mix（VCO1/2ミックス）、duty（PWM）を管理
   - getter/setterでパラメータアクセス

5. **VCF** - フィルタ
   - float 精度で cutoff_freq, resonance, eg_int 管理
   - filtertype で LPF/HPF 切り替え
   - EG は SoundUnit で管理（VCF自身は持たない）

6. **VCA** - アンプ
   - float 精度で gain 管理
   - pan でステレオパンニング制御 (-1.0=L, 0.0=C, 1.0=R)
   - EG は SoundUnit で管理（VCA自身は持たない）

7. **LFO** - LFO
   - 変調用
   - LFOWave Enum で波形管理 (SawUp, SawDown, Sin, Tri, Square, Random)
   - LFOTarget Enum でターゲット管理 (VCO1Pitch, VCO2Duty, VCOMix, VCFCutoff, VCAGain, VCAPan)
   - update() で波形生成、get_value() で現在値取得
   - reset() で gate_trigger 時に位相リセット

8. **ADCManager** - ADC値読み込み
   - SPI0経由で MCP3008 × 2 から VR値を読み込み
   - VCO, VCF, VCA, LFO パラメータに直接書き込み
   - read_vco(), read_vcf(), read_vca(), read_lfo() メソッド提供

9. **LEDManager** - LED制御
   - 74HC595 × 5 を制御してユニット状態・モード選択を表示
   - SoundModuleManager から呼ばれて LED を更新
   - update_unit_leds(), update_mode_leds(), update_all() メソッド提供

10. **TriStateToggle** - 3ステートトグルスイッチ
    - 右/左ボタンで状態を循環的に切り替え
    - VCO1/VCO2波形、VCFタイプ、LFO波形/ターゲット選択に使用

11. **VoiceAllocator** - Voice allocation
    - MIDI Note On/Off イベントを SoundUnit に割り当て
    - Round-robin 割り当て、Voice stealing、Sustain Pedal 対応
    - handle_note_on/off(), apply_to_units() メソッド提供

12. **ParameterStreamer** - パラメータストリーム生成
    - SoundModuleManager の完全なパラメータセットをシリアライズ
    - 各 SoundUnit の serialize() メソッドを呼び出し
    - 34ワード（68バイト）のストリームを構築
    - SPI1経由で Sound モジュール 8台にブロードキャスト

13. **SPIDevice** - SPI通信基底クラス
    - spi_inst_t と CS ピンを管理
    - write_bytes(), read_bytes(), write_read_bytes() 提供

14. **SPIMCP3008** - ADC (MCP3008) 制御
    - SPIDevice を継承
    - read(channel) で 10-bit ADC値を取得

15. **SPI74HC595** - シフトレジスタ (74HC595) 制御
    - SPIDevice を継承
    - put_8bit(value) で 8-bit 値を一括書き込み
    - put(pin, value) で個別ビット制御


### メインループの流れ

```Core0
while (true) {
    1. handle_uart_midi()
       └─ event_queue.add()
}
```

```Core1
while (true) {
    1. sound_mgr.update()
       ├─ if available: event_queue.get()
       ├─ TriStateToggle.update() × 5  (SW 読み込み)
       ├─ ADCManager.read_*() × 4      (VR 読み込み)
       ├─ SoundUnit.update() × 16      (LFO/Portamento/EG)
       └─ LEDManager.update_all()      (LED 更新)

    2. streamer.send_stream()
       ├─ sound_mgr.serialize()
       └─ SPI1 送信
}
```

---



---

## ビルド

プロジェクトルート (`34_pico_synth/`) で:

```bash
cmake --fresh -S . -B build -G Ninja
ninja -C build pico-synth-panel
```

出力: `build/panel/pico-synth-panel.uf2`

---

