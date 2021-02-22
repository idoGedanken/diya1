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
    int _einFeedback;
    double _lestStep;
    double _motorStuckTime;
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
    _motorStuckTime = 428859761399;
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
    bool isMotorStuck(double time, bool ignore){
        // Serial.println(_pos);
        // Serial.println(_posFeedback);
        //Serial.println(micros()- _motorStuckTime);
        // Serial.println(_motorStuckTime);
        // Serial.println(micros());
        // Serial.println("-------------");
        if(micros() - _motorStuckTime > 1000000)_motorStuckTime = micros();
        return (micros() - _motorStuckTime >=  time && !ignore);
    }
    bool Move(double speeed, double TargetPos) {
    if(abs(TargetPos - _pos) <= 0.0234){
        _motorStuckTime = micros();
        return true;
    }
    if ((micros() - _lestStep) > max((int)(625 / speeed), 150) && TargetPos != _pos) {
        digitalWrite(_motorPin, HIGH);
        digitalWrite(_motorPin, LOW);
        if(_pos != _posFeedback) _motorStuckTime = micros();
        _lestStep =  micros();
    }
    return false;
    }
};

