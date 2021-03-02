#include "Arduino.h"
#include <FastLED.h>
#include <analogWrite.h>
#include <EEPROM.h> //esp


class StepMotor{
  private:
    double _pos;
    double _posFeedback;
    int _motorPin;
    int _enPin;
    unsigned long _lestStep;
    unsigned long _motorStuck;
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
    _posFeedback = 1;
    _motorPin = motorPin;
    _enPin = enPin;
    _lestStep = micros();
    _motorStuck = 0;
    }
    void setOptic(bool opticVal){_motorOptic = opticVal;}
    bool getOptic(){return _motorOptic;}
    double getPos(){return _pos;}
    void setPos(double pos){_pos = pos;}
    bool getDir(){return _dir;}
    bool getDirFeedback(){return _dirFeedback;}
    void setDirFeedback(bool DirFeedback){_dirFeedback = DirFeedback;}
    void readEncoder() {
        _posFeedback = _pos;
        if (_motorOptic){
            _pos = 0;
            return;
        }
        bool encoderRead  = digitalRead(_enPin);
        if ( encoderRead != _encoderFeedback) {
            _encoderFeedback = encoderRead;
            _motorStuck = 0;
            if (_dir) _pos += 0.0234;
            else _pos -= 0.0234;
            return;
        }
        _encoderFeedback = encoderRead;
        }
    void calcMotorDir(double TargetPos){
        if (TargetPos < _pos)setDir(false);
        else if (TargetPos > _pos)setDir(true);      
    }
    bool isMotorStuck(int steps=70){
        return _motorStuck > steps;
    }
    bool move(double TargetPos, double speeed = 3.6) { 
    if(abs(TargetPos - _pos) <= 0.0234 ){
        return true;
    }
    if(isMotorStuck(1000)){
        Serial.println("stuck encoder eror");
        return true;
    }
    if ((micros() - _lestStep) > max((int)(625 / max(0.0 ,speeed)), 175)) {
        digitalWrite(_motorPin, HIGH);
        digitalWrite(_motorPin, LOW);
        _motorStuck ++;
        _lestStep =  micros();
    }
    return false;
    }
};

