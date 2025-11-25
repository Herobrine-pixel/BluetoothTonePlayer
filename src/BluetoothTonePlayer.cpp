#include "BluetoothTonePlayer.h"
#include <EEPROM.h>

BluetoothTonePlayer::BluetoothTonePlayer(uint8_t rxPin, uint8_t txPin, uint8_t speakerPin, unsigned long inactivityTimeout)
: _rxPin(rxPin), _txPin(txPin), _speakerPin(speakerPin), _baud(9600), _volume(8), _loop(false),
  _lastActivity(0), _inactivityTimeout(inactivityTimeout)
{
  btSerial = new SoftwareSerial(_rxPin, _txPin);
}

void BluetoothTonePlayer::begin(unsigned long baud){
  _baud = baud;
  btSerial->begin(_baud);
  pinMode(_speakerPin, OUTPUT);
  digitalWrite(_speakerPin, LOW);
  _lastActivity = millis();
}

void BluetoothTonePlayer::update(){
  while(btSerial->available()){
    char c = btSerial->read();
    if(c == '\r') continue;
    if(c == '\n'){
      String cmd = _buffer;
      _buffer = "";
      if(cmd.length()>0) processCommand(cmd);
      _lastActivity = millis();
    } else {
      _buffer += c;
      if(_buffer.length()>200) _buffer = _buffer.substring(_buffer.length()-200); // keep buffer small
    }
  }

  if(_inactivityTimeout>0 && millis() - _lastActivity > _inactivityTimeout){
    stop();
    _lastActivity = millis();
  }
}

// Basic tone player - duration=0 means indefinite (until STOP)
void BluetoothTonePlayer::playTone(unsigned int freq, unsigned long duration){
  // simulate volume by PWM on PWM-capable pins (analogWrite)
  // if speaker pin not PWM, tone() still works but volume control won't
  if(duration==0){
    tone(_speakerPin, freq);
  } else {
    tone(_speakerPin, freq, duration);
  }
  feedback("PLAYING TONE " + String(freq) + "Hz " + String(duration) + "ms");
}

// stop any tone
void BluetoothTonePlayer::stop(){
  noTone(_speakerPin);
  feedback("STOPPED");
}

void BluetoothTonePlayer::setVolume(uint8_t v){
  if(v>10) v=10;
  _volume = v;
  feedback("VOLUME " + String(_volume));
}

// Save simple presets in EEPROM: address 0..99 (we store length then bytes)
void BluetoothTonePlayer::addPreset(uint8_t id, const String &melody){
  if(id==0 || id>10) { feedback("PRESET ID 1-10"); return; }
  int addr = (id-1)*50; // max 50 bytes per preset
  int len = min((int)melody.length(), 49);
  EEPROM.update(addr, len);
  for(int i=0;i<len;i++) EEPROM.update(addr+1+i, melody[i]);
  feedback("SAVED PRESET " + String(id));
}

void BluetoothTonePlayer::playPreset(uint8_t id){
  if(id==0 || id>10) { feedback("PRESET ID 1-10"); return; }
  int addr = (id-1)*50;
  int len = EEPROM.read(addr);
  String melody;
  for(int i=0;i<len;i++){
    melody += char(EEPROM.read(addr+1+i));
  }
  playMelody(melody);
  feedback("PLAYING PRESET " + String(id));
}

