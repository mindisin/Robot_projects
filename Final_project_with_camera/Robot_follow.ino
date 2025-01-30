#include <ServoEasing.hpp>

// Константы для сервопривода
const int SERVO_PIN = 2;
const int INITIAL_ANGLE = 90;  // Начальный угол сервопривода

// Константы для фильтрации
const float FILTER_FACTOR = 0.2;  // Коэффициент фильтрации (0.0 - 1.0)
                                 // Меньше значение = более плавное движение
                                 // Больше значение = более быстрая реакция
const int MIN_MOVE_THRESHOLD = 1;  // Минимальное изменение угла для движения

// Переменные для хранения диапазона координат
int minX = 99999;  // Начальное значение для определения минимума
int maxX = -99999; // Начальное значение для определения максимума

// Переменная для хранения отфильтрованного угла
float filteredAngle = INITIAL_ANGLE;

// Длина звена манипулятора
const float L1 = 75.0;  // мм

ServoEasing servo;
static int servoSpeed = 10;  // Уменьшаем скорость для плавности

void setup() {
    Serial.begin(9600);
    
    // Инициализация сервопривода
    servo.attach(SERVO_PIN, 500, 2500, 0, 180);
    servo.write(INITIAL_ANGLE);
    servo.setSpeed(servoSpeed);
    
    Serial.println("Servo initialized with low pass filter");
}

void loop() {
    if (Serial.available() > 0) {
        // Читаем строку с координатами
        String input = Serial.readStringUntil('\n');
        
        // Парсим координаты
        int comma = input.indexOf(',');
        if (comma != -1) {
            int x = input.substring(0, comma).toInt();
            int y = input.substring(comma + 1).toInt();
            
            // Обновляем диапазон
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            
            // Вычисляем угол
            float theta;
            calculateAngles(x, y, theta);
            
            // Фильтруем угол
            filteredAngle = (FILTER_FACTOR * theta) + ((1 - FILTER_FACTOR) * filteredAngle);
            int newAngle = round(filteredAngle);
            
            // Проверяем, достаточно ли большое изменение для движения
            static int lastAngle = INITIAL_ANGLE;
            int angleDifference = abs(newAngle - lastAngle);

            // Уменьшаем минимальное пороговое значение для более плавного движения
            if (angleDifference >= MIN_MOVE_THRESHOLD) {
                if (servo.getCurrentAngle() != newAngle) {
                    servo.startEaseTo(newAngle);
                }
                lastAngle = newAngle;
            }
            
            // Выводим отладочную информацию
            Serial.print("X: ");
            Serial.print(x);
            Serial.print(" Y: ");
            Serial.print(y);
            Serial.print(" Theta: ");
            Serial.print(theta);
            Serial.print(" Filtered Angle: ");
            Serial.println(newAngle);
        }
    }
}

// Функция вычисления угла для одного звена
void calculateAngles(float x, float y, float& theta) {
    theta = atan2(y, x);  // Вычисляем угол в радианах
    theta = degrees(theta); // Переводим угол в градусы

    // Ограничиваем угол в пределах 0-180 градусов
    theta = constrain(theta, 0, 180);

    Serial.print("X: ");
    Serial.print(x);
    Serial.print(" Y: ");
    Serial.print(y);
    Serial.print(" Theta: ");
    Serial.println(theta);
}
