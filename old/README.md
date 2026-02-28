# Raspberry Pi Pico - Synthesizer Project
## 概要
Raspberry Pi Pico を複数台用いて、高機能なソフトウェアシンセを作るプロジェクト。  
廉価な電子ピアノをアナログシンセサイザーに改造することを目指す。

## ソフトウェア構成
### VCO
- のこぎり波, 正弦波, 三角波, 矩形波
- メインオシレーター(VCO1)とサブオシレーター(VCO2)
- サブオシレーターは周波数の微調整が可能
- 矩形波はDuty調整可能
- VCO1とVCO2のmix
### VCF
- LPF, HPF の2種のフィルター
- CutOff, Resonance
- EnvelopeGenerator (Attack, Decay, Sustain, Release)
### VCA
- Gain
- EnvelopeGenerator (Attack, Decay, Sustain, Release)
### LFO
- のこぎり波(上昇,下降), 正弦波, 三角波, 矩形波, ランダムノイズ
- 周波数と振幅の調整
- 各モジュールに介入する
  - VCO2 周波数
  - VCO1+2 周波数
  - VCO Duty
  - VCO mix
  - VCF CutOff
  - VCA Gain

## ハードウェア構成
外付けの部品は基本的なものを用いて製作する。
- Raspberry Pi Pico * 10台
  - メインコントロール * 1台  
    キーボードの状態に応じて、どの音をどこで生成するか割り振る。
    各種スイッチやつまみを読み取る。
  - 発音用 * 8台  
    1台当たり2音*2chの生成を行う。音声出力はPWMで行う。
  - キーボード用 * 1台  
    電子ピアノのキーボードから、ノート番号とベロシティを読み取る。
- アンプ部  
  各Raspberry Pi PicoからのPWM出力をLPFで平滑化する。
  それらを合成して、電子ピアノ内部のアンプに入力する。
  
