// ═══════════════════════════════════════════════════
//  NeuroPlay — Final EMG Football Controller v3
//  A0 → Left  wrist  → LEFT
//  A1 → Right wrist  → RIGHT
//  A2 → Right shin   → KICK
//  A3 → Right finger → FORWARD
//  A4 → Left  finger → BACKWARD
// ═══════════════════════════════════════════════════

const int LEFT_PIN     = A0;
const int RIGHT_PIN    = A1;
const int KICK_PIN     = A2;
const int FORWARD_PIN  = A3;
const int BACKWARD_PIN = A4;

// ── LEFT / RIGHT thresholds ──
const int ACT_L         = 30;   // lowered — easier LEFT
const int ACT_R         = 45;
const int MIN_PEAK_L    = 45;   // lowered — easier LEFT
const int MIN_PEAK_R    = 55;

// ── KICK thresholds ──
const int KICK_THRESH   = 20;
const int MIN_KICK_PEAK = 30;
const int KICK_DEBOUNCE = 600;

// ── FORWARD threshold ──
const int FORWARD_THRESH   = 460;
const int FORWARD_DEBOUNCE = 300;

// ── BACKWARD thresholds ──
const int ACT_B             = 15;   // raised — less sensitive
const int MIN_PEAK_B        = 22;   // raised — needs real press
const int BACKWARD_DEBOUNCE = 400;
const int BACKWARD_BLOCK_L  = 20;   // block BACKWARD if left wrist above this

// ── LEFT / RIGHT timing ──
const int DEBOUNCE_MS   = 350;
const int GESTURE_DELAY = 150;
const int REST_CONFIRM  = 250;

// ── Signal variables ──
float smoothL  = 0, smoothR  = 0;
float smoothK  = 0, smoothA3 = 0, smoothA4 = 0;
float envL     = 0, envR     = 0;
float envK     = 0, envA4    = 0;
float prevEnvL = 0, prevEnvR = 0;
float prevEnvK = 0, prevEnvA4 = 0;
int   baseL    = 0, baseR    = 0;
int   baseK    = 0, baseA3   = 0, baseA4 = 0;

// ── LEFT / RIGHT state ──
bool forwardLRActive    = false;
bool restTimerStarted   = false;
unsigned long restSince = 0;
bool leftTriggered      = false;
bool rightTriggered     = false;
bool leftPending        = false;
bool rightPending       = false;
unsigned long leftSince  = 0;
unsigned long rightSince = 0;
float peakL = 0, peakR  = 0;
String lastLRAction       = "REST";
unsigned long lastLRPrint = 0;

// ── KICK state ──
bool kickPending   = false;
bool kickTriggered = false;
float peakK        = 0;
unsigned long lastKickTime = 0;

// ── FORWARD state ──
String lastForward         = "REST";
unsigned long lastFwdPrint = 0;

// ── BACKWARD state ──
bool backwardPending   = false;
bool backwardTriggered = false;
float peakA4           = 0;
unsigned long lastBwdPrint = 0;

// ═══════════════════════════════════════════════════
float smoothFilter(float previous, int newVal) {
  return previous * 0.8 + newVal * 0.2;
}

int rectify(float signal, int baseline) {
  int val = abs((int)signal - baseline);
  if (val < 5) val = 0;
  return val;
}

float envelopeFilter(int rectified, float envelope) {
  if (rectified > envelope) return rectified;
  return envelope * 0.85;
}

// ═══════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  Serial.println("=== NeuroPlay EMG Controller v3 ===");
  Serial.println("Relax ALL muscles... calibrating 3 seconds");
  delay(3000);

  long sL=0, sR=0, sK=0, sA3=0, sA4=0;
  for (int i = 0; i < 300; i++) {
    sL  += analogRead(LEFT_PIN);
    sR  += analogRead(RIGHT_PIN);
    sK  += analogRead(KICK_PIN);
    sA3 += analogRead(FORWARD_PIN);
    sA4 += analogRead(BACKWARD_PIN);
    delay(10);
  }

  baseL  = sL  / 300;
  baseR  = sR  / 300;
  baseK  = sK  / 300;
  baseA3 = sA3 / 300;
  baseA4 = sA4 / 300;

  smoothL  = baseL;
  smoothR  = baseR;
  smoothK  = baseK;
  smoothA3 = baseA3;
  smoothA4 = baseA4;

  Serial.print("L=");  Serial.print(baseL);
  Serial.print(" R="); Serial.print(baseR);
  Serial.print(" K="); Serial.print(baseK);
  Serial.print(" F="); Serial.print(baseA3);
  Serial.print(" B="); Serial.println(baseA4);
  Serial.println("Ready! Flex to control.");
  Serial.println("------------------------------");
}

