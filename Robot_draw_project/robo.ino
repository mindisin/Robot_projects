#include <ServoEasing.hpp>

#define DEFAULT_MICROSECONDS_FOR_0_DEGREE 500
#define DEFAULT_MICROSECONDS_FOR_180_DEGREE 2500

const int SERVO_PIN_0 = 2; 
const int SERVO_PIN_1 = 4; 
const int SERVO_PIN_2 = 7; 
const int INITIAL_ANGLE_0 = 65;  
const int DRAWING_ANGLE_0 = 0;  
const int INITIAL_ANGLE_1 = 180; 
const int INITIAL_ANGLE_2 = 0;   

const float L1 = 75.0; 
const float L2 = 65.0; 

static int servoSpeed = 30; 

ServoEasing servo0, servo1, servo2;

void initializeServos() {
    // servo0.detach();
    // servo1.detach();
    // servo2.detach();

    servo0.attach(SERVO_PIN_0, DEFAULT_MICROSECONDS_FOR_0_DEGREE, DEFAULT_MICROSECONDS_FOR_180_DEGREE, 0, 180);
    servo1.attach(SERVO_PIN_1, DEFAULT_MICROSECONDS_FOR_0_DEGREE, DEFAULT_MICROSECONDS_FOR_180_DEGREE, 0, 180);
    servo2.attach(SERVO_PIN_2, DEFAULT_MICROSECONDS_FOR_0_DEGREE, DEFAULT_MICROSECONDS_FOR_180_DEGREE, 0, 180);
    
    servo0.setEasingType(EASE_LINEAR);
    servo1.setEasingType(EASE_LINEAR);
    servo2.setEasingType(EASE_LINEAR);
    
    liftPen();
    servo1.write(INITIAL_ANGLE_1);
    servo2.write(INITIAL_ANGLE_2);
}

void liftPen() {
    servo0.setEaseTo(INITIAL_ANGLE_0);
    Serial.println("Фломастер поднят");
}

void lowerPen() {
    servo0.setEaseTo(DRAWING_ANGLE_0);
    Serial.println("Фломастер опущен");
}

void calculateAngles(float x, float y, float& theta1, float& theta2) {
    float c = sqrt(x * x + y * y);

    if (c > L1 + L2 || c < abs(L1 - L2)) {
        Serial.println("Error: Target point out of reach.");
        return;
    }

    float phi2 = atan2(y, x); 
    float cos_phi1 = (L1 * L1 + c * c - L2 * L2) / (2 * L1 * c); 
    float phi1 = acos(cos_phi1); 

    theta1 = degrees(phi2 + phi1); 
    theta2 = degrees(acos((L1 * L1 + L2 * L2 - c * c) / (2 * L1 * L2))); 

    Serial.print("Рассчитаны углы: theta1=");
    Serial.print(theta1);
    Serial.print(", theta2=");
    Serial.println(theta2);
}

void moveServosLower(float theta1, float theta2) {
    servo1.setEaseTo(theta1);
    servo2.setEaseTo(theta2);
    lowerPen();
    synchronizeAllServosStartAndWaitForAllServosToStop();
    delay(1500);
    Serial.println("Позиция изменена");
}

void moveServosLift(float theta1, float theta2) {
    servo1.setEaseTo(theta1);
    servo2.setEaseTo(theta2);
    liftPen();
    synchronizeAllServosStartAndWaitForAllServosToStop();
    delay(1500);
    Serial.println("Позиция изменена");
}

void parseGCode(String gcode) {
    gcode.trim();

    int firstSpace = gcode.indexOf(' ');
    if (firstSpace == -1) {
        Serial.println("Error: Invalid G-code format.");
        return;
    }

    String command = gcode.substring(0, firstSpace);
    String coords = gcode.substring(firstSpace + 1);

    float x = 0, y = 0;
    if (!extractCoordinates(coords, x, y)) {
        Serial.println("Error: Unable to parse coordinates.");
        return;
    }

    if (command == "G1") {
        float theta1, theta2;
        calculateAngles(x, y, theta1, theta2);
        moveServosLower(theta1, theta2);
    } else if (command == "G0") {
        float theta1, theta2;
        calculateAngles(x, y, theta1, theta2);
        liftPen();
        moveServosLift(theta1, theta2);
    } else {
        Serial.println("Error: Unsupported G-code command.");
        return;
    }

    // Подтверждение выполнения команды
    Serial.println("OK");
}

bool extractCoordinates(String coords, float& x, float& y) {
    int firstSpace = coords.indexOf(' ');
    if (firstSpace == -1) {
        return false;
    }

    String xCoord = coords.substring(0, firstSpace);
    String yCoord = coords.substring(firstSpace + 1);

    x = xCoord.toFloat();
    y = yCoord.toFloat();

    return true;
}

void setup() {
    Serial.begin(9600); 
    initializeServos();
    setSpeedForAllServos(servoSpeed);
    Serial.println("Готов к работе");
}

void loop() {
    if (Serial.available() > 0) {
        String input = Serial.readString();
        Serial.println("Получено: " + input);
        parseGCode(input);
    }
}
