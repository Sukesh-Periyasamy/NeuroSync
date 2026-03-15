# NeuroSync — EMG-Based Football Game Controller

> Control a 3D football game using nothing but your muscle and nerve signals.
> No keyboard. No controller. Just your body.

![Project Banner](images/banner.jpg)

---

## 📌 Overview

NeuroSync is a real-time neuromuscular game controller built using 5 BioAmp EXG Pill sensors, an Arduino Uno R3 and Unity 3D. It reads electrical signals from muscles across 5 body locations simultaneously, processes them through a custom signal processing pipeline, and translates them into game commands — all in under 200 milliseconds.

The system was built and demoed live at a hackathon in 48 hours. Players controlled a VR football game by flexing their wrists, bending fingers and performing dorsiflexion — scoring actual goals using only their nervous system.

---

## 🎮 Gesture Mapping

| Gesture | Body Movement | Arduino Pin |
|---|---|---|
| LEFT | Left wrist flex (ulnar nerve) | A0 |
| RIGHT | Right wrist flex (ulnar nerve) | A1 |
| KICK | Right leg dorsiflexion (tibialis anterior) | A2 |
| FORWARD | Right index finger bend | A3 |
| BACKWARD | Left index finger press | A4 |

---

## 🔧 Hardware

| Component | Quantity | Purpose |
|---|---|---|
| BioAmp EXG Pill | 5 | Amplify EMG signals ×1000 |
| Arduino Uno R3 | 1 | Read 5 analog channels, process gestures |
| Gel Electrodes | 15 (3 per pill) | Skin contact for signal pickup |
| USB Cable | 1 | Arduino to laptop connection |
| Jumper Wires | 15 | Hardware connections |
| Breadboard | 1 | Power distribution |
| Laptop | 1 | Run Unity 3D game |

**Total hardware cost: under ₹3,000**

---

## 🔌 Wiring

Each BioAmp EXG Pill is individually wired:
```
Each Pill:
  VCC → Arduino 5V
  GND → Arduino GND
  OUT → Arduino A0 / A1 / A2 / A3 / A4

Each Pill has 3 dedicated electrodes:
  IN+  → muscle belly
  IN-  → 2–3 cm away on same muscle
  REF  → nearest bony area (individually connected per pill)
```

---

## 📍 Electrode Placement
```
Pill 1 (A0) — Left wrist:
  IN+  → left forearm muscle
  IN-  → 2cm away same muscle
  REF  → left wrist bone

Pill 2 (A1) — Right wrist:
  IN+  → right forearm muscle
  IN-  → 2cm away same muscle
  REF  → right wrist bone

Pill 3 (A2) — Right shin:
  IN+  → tibialis anterior (front of shin)
  IN-  → 2cm away same muscle
  REF  → ankle bone

Pill 4 (A3) — Right index finger:
  IN+  → finger flexor muscle
  IN-  → 2cm away same muscle
  REF  → knuckle bone

Pill 5 (A4) — Left index finger:
  IN+  → finger flexor muscle
  IN-  → 2cm away same muscle
  REF  → knuckle bone
```

---

## ⚙️ Signal Processing Pipeline
```
Raw EMG (0.01mV)
      ↓
BioAmp EXG Pill amplifies ×1000
      ↓
Arduino reads 0–1023 (10-bit ADC)
      ↓
Step 1: Smoothing filter
        output = previous × 0.8 + new × 0.2
      ↓
Step 2: Rectification
        output = abs(signal − baseline)
      ↓
Step 3: Envelope detection
        instant rise, decay = envelope × 0.85
      ↓
Step 4: Threshold comparison
        gesture classified and sent via Serial
```

---

## 🧠 Algorithms Built From Scratch

1. **Auto Calibration** — measures personal baseline from 300 samples on every startup
2. **Rise-Only Detection** — gesture fires only on rising signal, never on falling
3. **Peak Confirmation** — signal must reach minimum verified peak to confirm real flex
4. **FORWARD Lock Timer** — prevents LEFT/RIGHT from interrupting FORWARD gesture
5. **Rest Confirmation Delay** — signal must stay low before state resets
6. **Dominance Check** — one arm must be significantly stronger than the other for LEFT/RIGHT
7. **Separate Thresholds Per Channel** — each body location individually calibrated
8. **Fast Envelope Decay** — 0.85 decay constant for instant signal clearance between gestures

---

## 💻 Software Stack

| Layer | Technology | Purpose |
|---|---|---|
| Firmware | Arduino C++ | Signal processing and gesture detection |
| Serial | USB 115200 baud | Send commands to PC |
| Game Engine | Unity 3D | 3D football game |
| Unity Script | C# ArduinoEMGInput.cs | Background serial reader |
| Unity Script | C# EMGMovement.cs | Apply movement to player |
| Data Analysis | Python + CSV | Signal recording and threshold calibration |

