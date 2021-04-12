import sys
import time
from datetime import datetime

import scrollphathd as sphd
from gpiozero import LightSensor

LDR_PIN = 24
MAX_BRIGHTNESS = 0.2

digits = (
    ("XXX", "..X", "XXX", "XXX", "X.X", "XXX", "XXX", "XXX", "XXX", "XXX"),
    ("X.X", "..X", "..X", "..X", "X.X", "X..", "X..", "..X", "X.X", "X.X"),
    ("X.X", "..X", "..X", "..X", "X.X", "X..", "X..", "..X", "X.X", "X.X"),
    ("X.X", "..X", "XXX", "XXX", "XXX", "XXX", "XXX", "..X", "XXX", "XXX"),
    ("X.X", "..X", "X..", "..X", "..X", "..X", "X.X", "..X", "X.X", "..X"),
    ("X.X", "..X", "X..", "..X", "..X", "..X", "X.X", "..X", "X.X", "..X"),
    ("XXX", "..X", "XXX", "XXX", "..X", "XXX", "XXX", "..X", "XXX", "XXX")
)


def draw_digit(x, y, number, brightness):
    for y2 in range(7):
        for x2 in range(3):
            if digits[y2][number][x2] == 'X':
                sphd.set_pixel(x + x2, y + y2, brightness)


def main():
    brightness = 0
    last_time = None
    show_colon = False
    ldr = LightSensor(LDR_PIN)

    if "flip" in sys.argv:
        sphd.rotate(180)

    while True:
        now = datetime.now()
        current_time = (now.hour, now.minute)
        old_brightness = brightness
        brightness = MAX_BRIGHTNESS if ldr.light_detected else 0.1

        if current_time != last_time or brightness != old_brightness:
            sphd.clear()
            draw_digit(0, 0, now.hour // 10, brightness)
            draw_digit(4, 0, now.hour % 10, brightness)
            draw_digit(10, 0, now.minute // 10, brightness)
            draw_digit(14, 0, now.minute % 10, brightness)
            last_time = current_time

        if show_colon != (now.microsecond < 500000) or brightness != old_brightness:
            show_colon = now.microsecond < 500000
            sphd.set_pixel(8, 1, brightness * show_colon)
            sphd.set_pixel(8, 5, brightness * show_colon)
            sphd.show()

        time.sleep(0.01)


if __name__ == "__main__":
    try:
        main()
    except (KeyboardInterrupt, SystemExit):
        sphd.clear()
