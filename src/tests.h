#include "buttons.h"
bool stepMotorTestDir(StepMotor &motor){
    while(!moveStep(motor,2.5,5)){
    }
}
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
bool OpticSensorTest(bool sensor){
    if(!sensor){
        return false;
    }
    return true;
}
bool RFTest(){
    if(!mfrc522.PICC_IsNewCardPresent() && !mfrc522.PICC_ReadCardSerial()){
        return false;
    }
    return true;
}
void test(){
    int dataLenght = 0;
    char* temp = new char[20];
    bool trayClosedf = trayClosed;
    bool trayOpenf = trayOpen;
    bool capsuleInterface1f = capsuleInterface1;
    bool capsuleInterface2f = capsuleInterface2;
    while(1){
        delay(50);
        readSensors();
        if( Serial.available()){
           while (Serial.available()){
                delay(2);
                temp[dataLenght] = Serial.read();
                dataLenght++;
            }
            temp[dataLenght]='\0';
            dataLenght = 0;
            if (String(temp).equals("done"))return;
            if (String(temp).equals("Stiring")){
                spinMixser(200);
                delay(500);
                spinMixser(0);
            }
            if (trayOpenf != trayOpen){
                if(trayOpen) Serial.print("trayOpen1");
                else Serial.print("trayOpen0");
                }
            if (trayClosedf != trayClosed){
                if(trayClosed) Serial.print("trayClosed1");
                else Serial.print("trayClosed0");
                }
            if (String(temp).equals("closeTray")){
                analogWrite(TRAY_DIR_OUT,0);
                analogWrite(TRAY_DIR_IN,255);
                delay(250);
                analogWrite(TRAY_DIR_OUT,0);
                analogWrite(TRAY_DIR_IN,0);
                }
            if (String(temp).equals("RFid")){
                if(RFTest()) Serial.print("RFid1");
                else Serial.print("RFid0");
                }
            if(String(temp).equals("peripheralDown")){
                spinMixser(200);
                delay(500);
                spinMixser(0);
            }
            trayClosedf = trayClosed;
            trayOpenf = trayOpen;
            capsuleInterface1f = capsuleInterface1;
            capsuleInterface2f = capsuleInterface2;
            readSensors();
            temp = new char[20];
        }
    }
}