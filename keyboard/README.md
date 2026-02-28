# Keyboard Module

Pico Synthシステムにおけるキーボードスキャンモジュールです。76鍵マトリクスをスキャンし、ノートオン/オフおよびサステインペダルイベントをUSB MIDIとUART MIDIで出力します。

---

## 1. 概要

### 役割
- 76鍵キーボードマトリクスの高速スキャン
- サステインペダル（CC#64）の検出
- USB MIDI Device機能（DAWとの接続）
- UART MIDI送信（Panelモジュールへイベント転送）
- USB→UART MIDIリレー（DAWからの制御をPanelに転送）

### 技術仕様
- **MCU**: Raspberry Pi Pico (RP2040)
- **クロック**: デフォルト（125MHz）
- **マルチコア**: Core0=USB処理、Core1=キーボードスキャン
- **通信**: USB MIDI + UART（115200bps）
- **スキャンレート**: 約10ms周期

---

## 2. ハードウェア構成

### 2.1 ピン配置

| ピン番号 | 機能 | 説明 |
|---------|------|------|
| GP0 | UART TX | Panel向けMIDI送信 |
| GP1 | UART RX | （将来拡張用） |
| GP2-13 | Matrix High (12本) | キーマトリクス高側（出力） |
| GP15 | Pedal Input | サステインペダル（プルアップ） |
| GP16-29 | Matrix Low (14本) | キーマトリクス低側（入力） |

### 2.2 キーマトリクス構成

```
        High側 (出力: GP2-13) - 12本
          ↓
    ┌─────┬─────┬─────┬─────┐
    │ K0  │ K1  │ ... │ K75 │  76鍵
    └─────┴─────┴─────┴─────┘
          ↑
     Low側 (入力: GP16-29) - 14本
```

- **マトリクスサイズ**: 12 × 14 = 168接点（76鍵を配置）
- **スキャン方式**: High側を1本ずつ駆動し、Low側全体を読み取り
- **デバウンス**: ハードウェア回路で対応（ソフトウェア実装なし）

### 2.3 ペダル入力

- **GP15**: サステインペダル（常時監視）
- **回路**: プルアップ抵抗、ペダル踏むとGND接続
- **MIDI出力**: CC#64（value=127でON、0でOFF）

---

## 3. 実装された機能

### 3.1 USB MIDI
- **Device Class**: USB Audio MIDI 1.0
- **VID/PID**: 0xCafe / 自動生成
- **製品名**: "Pico Synth Keyboard"
- **双方向通信**: 鍵盤→DAW + DAW→Panel（リレー機能）

### 3.2 UART MIDI
- **ボーレート**: 115200bps（8N1）
- **プロトコル**: ASCIIヘキサ形式（例: `90 3c 7f\n`）
- **接続先**: Panelモジュール

### 3.3 MIDIメッセージ
| メッセージ | Status | Data1 | Data2 | 説明 |
|-----------|--------|-------|-------|------|
| Note On | 0x90 | Note (0-127) | Velocity (127固定) | 鍵盤押下 |
| Note Off | 0x80 | Note (0-127) | 0 | 鍵盤リリース |
| Control Change | 0xB0 | 64 (Sustain) | 0/127 | ペダルON/OFF |

> **Note**: ベロシティは現在127固定。将来的にスキャンタイミングベースで実装予定。

---

## 4. アーキテクチャ

### 4.1 マルチコア構成

```
┌──────────────────────────────────────────┐
│ RP2040 (pico-synth-keyboard)             │
├──────────────────────────────────────────┤
│                                          │
│  [Core 1]                    [Core 0]   │
│   ┌────────────┐              ┌────────┐│
│   │ Keyboard   │  Queue       │  USB   ││
│   │ Scanner    ├─────────────→│  MIDI  ││
│   │            │ MidiMessage  │  Task  ││
│   │ Pedal      │              │        ││
│   │ Monitor    │              │ UART   ││
│   └────────────┘              │ MIDI   ││
│                               └────────┘│
│                                          │
└──────────────────────────────────────────┘
       ↓ UART (GP0)              ↓ USB
    To Panel                   To DAW
```

### 4.2 データフロー

1. **Core 1**: 10ms周期でキーマトリクススキャン
2. イベント検出 → `MidiMessage`をキューに追加
3. **Core 0**: キューから取り出し → USB + UART両方に送信
4. USB受信データ → UART転送（リレー機能）

---

## 5. ファイル構成

```
keyboard/
├── CMakeLists.txt              # ビルド設定
├── README.md                   # このファイル
├── include/
│   ├── config.hpp              # ピン定義、定数
│   ├── midi_protocol.hpp       # MIDIメッセージ構造体
│   ├── uart_midi.hpp           # UART MIDI送信クラス
│   ├── usb_midi.hpp            # USB MIDIラッパークラス
│   ├── keyboard_scanner.hpp    # マトリクススキャナ
│   └── tusb_config.h           # TinyUSB設定
├── src/
│   ├── main.cpp                # エントリポイント、マルチコア制御
│   ├── uart_midi.cpp           # UART MIDI実装
│   ├── usb_midi.cpp            # USB MIDI実装
│   ├── keyboard_scanner.cpp    # スキャナ実装
│   └── usb_descriptors.c       # USB Device Descriptors
└── build/
    └── pico-synth-keyboard.uf2 # ビルド出力（178KB）
```

---

## 6. ビルド方法

