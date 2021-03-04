#include "StepMotor.h"

StepMotor::StepMotor(int motorPin,int enPin)
{
    pinMode(enPin, INPUT_PULLUP);
    pinMode(motorPin, OUTPUT);
    _pos = 0;
    _posFeedback = 1;
    _motorPin = motorPin;
    _enPin = enPin;
    _lestStep = micros();
    _motorStuck = 0;
}
void StepMotor::setDir(bool newDir){_dir = newDir;}
void StepMotor::setOptic(bool opticVal){_motorOptic = opticVal;}
bool StepMotor::getOptic(){return _motorOptic;}
double StepMotor::getPos(){return _pos;}
void StepMotor::setPos(double pos){_pos = pos;}
bool StepMotor::getDir(){return _dir;}
bool StepMotor::getDirFeedback(){return _dirFeedback;}
void StepMotor::setDirFeedback(bool DirFeedback){_dirFeedback = DirFeedback;}
void StepMotor::readEncoder() {
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
void StepMotor::calcMotorDir(double TargetPos){
    if (TargetPos < _pos)setDir(false);
    else if (TargetPos > _pos)setDir(true);      
}
bool StepMotor::isMotorStuck(int steps){
    return _motorStuck > steps;
}
bool StepMotor::move(double targetPos, double movingSpeed) { 
    if(abs(targetPos - _pos) <= 0.0234 ){
        return true;
    }
    if(isMotorStuck(1600)){
        //Serial.println("stuck encoder eror");
        return true;
    }
    if ((micros() - _lestStep) > max((int)(625 / max(0.5,movingSpeed)), 175)) {
        digitalWrite(_motorPin, HIGH);
        digitalWrite(_motorPin, LOW);
        _motorStuck ++;
        _lestStep =  micros();
    }
    return false;
}