// ═══════════════════════════════════════════════════
void loop() {
  unsigned long now = millis();

  prevEnvL  = envL;
  prevEnvR  = envR;
  prevEnvK  = envK;
  prevEnvA4 = envA4;

  smoothL  = smoothFilter(smoothL,  analogRead(LEFT_PIN));
  smoothR  = smoothFilter(smoothR,  analogRead(RIGHT_PIN));
  smoothK  = smoothFilter(smoothK,  analogRead(KICK_PIN));
  smoothA3 = smoothFilter(smoothA3, analogRead(FORWARD_PIN));
  smoothA4 = smoothFilter(smoothA4, analogRead(BACKWARD_PIN));

  envL = envelopeFilter(rectify(smoothL, baseL), envL);
  envR = envelopeFilter(rectify(smoothR, baseR), envR);

  int rectK = abs((int)smoothK - baseK);
  if (rectK < 3) rectK = 0;
  envK = envelopeFilter(rectK, envK);

  int rectA4 = abs((int)smoothA4 - baseA4);
  if (rectA4 < 2) rectA4 = 0;
  envA4 = envelopeFilter(rectA4, envA4);

  // ════════════════════════════
  // KICK
  // ════════════════════════════
  if (envK > peakK) peakK = envK;

  if (envK < KICK_THRESH) {
    kickTriggered = false;
    kickPending   = false;
    peakK         = 0;
  }

  if (!kickTriggered && envK > KICK_THRESH && envK >= prevEnvK) {
    if (!kickPending) { kickPending = true; peakK = 0; }
  }

  if (kickPending && peakK > MIN_KICK_PEAK &&
      (now - lastKickTime) > KICK_DEBOUNCE) {
    Serial.println("KICK");
    kickTriggered = true;
    kickPending   = false;
    lastKickTime  = now;
    peakK         = 0;
  }

  // ════════════════════════════
  // FORWARD
  // ════════════════════════════
  int valA3        = (int)smoothA3;
  String fwdAction = (valA3 < FORWARD_THRESH) ? "FORWARD" : "REST";

  if (fwdAction != lastForward &&
      (now - lastFwdPrint) > FORWARD_DEBOUNCE) {
    if (fwdAction != "REST") Serial.println("FORWARD");
    lastForward  = fwdAction;
    lastFwdPrint = now;
  }

  // ════════════════════════════
  // BACKWARD
  // ════════════════════════════
  if (envA4 > peakA4) peakA4 = envA4;

  if (envA4 < ACT_B) {
    backwardTriggered = false;
    backwardPending   = false;
    peakA4            = 0;
  }

  // Only start pending if left wrist is completely quiet
  if (!backwardTriggered &&
      envA4 > ACT_B &&
      envA4 >= prevEnvA4 &&
      envL < BACKWARD_BLOCK_L) {
    if (!backwardPending) {
      backwardPending = true;
      peakA4          = 0;
    }
  }

  // Cancel if left wrist activates at all
  if (backwardPending && envL > BACKWARD_BLOCK_L) {
    backwardPending = false;
    peakA4          = 0;
  }

  if (backwardPending && peakA4 > MIN_PEAK_B &&
      (now - lastBwdPrint) > BACKWARD_DEBOUNCE) {
    Serial.println("BACKWARD");
    backwardTriggered = true;
    backwardPending   = false;
    lastBwdPrint      = now;
    peakA4            = 0;
  }

  // ════════════════════════════
  // LEFT / RIGHT
  // ════════════════════════════
  if (envL < ACT_L) { leftTriggered  = false; peakL = 0; }
  if (envR < ACT_R) { rightTriggered = false; peakR = 0; }
  if (envL > peakL)   peakL = envL;
  if (envR > peakR)   peakR = envR;

  if (envL > ACT_L && envR > ACT_R) {
    if (!forwardLRActive) {
      forwardLRActive  = true;
      restTimerStarted = false;
      leftPending      = false;
      rightPending     = false;
      leftTriggered    = true;
      rightTriggered   = true;
      peakL = 0; peakR = 0;
    }
  }

  if (forwardLRActive && envL < ACT_L && envR < ACT_R) {
    if (!restTimerStarted) {
      restSince        = now;
      restTimerStarted = true;
    }
    if (now - restSince > REST_CONFIRM) {
      forwardLRActive  = false;
      restTimerStarted = false;
      lastLRAction     = "REST";
    }
  }

  if (forwardLRActive && restTimerStarted &&
      (envL > ACT_L || envR > ACT_R)) {
    restTimerStarted = false;
  }

  if (!forwardLRActive && !leftTriggered &&
      envL > ACT_L && envR < ACT_R && envL > prevEnvL) {
    if (!leftPending) {
      leftPending = true;
      leftSince   = now;
      peakL       = 0;
    }
  } else if (envL < ACT_L) {
    leftPending = false;
  }

  if (!forwardLRActive && !rightTriggered &&
      envR > ACT_R && envL < ACT_L && envR > prevEnvR) {
    if (!rightPending) {
      rightPending = true;
      rightSince   = now;
      peakR        = 0;
    }
  } else if (envR < ACT_R) {
    rightPending = false;
  }

  String lrAction = "REST";

  if (leftPending &&
      (now - leftSince) > GESTURE_DELAY &&
      peakL > MIN_PEAK_L) {
    lrAction      = "LEFT";
    leftTriggered = true;
    leftPending   = false;
  }
  else if (rightPending &&
           (now - rightSince) > GESTURE_DELAY &&
           peakR > MIN_PEAK_R) {
    lrAction       = "RIGHT";
    rightTriggered = true;
    rightPending   = false;
  }

  if (lrAction != lastLRAction &&
      (now - lastLRPrint) > DEBOUNCE_MS) {
    if (lrAction != "REST") Serial.println(lrAction);
    lastLRAction = lrAction;
    lastLRPrint  = now;
  }

  delay(20);
}