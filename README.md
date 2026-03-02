# 34_pico_synth

Raspberry Pi Pico (RP2040) を複数台連携させ、電子ピアノを 16 ボイス同時発音のポリフォニックシンセとして動かすプロジェクトです。

既存実装は `old/` に保持しつつ、`keyboard/` と `panel/` を新構成へ移行しています。`sound/` は未移植のため、現在は `old/pico-synth-sound/` をビルドして使用します。

---

## 1. ハードウェア構成

- Raspberry Pi Pico 合計 10 台
- Keyboard: 1 台
- Panel: 1 台
- Sound: 8 台 (1 台あたり 2 ボイス担当)

### 各モジュールの役割

#### Keyboard
- 76 鍵マトリクス走査
- サステインペダル検出
- UART で MIDI 互換イベントを Panel に送信

#### Panel
- UART で Keyboard イベント受信
- 16 ボイスの割り当てと sustain 制御
- ノブ/スイッチ入力の収集 (MCP3008 + GPIO)
- Sound 全台へ SPI ブロードキャスト
- 現在は **シングルコア実装**

#### Sound
- 各 Pico が 2 ボイス分を発音
- SPI 受信データから担当ボイスを抽出
- VCO/VCF/VCA/EG で音声生成し PWM 出力
- 現在は `old/pico-synth-sound/` 実装を利用

---

## 2. 現在のシステム構成

```text
[Keyboard Pico] --UART--> [Panel Pico]
                           |
                           | SPI Broadcast (34 x 16-bit words)
                           +--> [Sound Pico 0] (Voice 1-2)
                           +--> [Sound Pico 1] (Voice 3-4)
                           +--> ...
                           +--> [Sound Pico 7] (Voice 15-16)
```

Panel から Sound へは全ボイス情報を含むストリームをブロードキャストし、各 Sound は自分の担当ユニットのみを処理します。

---

## 3. リポジトリ構成 (現状)

```text
34_pico_synth/
├── CMakeLists.txt
├── README.md
├── keyboard/                  # 新実装
│   ├── include/
│   ├── src/
│   └── README.md
├── panel/                     # 新実装 (single-core)
│   ├── include/
│   ├── src/
│   └── README.md
├── old/
│   ├── pico-synth-keyboard/
│   ├── pico-synth-panel/
│   └── pico-synth-sound/
├── common/                    # 共通化用 (整備中)
└── build/
```

---

## 4. 実装ステータス

- [x] `old/` への旧実装退避
- [x] `keyboard/` 新構成化
- [x] `panel/` 新構成化
- [x] `panel` のシングルコア化 (`panel/src/main.cpp`)
- [ ] `sound/` 新構成化 (現在は `old/pico-synth-sound/` を使用)
- [ ] `common/` プロトコル定義の整理
- [ ] 統合テストと性能評価

---

## 5. ビルド

プロジェクトルートで実行:

```bash
cmake --fresh -S . -B build -G Ninja
ninja -C build
```

個別ターゲット例:

```bash
ninja -C build pico-synth-keyboard
ninja -C build pico-synth-panel
ninja -C build pico-synth-sound0
```

---

## 6. 今後の方針

- Sound モジュールを新構成へ移植
- Panel-Sound 間プロトコルを整理 (将来的にキュー/イベント指向へ拡張)
- DSP コアの共通化 (`common/` / `dsp` 相当)
