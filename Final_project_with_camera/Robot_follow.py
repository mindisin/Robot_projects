import cv2
import numpy as np
import serial
import time

# Инициализация последовательного порта
serialDevice = serial.Serial('COM11', 9600)

# Функция для отправки данных
def sendMessage(data):
    serialDevice.write(data.encode('utf-8'))
    time.sleep(0.01)

# Инициализация камеры
cap = cv2.VideoCapture(0)

# Проверка, открыта ли камера
if not cap.isOpened():
    print("Ошибка: Камера не найдена.")
    exit()

# Расширенный диапазон желтого цвета в HSV, ближе к оранжевому
lower_yellow = np.array([10, 80, 80])   # Более тёплый желтый, ближе к оранжевому
upper_yellow = np.array([40, 255, 255])  # Более яркий, насыщенный желтый

# Списки для хранения значений координат
cx_list = []
cy_list = []

# Переменные для таймера
start_time = time.time()
interval = 1  # Интервал времени в секундах

while True:
    ret, frame = cap.read()
    if not ret:
        break

    # Преобразование в HSV
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # Фильтрация по цвету
    mask = cv2.inRange(hsv, lower_yellow, upper_yellow)

    # Размытие маски (для устранения шума)
    mask = cv2.GaussianBlur(mask, (5, 5), 0)

    # Поиск контуров
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    if contours:
        # Выбор наибольшего контура (ближайший объект)
        largest_contour = max(contours, key=cv2.contourArea)

        # Вычисление центра масс
        M = cv2.moments(largest_contour)
        if M["m00"] != 0:
            cx = int(M["m10"] / M["m00"])
            cy = int(M["m01"] / M["m00"])
            center = (cx, cy)

            # Отрисовка границ объекта
            cv2.drawContours(frame, [largest_contour], -1, (0, 255, 0), 2)
            cv2.circle(frame, center, 5, (0, 255, 0), -1)

            # Добавление координат в списки
            cx_list.append(cx)
            cy_list.append(cy)

    # Отображение результата
    cv2.imshow("Tracking", frame)
    cv2.imshow("Mask", mask)  # Показываем маску для отладки

    # Проверка времени
    current_time = time.time()
    if current_time - start_time >= interval:
        if cx_list and cy_list:
            avg_cx = int(np.mean(cx_list))
            avg_cy = int(np.mean(cy_list))
            print(f"Средние координаты: ({avg_cx}, {avg_cy})")
            sendMessage(f"{avg_cx},{avg_cy}\n")

        # Очистка списков
        cx_list.clear()
        cy_list.clear()
        start_time = current_time

    # Выход по 'q'
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Освобождение ресурсов
cap.release()
cv2.destroyAllWindows()
