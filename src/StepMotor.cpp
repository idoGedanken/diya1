#include "Arduino.h"
#include <FastLED.h>
#include <analogWrite.h>
#include <EEPROM.h> //esp


class StepMotor{
  private:
    double _pos;
    int _motorPin;
    int _enPin;
    int _einFeedback;
    unsigned long _lestStep;
    bool _dir = true;
    bool _dirFeedback = false;
    bool _encoderFeedback;
    bool _motorOptic;
    void setDir(bool newDir){_dir = newDir;}
   public: 
    StepMotor(int motorPin,int enPin){
    pinMode(enPin, INPUT_PULLUP);
    pinMode(motorPin, OUTPUT);
    _pos = 0;
    _motorPin = motorPin;
    _enPin = enPin;
    _lestStep = micros();
    }
    void setOptic(bool opticVal){_motorOptic = opticVal;}
    bool getOptic(){return _motorOptic;}
    double getPos(){return _pos;}
    void setPos(double pos){_pos = pos;}
    bool getDir(){return _dir;}
    bool getDirFeedback(){return _dirFeedback;}
    void setDirFeedback(bool DirFeedback){_dirFeedback = DirFeedback;}
    void readEncoder() {
    if (_motorOptic){
        _pos = 0;
        return;
    }
    bool encoderRead  = digitalRead(_enPin);
    if ( encoderRead != _encoderFeedback) {
        _encoderFeedback = encoderRead;
        if (_dir) _pos += 0.0234;
        else _pos -= 0.0234;
        return;
    }
    _encoderFeedback = encoderRead;
    }
    void culcMotorDir(double TargetPos){
        if (TargetPos < _pos)setDir(false);
        else if (TargetPos > _pos)setDir(true);      
    }
    bool Move(double speeed, double TargetPos) {
    if(abs(TargetPos - _pos) <= 0.0234)return true;
    if ((micros() - _lestStep) > max((int)(625 / speeed), 150) && TargetPos != _pos) {
        digitalWrite(_motorPin, HIGH);
        digitalWrite(_motorPin, LOW);
        _lestStep =  micros();
    }
    return false;
    }
};