---

## 📁 Project Structure
```
NeuroSync/
│
├── arduino/
│   └── neuroplay_final.ino        # Main Arduino firmware (5 channels)
│
├── unity/
│   ├── ArduinoEMGInput.cs         # Serial reader background thread
│   └── EMGMovement.cs             # Player movement controller
│
├── python/
│   ├── emg_record.py              # CSV data recorder
│   └── analyse_emg.py             # Threshold analysis script
│
├── data/
│   ├── left_hand_data.csv         # Recorded left hand signals
│   ├── right_hand_data.csv        # Recorded right hand signals
│   ├── kick_data.csv              # Recorded kick signals
│   ├── a3_forward_data.csv        # Recorded forward signals
│   └── a4_backward_data.csv       # Recorded backward signals
│
├── images/
│   ├── hardware_setup.jpg
│   └── demo_photo.jpg
│
└── README.md
```

---

## 🚀 Getting Started

### 1. Hardware Setup
```
1. Stack all 5 BioAmp EXG Pill sensors
2. Wire each pill: VCC → 5V, GND → GND, OUT → A0–A4
3. Attach gel electrodes to body locations as described above
4. Connect Arduino to laptop via USB
```

### 2. Arduino Firmware
```
1. Open Arduino IDE
2. Open arduino/neuroplay_final.ino
3. Select board: Arduino Uno
4. Select correct COM port
5. Upload
6. Open Serial Monitor at 115200 baud
7. Relax all muscles for 3 seconds (calibration)
8. Flex muscles to see gesture output
```

### 3. Python Data Recording (optional — for threshold tuning)
```bash
pip install pyserial
python python/emg_record.py
```

### 4. Unity Integration
```
1. Open your Unity project
2. Add ArduinoEMGInput.cs to an empty GameObject
3. Set Port Name = your COM port (e.g. COM13)
4. Add EMGMovement.cs to your player GameObject
5. Set moveSpeed and turnSpeed in Inspector
6. Press Play
7. Arduino commands now control your player directly
```

---

## 📊 Calibrated Thresholds (from real body data)

These were calibrated from actual recorded EMG data:

| Channel | Body Location | ACT Threshold | Peak Threshold |
|---|---|---|---|
| A0 Left wrist | Ulnar nerve | 30 | 45 |
| A1 Right wrist | Ulnar nerve | 45 | 55 |
| A2 Shin | Tibialis anterior | 20 | 30 |
| A3 Right finger | Flexor | 460 raw | Direction based |
| A4 Left finger | Flexor | 15 | 22 |

> Note: Run auto calibration on every startup. Thresholds may need tuning per person.

---

## 🔄 Serial Output Format

Arduino sends plain text commands via USB Serial at 115200 baud:
```
LEFT
RIGHT
FORWARD
BACKWARD
KICK
```

Unity reads these on a background thread and maps them to player movement.

---

## 🏥 Future Applications

- **Stroke rehabilitation** — game-based motor recovery using EMG biofeedback
- **Nerve disease therapy** — VR games stimulate damaged nerve pathways
- **Prosthetic limb training** — EMG control practice in game environment
- **Hands-free productivity** — video editing, tool control using body signals
- **Assistive technology** — computer access for people with motor disabilities
- **Sports performance** — real-time muscle activation profiling for athletes
- **VR immersion** — full body control in virtual environments

---

## 🛠️ Troubleshooting

| Problem | Cause | Fix |
|---|---|---|
| Flat signal | EMG solder joint not made | Bridge EMG/ECG pads on BioAmp pill |
| False triggers | Threshold too low | Raise ACT threshold by 5 and retest |
| No gesture detected | Threshold too high | Lower ACT threshold by 5 and retest |
| Serial port denied | Serial Monitor open | Close Arduino Serial Monitor before running Python |
| Wrong gesture | Electrode placement off | Reposition IN+ directly on muscle belly |
| Signal too noisy | REF on muscle | Move REF electrode to bony area |

---

## 👥 Team

| Name | Role |
|---|---|
| **You** | Hardware, signal processing, Arduino firmware, Python analysis |
| **Aditya** | Unity 3D football environment and game development |
| **Vishal D** | Hardware assembly and testing |

**Mentors:** Bevraj Suthar, Vignesh Muralidharan

---

## 📚 References

- [BioAmp EXG Pill — Upside Down Labs](https://upsidedownlabs.tech)
- [Arduino Uno R3 Documentation](https://docs.arduino.cc)
- [Unity Scripting API](https://docs.unity3d.com)
- [Electromyography — Wikipedia](https://en.wikipedia.org/wiki/Electromyography)

---

## 📄 License

MIT License — free to use, modify and build upon.

---

## ⭐ Star this repo if NeuroSync inspired you

*Built in 48 hours at a hackathon. This is version 1.*
*The body is the controller.*