### 6.1 前提条件
- Pico SDK 2.2.0以降
- CMake 3.13以降
- Ninja または Make
- arm-none-eabi-gcc 14.2以降

### 6.2 ビルドコマンド

```bash
# プロジェクトルートから
cd /path/to/34_pico_synth

# CMake設定（初回のみ）
cmake --fresh -G Ninja -B build

# ビルド
ninja -C build pico-synth-keyboard

# 出力: build/keyboard/pico-synth-keyboard.uf2
```

### 6.3 書き込み方法

1. Picoの**BOOTSEL**ボタンを押しながらUSB接続
2. マスストレージデバイスとして認識
3. `pico-synth-keyboard.uf2`をドラッグ&ドロップ
4. 自動的に再起動してプログラム実行

---

## 7. 使用方法

### 7.1 USB MIDI接続（DAWとの接続）

```bash
# Linuxでの確認
aconnect -l
# → "Pico Synth Keyboard" が表示される

# MIDIモニタで動作確認
aseqdump -p "Pico Synth Keyboard"
```

### 7.2 UART MIDI出力確認

```bash
# GP0 (UART TX) をUSB-UART変換器で受信
screen /dev/ttyUSB0 115200
# → 鍵盤を押すと "90 3c 7f" などが表示される
```

### 7.3 ノートマッピング

| 鍵盤番号 | MIDIノート | 音名 |
|---------|-----------|------|
| 0 | 24 | C1 |
| 12 | 36 | C2 |
| 24 | 48 | C3 |
| 36 | 60 | C4 (Middle C) |
| 48 | 72 | C5 |
| 60 | 84 | C6 |
| 75 | 99 | D#7 |

---

## 8. 実装の詳細

### 8.1 KeyboardScanner クラス
- **役割**: GPIO制御とマトリクススキャン
- **主要メソッド**:
  - `void init()`: GPIO初期化
  - `void scan(uint16_t* results)`: High側1本ずつ駆動して読み取り
  - `KeyEvent process_scan(const uint16_t* scan, uint8_t& key_index, uint8_t& velocity)`: 前回スキャン結果と比較してイベント検出

### 8.2 UartMidi クラス
- **役割**: UART経由でASCIIヘキサMIDI送信
- **フォーマット**: `"SS DD DD\n"` (例: `"90 3c 7f\n"`)
- **バッファリング**: FIFOバッファ利用（非ブロッキング）

### 8.3 UsbMidi クラス
- **役割**: TinyUSBのラッパー
- **主要メソッド**:
  - `void init()`: tusb_init(), board_init()
  - `void task()`: tud_task() (定期呼び出し必須)
  - `bool send(MidiMessage)`: tud_midi_stream_write()
  - `bool read(MidiMessage&)`: tud_midi_packet_read()

### 8.4 Inter-core Queue
- **型**: `queue_t` (Pico SDK標準)
- **サイズ**: 32メッセージ
- **データ**: `MidiMessage` 構造体（3バイト）

---

## 9. 既知の制限事項

### 9.1 ベロシティ固定
- 現在、すべてのNote Onはvelocity=127で送信
- 将来的にスキャン間隔を利用した簡易ベロシティ検出を実装予定

### 9.2 ソフトウェアデバウンス未実装
- ハードウェア回路でデバウンス対応
- チャタリングが発生する場合はソフトウェアフィルタ追加が必要

### 9.3 Note Offの遅延
- スキャン周期（10ms）分の遅延あり
- 高速トリルなどで若干のレイテンシ発生の可能性

---

## 10. 今後の実装予定

- [ ] ベロシティ検出（スキャンタイミングベース）
- [ ] ソフトウェアデバウンス（必要に応じて）
- [ ] アフタータッチ対応（ハードウェア変更必要）
- [ ] 追加ペダル入力（Expression, Modulation等）
- [ ] USB MIDI 2.0対応（Pico SDK更新待ち）
- [ ] SysExによる設定変更機能

---

## 11. トラブルシューティング

### 問題: USB MIDIデバイスが認識されない
- **原因**: TinyUSBの初期化失敗、またはUSBケーブル不良
- **対策**: 
  - BOOTSELモードで書き込み直す
  - データ転送対応USBケーブル使用確認
  - `dmesg | grep usb` でエラー確認（Linux）

### 問題: UART MIDIが出力されない
- **原因**: UART未初期化、またはボーレート不一致
- **対策**:
  - GP0ピンが正しく接続されているか確認
  - オシロスコープで波形確認（115200bps = 約8.7μs/bit）
  - Panelモジュールのボーレート設定を確認

### 問題: 鍵盤が反応しない
- **原因**: マトリクス配線ミス、またはGPIO設定エラー
- **対策**:
  - `config.hpp`のPIN_HIGH_MASK, PIN_LOW_MASKを確認
  - マルチメータで各GPIOの動作確認
  - デバッガで`scan_results`配列の値をモニタ

---

## 12. 参考資料

- [Pico SDK Documentation](https://www.raspberrypi.com/documentation/pico-sdk/)
- [TinyUSB MIDI Device Example](https://github.com/hathach/tinyusb/tree/master/examples/device/midi_test)
- [MIDI 1.0 Specification](https://www.midi.org/specifications)
- [RP2040 Datasheet](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)

---

**最終更新**: 2026年3月1日  
**バージョン**: 0.2  
**作成者**: Jun Sotohigashi
