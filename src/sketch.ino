#include <ZumoBuzzer.h>
#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <ZumoMotors.h>
#include <Pushbutton.h>

/* Speeds, in motors unit */
#define ULTRAFAST 400
#define FAST 300
#define SLOW 200

/* Motor times, in ms */
#define FORWARD 500
#define CHARGE 50
#define BACK 100
#define TURN 543

/* Reflectance sensors threshold for white/black decision */
#define QTHRES 500

/* IR distance sensors */
#define FRONT_SENSOR 2
#define REAR_SENSOR  3

/* Activate beeps */
bool silent = false;

#define SAY(note) if (! silent) buzzer.play("L32 MS " note)

ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);
ZumoBuzzer buzzer;

ZumoReflectanceSensorArray reflectArray(QTR_NO_EMITTER_PIN);
unsigned int sensors[6];
int left, right;

int distance(int pin){
    return 67870.0/(analogRead(pin)-3.0)-40.0;
}

/* Return true if there is something in the front line of sight */
bool inFront(){
    int d = distance(FRONT_SENSOR);
    return d > 0 && d < 200; //mm
}

/* Return true if there is something in the rear line of sight */
bool inBack(){
    int d = distance(REAR_SENSOR);
    return d > 0 && d < 100; //mm
}

long unsigned int now, tlast = 0;

void setup(){
    button.waitForButton();
    for (int i=0; i<5; i++){
        SAY("F#");
        delay(1000);
    }
}

/*
 * Notes:
 * - A: Border detected
 * - C: Random direction choosed
 * - E: Back detection
 * - G: Front detection
 */

void loop(){
    now = millis();
    reflectArray.read(sensors);
    
    if (sensors[0] < QTHRES){
        /* Border on left: go back then turn right */
        SAY("A");
        motors.setSpeeds(-FAST, -FAST);
        delay(BACK);
        while (millis()-now < TURN && ! inFront())
            motors.setSpeeds(FAST, -FAST);
        motors.setSpeeds(FAST, SLOW);
        tlast = now;
    }

    else if (sensors[5] < QTHRES){
        /* Border on left: go back then turn right */
        SAY("A");
        motors.setSpeeds(-FAST, -FAST);
        delay(BACK);
        while (millis()-now < TURN && ! inFront())
            motors.setSpeeds(-FAST, FAST);
        motors.setSpeeds(SLOW, FAST);
        tlast = now;
    }
    
    else if (inBack()){
        /* Someone behind; face him */
        SAY("E");
        while (! inFront() && millis() - now < 1000)
            motors.setSpeeds(ULTRAFAST, -ULTRAFAST);
    }

    else if (inFront()){
        /* Someone in front; charge */
        SAY("G");
        motors.setSpeeds(ULTRAFAST, ULTRAFAST);
        delay(CHARGE);
    }

    else {
        /* Nothing in front, walk randomly on the pitch */
        if (now - tlast > FORWARD){
            SAY("C");
            int n = rand()%3;
            switch (n){
                case 1:
                    motors.setSpeeds(SLOW, FAST); 
                    break;
                case 2: 
                    motors.setSpeeds(FAST, SLOW); 
                    break;
                default:
                    motors.setSpeeds(FAST, FAST); 
                    break;
            }
            tlast = now;
        }
    }
}
