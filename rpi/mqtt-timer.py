import math
import os
import sys
import threading
import time
from datetime import datetime

# pip3 install gpiozero paho-mqtt scrollphathd
import paho.mqtt.client as mqtt
import scrollphathd as sphd
from gpiozero import LightSensor

LDR_PIN = 24
MAX_BRIGHTNESS = 0.2


class Timer:
    digits = (
        ("XXX", "..X", "XXX", "XXX", "X.X", "XXX", "XXX", "XXX", "XXX", "XXX"),
        ("X.X", "..X", "..X", "..X", "X.X", "X..", "X..", "..X", "X.X", "X.X"),
        ("X.X", "..X", "..X", "..X", "X.X", "X..", "X..", "..X", "X.X", "X.X"),
        ("X.X", "..X", "XXX", "XXX", "XXX", "XXX", "XXX", "..X", "XXX", "XXX"),
        ("X.X", "..X", "X..", "..X", "..X", "..X", "X.X", "..X", "X.X", "..X"),
        ("X.X", "..X", "X..", "..X", "..X", "..X", "X.X", "..X", "X.X", "..X"),
        ("XXX", "..X", "XXX", "XXX", "..X", "XXX", "XXX", "..X", "XXX", "XXX")
    )
    next_alarm_time = datetime.utcfromtimestamp(0)

    @staticmethod
    def draw_digit(x, y, number, brightness):
        for y2 in range(7):
            for x2 in range(3):
                if Timer.digits[y2][number][x2] == 'X':
                    sphd.set_pixel(x + x2, y + y2, brightness)

    @staticmethod
    def run():
        ldr = LightSensor(LDR_PIN)
        if "flip" in sys.argv:
            sphd.rotate(180)

        while True:
            delta_seconds = int((Timer.next_alarm_time - datetime.utcnow()).total_seconds())
            current_time = None
            if 0 <= delta_seconds <= 43200:
                delta_minutes = math.ceil(delta_seconds / 60)
                current_time = (delta_minutes // 60, delta_minutes % 60)
            brightness = MAX_BRIGHTNESS if ldr.light_detected else 0.1

            sphd.clear()
            if current_time is not None:
                if current_time[0] > 9:
                    Timer.draw_digit(0, 0, current_time[0] // 10, brightness)
                if delta_seconds >= 60:
                    Timer.draw_digit(4, 0, current_time[0] % 10, brightness)
                    Timer.draw_digit(10, 0, current_time[1] // 10, brightness)
                    Timer.draw_digit(14, 0, current_time[1] % 10, brightness)
                else:
                    Timer.draw_digit(10, 0, delta_seconds // 10, brightness)
                    Timer.draw_digit(14, 0, delta_seconds % 10, brightness)
                sphd.set_pixel(8, 1, brightness)
                sphd.set_pixel(8, 5, brightness)
                sphd.set_clear_on_exit()
            sphd.show()

            time.sleep(0.5)


def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    client.subscribe("pi/next-alarm-time", qos=1)


def on_message(client, userdata, msg):
    print(f"{msg.topic} {msg.payload}")
    if msg.topic == "pi/next-alarm-time":
        Timer.next_alarm_time = datetime.utcfromtimestamp(int(msg.payload) / 1000)


def main():
    threading.Thread(target=Timer.run).start()
    client = mqtt.Client(client_id=os.uname()[1], clean_session=True)
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect("localhost", 1883, 60)
    client.loop_forever()


if __name__ == "__main__":
    main()