// Very simple melody parser: notes separated by commas; supports pitch names (C4) or frequencies.
// Example: "C4,500;D4,500;E4,1000" (note,duration)
void BluetoothTonePlayer::playMelody(const String &melody){
  // parse entries separated by ';' or ','
  int i=0;
  int L = melody.length();
  String token;
  while(i < L){
    char c = melody[i];
    if(c==';'){
      if(token.length()>0){
        // token may be "C4,500" or "440,500" or "FX:ALARM"
        if(token.startsWith("FX:")){
          // do some built-ins
          String fx = token.substring(3);
          if(fx=="BEEP"){
            tone(_speakerPin,1000,100);
            delay(150);
          } else if(fx=="ALARM"){
            for(int k=0;k<3;k++){ tone(_speakerPin,900,200); delay(250); }
          } else if(fx=="UP"){
            for(int f=400; f<1200; f+=100){ tone(_speakerPin,f,60); delay(80); }
          } else if(fx=="DOWN"){
            for(int f=1200; f>400; f-=100){ tone(_speakerPin,f,60); delay(80); }
          }
        } else {
          int comma = token.indexOf(',');
          if(comma>0){
            String note = token.substring(0,comma);
            String durS = token.substring(comma+1);
            int dur = durS.toInt();
            int freq = note.toInt();
            if(freq==0){
              // try note name mapping
              // very small map:
              struct { const char *n; int f; } map[] = {{"C4",262},{"D4",294},{"E4",330},{"F4",349},{"G4",392},{"A4",440},{"B4",494},{"C5",523}};
              for(auto &m: map) if(note==m.n) freq = m.f;
            }
            if(freq>0){
              tone(_speakerPin, freq, dur);
              delay(dur + 20);
            }
          }
        }
      }
      token = "";
    } else {
      token += c;
    }
    i++;
  }
  // last token (if no trailing ;)
  if(token.length()>0){
    // reuse above logic
    if(token.startsWith("FX:")){
      String fx = token.substring(3);
      if(fx=="BEEP"){ tone(_speakerPin,1000,100); delay(150); }
      else if(fx=="ALARM"){ for(int k=0;k<3;k++){ tone(_speakerPin,900,200); delay(250); } }
    } else {
      int comma = token.indexOf(',');
      if(comma>0){
        String note = token.substring(0,comma);
        String durS = token.substring(comma+1);
        int dur = durS.toInt();
        int freq = note.toInt();
        if(freq==0){
          struct { const char *n; int f; } map[] = {{"C4",262},{"D4",294},{"E4",330},{"F4",349},{"G4",392},{"A4",440},{"B4",494},{"C5",523}};
          for(auto &m: map) if(note==m.n) freq = m.f;
        }
        if(freq>0){ tone(_speakerPin, freq, dur); delay(dur + 20); }
      }
    }
  }
}

void BluetoothTonePlayer::processCommand(const String &cmd){
  String c = cmd;
  c.trim();
  c.toUpperCase();
  // feedback
  feedback("CMD: " + c);
  if(c.startsWith("TONE:")){
    // TONE:440 or TONE:440:500
    int parts[2] = {0,0};
    int p1 = c.indexOf(':',5);
    if(p1>0){
      String f = c.substring(5,p1);
      String d = c.substring(p1+1);
      unsigned int freq = (unsigned int)f.toInt();
      unsigned long dur = (unsigned long)d.toInt();
      playTone(freq, dur);
    } else {
      unsigned int freq = (unsigned int)c.substring(5).toInt();
      playTone(freq, 0);
    }
  } else if(c.startsWith("MELODY:")){
    String m = cmd.substring(7); // keep original casing for note names
    playMelody(m + ";");
  } else if(c.startsWith("FX:")){
    String fx = c.substring(3);
    playMelody("FX:" + fx + ";");
  } else if(c.startsWith("STOP")){
    stop();
  } else if(c.startsWith("VOLUME:")){
    uint8_t v = (uint8_t)c.substring(7).toInt();
    setVolume(v);
  } else if(c.startsWith("LOOP:")){
    String s = c.substring(5);
    if(s.indexOf("ON")>=0) _loop = true;
    else _loop = false;
    feedback(String("LOOP ") + (_loop? "ON": "OFF"));
  } else if(c.startsWith("SAVE:")){
    // SAVE:id:melody
    int p = c.indexOf(':',5);
    if(p>0){
      uint8_t id = c.substring(5,p).toInt();
      String melody = cmd.substring(p+1);
      addPreset(id, melody);
    }
  } else if(c.startsWith("PLAY:")){
    uint8_t id = c.substring(5).toInt();
    playPreset(id);
  } else {
    feedback("UNKNOWN");
  }
}

void BluetoothTonePlayer::feedback(const String &msg){
  // print feedback to both bt serial and hardware Serial for debugging
  if(btSerial) btSerial->println(msg);
  Serial.println(msg);
}
