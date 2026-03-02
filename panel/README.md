# Panel Module (pico-synth-panel)

Panel モジュールは、Keyboard から受け取った MIDI イベントをもとに 16 ボイス（8 台の Sound Pico × 2 ボイス）の割り当てを管理し、Sound 全台へ音声パラメータを SPI ブロードキャストする中核的な制御ユニットです。

現在は **シングルコア実装** で、UART 受信、Voice Allocation、UI/ADC 入力、SPI 送信をすべてメインループで処理します。

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

---

## ピン配置

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

## アーキテクチャ（現在）

### メインループ フロー

```
while (true) {
    1. handle_uart_midi_nonblocking()
       └─ UART から MIDI メッセージを読み込み
       └─ Voice Allocation （ノンブロッキング）
    
    2. update_selectors()
       └─ UI スイッチ読み込み + LED フィードバック
    
    3. update_unit_leds()
       └─ ボイス状態 LED 更新（74HC595）
    
    4. build_stream()
       └─ ADC 読み込み（MCP3008）
       └─ パラメータストリーム構築（34 words）
    
    5. transmit_stream()
       └─ SPI で Sound 全台にブロードキャスト
}
```

**処理時間**: メインループ 1 回 = 約 15-30ms

### Voice Allocation エンジン

ボイス割り当てはシンプルな Hybrid データ構造で管理：

```cpp
// Forward mapping: Note 番号 → Unit ID
uint8_t g_note_to_unit[128];      // 0xFF = unassigned

// Reverse mapping: Unit ID → Note 番号、使用履歴
uint8_t g_unit_note[16];
uint32_t g_unit_history[16];      // Voice stealing 用タイムスタンプ

// ボイス状態
uint16_t g_unit_state;            // 16-bit bitmap (Unit 0-15)
uint16_t g_unit_sustain;          // Sustain 中の Unit
```

**割り当てロジック:**

1. **Note On**
   - `g_note_to_unit[note]` が有効（既に割り当て済み）→ **Retrigger**: 同じ Unit で再トリガー
   - それ以外 → 空き Unit を Round-robin で探索
   - 空き Unit 見つからず → 最古の Unit を Stealing

2. **Note Off**
   - Sustain OFF → 即座に Unit 解放
   - Sustain ON → RELEASE 状態で保持

3. **Sustain Pedal (CC 64)**
   - Sustain OFF → RELEASE 状態の全 Note を一括解放

---

## パラメータストリーム フォーマット

### ストリーム構造（34 × 16-bit words）

```
Word 0:      0xFFFF（ヘッダー）

Word 1-16:   Unit 0-15 ステータス
  bit 15:    Gate（1=ON, 0=OFF）
  bit 14-8:  Note 番号（0-127）
  bit 7-0:   Velocity（0-127）

Word 17:     モード設定
  bit 11-10: VCO1 波形（0-3: Saw/Sin/Tri/Sq）
  bit 9-8:   VCO2 波形（0-3）
  bit 7:     VCF タイプ（0=LPF, 1=HPF）
  bit 6-3:   LFO 波形（0-5: Saw↑/Saw↓/Sin/Tri/Sq/Random）
  bit 2-0:   LFO ターゲット（0-5: VCO2/VCO1+2/Duty/Mix/Cutoff/Gain）

Word 18-33:  パラメータ値（ADC 入力）
  18: VCO Duty
  19: VCO2 Tune
  20: VCO Mix
  21: VCF CutOff
  22: VCF Resonance (Q)
  23: VCF Attack
  24: VCF Decay
  25: VCF Sustain
  26: VCF Release
  27: VCA Attack
  28: VCA Decay
  29: VCA Sustain
  30: VCA Release
  31: VCA Gain
  32: LFO Speed
  33: LFO Depth
```

---

## UART MIDI プロトコル

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

## ビルド

プロジェクトルート (`34_pico_synth/`) で:

```bash
cmake --fresh -S . -B build -G Ninja
ninja -C build pico-synth-panel
```

出力: `build/panel/pico-synth-panel.uf2`

---

## 今後の展開

- Panel で EG（Envelope Generator）と LFO を計算し、Sound 側の負荷を軽減する設計検討中
- Sound モジュール側でキューベースのメッセージ受信を実装予定（現在はストリーミング方式）
