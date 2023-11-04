import requests
from datetime import datetime
from datetime import timedelta
from tft2 import DisplayController
import time
import json
import argparse

# Define default values for price margin and COM port
DEFAULT_PRICE_MARGIN = 0.0
DEFAULT_COM_PORT = 'COM7'

#seems wrong? fix it
def fetch_sahkonhinta_api_fi_price():
    now = datetime.now()
    later = now + timedelta(hours=1)

    raja = (now.strftime  ("%Y-%m-%dT%H:%M") + '_' +
            later.strftime("%Y-%m-%dT%H:%M"))

    params = {
        "tunnit": 1, # number of hours
        "tulos": "sarja",
        "aikaraja" : raja
    }
    url = "https://www.sahkohinta-api.fi/api/v1/halpa"
    response = requests.get(url, params=params)

    if response.status_code == 200:
        data = response.json()
        print(data)
        return float(data[0].get("hinta"))
    else:
        return None

def fetch_porssisahko_net_price():
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





def fetch_porssisahko_net_all():
    url = "https://api.porssisahko.net/v1/latest-prices.json"
    response = requests.get(url)

    if response.status_code == 200:
        data = response.json()
        # Convert timestamps to datetime objects
        for entry in data["prices"]:
            entry["startDate"] = datetime.strptime(entry["startDate"], "%Y-%m-%dT%H:%M:%S.%fZ")
            entry["endDate"] = datetime.strptime(entry["endDate"], "%Y-%m-%dT%H:%M:%S.%fZ")

        # Sort data by timestamps
        return sorted(data["prices"], key=lambda x: x["startDate"])
    else:
        return []


def price_graph(dc):
    data = fetch_porssisahko_net_all()

    oldest_date = min(entry["startDate"] for entry in data)
    newest_date = max(entry["startDate"] for entry in data)
    max_price = max(entry["price"] for entry in data)
    min_price = min(entry["price"] for entry in data)
    total_price = sum(entry["price"] for entry in data)
    average_price = total_price / len(data)


    dc.color(*select_color(max_price))
    dc.vtext(f'MAX {max_price:.2f}', 0, 220, 40)
    dc.color(*select_color(average_price))
    dc.vtext(f'AVG {average_price:.2f}', 0, 240, 40)
    dc.color(*select_color(min_price))
    dc.vtext(f'MIN {min_price:.2f}', 0, 260, 40)

    # scale dates to range 0...1
    scale = (newest_date - oldest_date)

    xo = 120 #left origin of graph
    yo = 260 #bottom origin of graph
    dx = 320 #width of graph
    dy = 100 #height of graph

    # mark the current date on the graph
    dc.color(0x80, 0x80, 0x30)
    x = (datetime.now() - oldest_date) / (newest_date - oldest_date)
    y = 1
    dc.line(int(xo + x*dx), int(yo+5), int(xo+x*dx), int(yo-y*dy))
    #zero line
    dc.line(int(xo), int(yo-1), int(xo+dx), int(yo-1))

    # draw a graph of prices
    dc.color(0x60,0xd0,0x60)
    for entry in data:
        date = entry["startDate"]
        price = entry["price"]
        x = (date - oldest_date ) / (newest_date - oldest_date)
        y = price / max_price
        dc.line(int(xo + x*dx), int(yo), int(xo+x*dx), int(yo-y*dy))



def fetch_electricity_price():
    return fetch_porssisahko_net_price()
    #return fetch_sahkonhinta_api_fi_price()


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

    # Create an argument parser
    parser = argparse.ArgumentParser(description='Display electricity price on TFT screen.')

    # Add command-line arguments for price margin and COM port
    parser.add_argument('--price-margin', type=float, default=DEFAULT_PRICE_MARGIN, help='Price margin to add to the retrieved price.')
    parser.add_argument('--com-port', default=DEFAULT_COM_PORT, help='COM port for the display controller.')
    args = parser.parse_args()

    #fetch_porssisahko_net_all()  todo...

    with DisplayController(args.com_port) as dc:
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
                dc.vtext('FETCH FAILED', 0, 200, 100)

                time.sleep(retry_delay)
                retry_delay = min(retry_delay * 1.5, 3600)
            else:
                sleeptime = calculate_seconds_to_next_fetch()
                print(f"price: {price} sleeping {sleeptime/60} minutes")

                dc.clear()

                # print graph
                price_graph(dc)

                # current price text
                dc.color(0xaa,0xaa,0xcc)
                dc.vtext('CURRENT PRICE:', 0, 40, 80)

                # add margin to the price before displaying..
                price += args.price_margin
                dc.color(*select_color(price))
                dc.vtext(f'{price}$', 0, 140, 180)

                # Get the current time in HH:MM format
                current_time = datetime.now().strftime('%H:%M')
                dc.color(0xaa,0xaa,0xcc)
                dc.vtext(f'UPDATED {current_time}', 0, 300, 40)

                time.sleep(sleeptime)
                retry_delay = 60
