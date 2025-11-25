#ifndef BLUETOOTHTONEPLAYER_H
#define BLUETOOTHTONEPLAYER_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class BluetoothTonePlayer {
public:
  BluetoothTonePlayer(uint8_t rxPin, uint8_t txPin, uint8_t speakerPin, unsigned long inactivityTimeout=0);
  void begin(unsigned long baud=9600);
  void update();
  void playTone(unsigned int freq, unsigned long duration=0);
  void stop();
  void setVolume(uint8_t v); // 0-10
  void addPreset(uint8_t id, const String &melody);
  void playPreset(uint8_t id);

private:
  SoftwareSerial* btSerial;
  uint8_t _rxPin, _txPin, _speakerPin;
  unsigned long _baud;
  uint8_t _volume; // 0-10
  bool _loop;
  unsigned long _lastActivity;
  unsigned long _inactivityTimeout;
  String _buffer;
  void processCommand(const String &cmd);
  void playMelody(const String &melody);
  void feedback(const String &msg);
};

#endif
