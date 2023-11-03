import requests
from datetime import datetime
from tft2 import DisplayController
from objloader import OBJLoader
import time
import json

def fetch_electricity_price():
    now = datetime.now()
    date = now.strftime("%Y-%m-%d")
    hour = now.strftime("%H")

    params = {
        "date": date,
        "hour": hour
    }

    url = "https://api.porssisahko.net/v1/price.json"
    response = requests.get(url, params=params)

    if response.status_code == 200:
        data = response.json()
        return data.get("price")
    else:
        return None


def calculate_seconds_to_next_fetch():
    # wait until next 5 mins past full hour
    current_minute = datetime.now().minute
    if current_minute < 5:
        return (5 - current_minute) * 60
    return (65 - current_minute) * 60

def select_color(price):
    if price < 5:
        return (0x40, 0xcc, 0x40)  # green
    elif price < 10:
        return (0xcc, 0xcc, 0x40)  # yellow
    elif price < 15:
        return (0xdd, 0x8c, 0x40)  # orange
    else:
        return (0xff, 0x40, 0x40)  # red


if __name__ == '__main__':

    vfont = OBJLoader("./vfont.obj")

    with DisplayController('COM7') as dc:
        dc.sync()
        dc.console(False)
        dc.clear()
        dc.font(DisplayController.BIGFONT)
        retry_delay = 60
        while True:

            price = fetch_electricity_price()
            if price is None:
                print("fetch failed")
                dc.clear()
                dc.color(0xaa, 0x0, 0x0)
                vfont.print(dc, 'FETCH FAILED', 0, 0, 200, 60)

                time.sleep(retry_delay)
                retry_delay = min(retry_delay * 1.5, 3600)
            else:
                sleeptime = calculate_seconds_to_next_fetch()
                print(f"price: {price} sleeping {sleeptime/60} minutes")

                dc.clear()

                dc.color(0xaa,0xaa,0xcc)
                vfont.print(dc, 'CURRENT PRICE:', 0, 40, 40)

                dc.color(*select_color(price))
                vfont.print(dc, f'{price}¤', 0, 140, 130)

                # Get the current time in HH:MM format
                current_time = datetime.now().strftime('%H:%M')
                dc.color(0xaa,0xaa,0xcc)
                vfont.print(dc, f'UPDATED {current_time}', 0, 220, 20)

                # color legend
                dc.color(*select_color(4))
                vfont.print(dc, 'GREEN IS UNDER 5.0', 0, 240, 20)

                dc.color(*select_color(6))
                vfont.print(dc, 'YELLOW IS UNDER 10.0', 0, 260, 20)

                dc.color(*select_color(11))
                vfont.print(dc, 'ORANGE IS UNDER 15.0', 0, 280, 20)

                dc.color(*select_color(16))
                vfont.print(dc, 'RED ALERT OVER 15.0', 0, 300, 20)

                time.sleep(sleeptime)
                retry_delay = 60
