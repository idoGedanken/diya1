#include "buttons.h"
bool stepMotorTest(StepMotor &motor){
    readSensors();
    if(!motor.getOptic()){
        Serial.println("motor optic eror");
        return false;
    }
    while(!moveStep(motor,2.5,20)){
        EVERY_N_MILLISECONDS( 100 ) {
            readSensors();
        }
        if(motor.isMotorStuck(-1) && motor.getPos() > 2){
            Serial.println("d encoder/mechanical eror");
            return false;
        }
    }
    while(!moveStep(motor,2.5,0)){
        EVERY_N_MILLISECONDS(100){
            readSensors();
        }
        if(motor.isMotorStuck()){
        Serial.println("u encoder/mechanical eror");
        return false;
        }
    }
    motor.setPos(0.2);//toleranc
    while(!moveStep(motor,2.5,0)){
    readSensors();
    };

    if(!motor.getOptic()){
        Serial.println("encoder eror");
        return false;
    }
    return true;
}