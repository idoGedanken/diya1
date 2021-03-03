#ifndef __STEPMOTOR_H
#define __STEPMOTOR_H

#include "Arduino.h"
#include <FastLED.h>
#include <analogWrite.h>
#include <EEPROM.h> //esp
class StepMotor {
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
    void setDir(bool newDir);
   public: 
    StepMotor(int motorPin,int enPin);
    void setOptic(bool opticVal);
    bool getOptic();
    double getPos();
    void setPos(double pos);
    bool getDir();
    bool getDirFeedback();
    void setDirFeedback(bool DirFeedback);
    void readEncoder();
    void calcMotorDir(double TargetPos);
    bool isMotorStuck(int steps=70);
    bool move(double targetPos, double movingSpeed = 3.6);
};

#endif //__STEPMOTOR_H