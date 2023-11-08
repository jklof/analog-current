import requests
from datetime import datetime
from datetime import timedelta
from tft2 import DisplayController
import argparse
import threading
import pytz
import traceback

# Define default values for price margin and COM port
DEFAULT_PRICE_MARGIN = 0.0
DEFAULT_COM_PORT = 'COM7'


# Event-related variables
g_quit_flag = False
g_lock = threading.Lock()
g_condition = threading.Condition(g_lock)


def fetch_porssisahko_net_all():
    """
    Fetch electricity price data from the Porssisahko API and process it.
    Returns a sorted list of price data.
    """

    url = "https://api.porssisahko.net/v1/latest-prices.json"
    response = requests.get(url)

    if response.status_code == 200:
        data = response.json()

        # Define the timezone using pytz
        local_tz = pytz.timezone('Europe/Helsinki')

        for entry in data["prices"]:
            start_utc = datetime.strptime(entry["startDate"], "%Y-%m-%dT%H:%M:%S.%fZ").replace(tzinfo=pytz.utc)
            end_utc = datetime.strptime(entry["endDate"], "%Y-%m-%dT%H:%M:%S.%fZ").replace(tzinfo=pytz.utc)
            # Convert to your local time zone
            entry["startDate"] = start_utc.astimezone(local_tz).replace(tzinfo=None)
            entry["endDate"] = end_utc.astimezone(local_tz).replace(tzinfo=None)

        # Sort data by timestamps
        return sorted(data["prices"], key=lambda x: x["startDate"])
    else:
        return None

class Updater:
    def __init__(self):
        """
        Initialize the Updater with the last update time and fetched data.
        """
        self.last_update = datetime.now()
        self.data = fetch_porssisahko_net_all()

    def next_update_time(self):
        """
        Calculate the next update time based on the last update time.
        If there is no data, retry in 5 minutes; otherwise, update on the next full hour.
        """
        if self.data is None:
            return self.last_update + timedelta(mins=5)

        return self.last_update.replace(minute=0,second=0,microsecond=0) + timedelta(hours=1)

    def get_data(self):
        """
        Get the data, first updating if the current time exceeds the update time.
        """
        current_time = datetime.now()
        if current_time >= self.next_update_time():
            self.data = fetch_porssisahko_net_all()
            self.last_update = current_time
        return self.data



def gradient_color(value):
    """
    Calculate a gradient color based on the given value.
    """
    colors = [(0, 255, 0), (255, 255, 0), (255, 165, 0), (255, 0, 0)]
    positions = [0.0, 0.25, 0.5, 1.0]

    for i in range(1, len(positions)):
        if value <= positions[i]:
            t = (value - positions[i-1]) / (positions[i] - positions[i-1])
            color = (
                int(colors[i-1][0] * (1 - t) + colors[i][0] * t),
                int(colors[i-1][1] * (1 - t) + colors[i][1] * t),
                int(colors[i-1][2] * (1 - t) + colors[i][2] * t)
            )
            return color

    return colors[-1]

def select_color(price):
    """
    Calculate a color based on the price value.
    """
    cool = 4
    warm = 20
    p = min(max(0, (price-cool) / (warm-cool)), 1)
    return gradient_color(p)

def text_color():
    """
    Return the default text color.
    """
    return (0xaa,0xaa,0xcc)


class Dash:
    def __init__(self, dc):
        """
        Initialize the Dash with the display controller and start a thread to run the Dash.
        """
        self.dc = dc
        self.updater = Updater()
        self.thread = threading.Thread(target=self.run)
        self.thread.start()

    def join(self):
        """
        Wait for the Dash thread to finish.
        """
        self.thread.join()

    def draw_price_graph(self):
        """
        Draw the main price graph on the display.
        """
        # get latest data
        data = self.updater.get_data()

        # failed to fetch?
        if data is None:
            dc.color(0, 0, 0)
            dc.box(0,0, 479, 290)
            self.dc.color(0xaa, 0x0, 0x0)
            self.dc.vtext('FETCH FAILED', 0, 200, 100)
            return

        # calculate boundaries and statistics
        current_date = datetime.now()
        current_entry = None
        oldest_date = datetime.max
        newest_date = datetime.min
        max_price = float('-inf')
        min_price = float('inf')
        total_price = 0
        for entry in data:
            startDate = entry['startDate']
            endDate = entry['endDate']
            price = entry['price']
            oldest_date = min(oldest_date, startDate)
            newest_date = max(newest_date, startDate)
            max_price = max(max_price, price)
            min_price = min(min_price, price)
            total_price += price
            if current_date >= startDate and current_date <= endDate:
                current_entry = entry
        average_price = total_price / len(data)

        # clear background area
        dc.color(0, 0, 0)
        dc.box(0,0, 479, 290)

        if current_entry is not None:
            # "current price" text
            start_date = current_entry["startDate"].strftime('%H:%M')
            end_date = current_entry["endDate"].strftime('%H:%M')
            updated = self.updater.last_update.strftime('%H:%M')
            dc.color(*text_color())
            dc.vtext(f'HOUR {start_date}-{end_date} - UPDATED {updated}', 0, 40, 50)

            # price
            current_price = current_entry["price"]
            dc.color(*select_color(current_price))
            dc.vtext(f'{current_price}$', 0, 140, 180)

        # max, average and min for the range
        self.dc.color(*select_color(max_price))
        self.dc.vtext(f'MAX {max_price:.2f}', 0, 220, 40)
        self.dc.color(*select_color(average_price))
        self.dc.vtext(f'AVG {average_price:.2f}', 0, 240, 40)
        self.dc.color(*select_color(min_price))
        self.dc.vtext(f'MIN {min_price:.2f}', 0, 260, 40)

        # scale dates to range 0...1
        scale = (newest_date - oldest_date)

        xo = 120 #left origin of graph
        yo = 260 #bottom origin of graph
        dx = 320 #width of graph
        dy = 100 #height of graph

        # mark the current date on the graph
        dc.color(*text_color())
        x = (current_date - oldest_date) / (newest_date - oldest_date)
        self.dc.line(int(xo + x*dx), int(yo+5), int(xo+x*dx), int(yo-dy-5))
        #zero line
        self.dc.line(int(xo), int(yo+1), int(xo+dx), int(yo+1))

        # draw a graph of prices
        for entry in data:
            date = entry["startDate"]
            price = entry["price"]
            x = (date - oldest_date ) / (newest_date - oldest_date)
            y = price / max_price
            self.dc.color(*select_color(price))
            self.dc.line(int(xo + x*dx), int(yo), int(xo+x*dx), int(yo-y*dy))


    def next_update_time_in_seconds(self):
        """
        Calculate the time in seconds until the next update is required.
        """
        td = self.updater.next_update_time() - datetime.now()
        return max(0,td.total_seconds())

    def run(self):
        """
        Run the Dash thread, continuously updating the display.
        """
        global g_lock, g_quit_flag
        try:
            # This is the function that will run in the thread
            with g_lock:
                while not g_quit_flag:
                    self.draw_price_graph()
                    g_condition.wait(self.next_update_time_in_seconds())
        except:
            traceback.print_exc()
            with g_lock:
                g_quit_flag = True
                g_condition.notify_all()


class Clock:
    def __init__(self, dc):
        """
        Initialize the Clock with the display controller and start a thread to run the Clock.
        """
        self.dc = dc
        self.thread = threading.Thread(target=self.run)
        self.thread.start()

    def join(self):
        """
        Wait for the Clock thread to finish.
        """
        self.thread.join()

    def draw_clock(self):
        """
        Draw the current time on the display.
        """
        self.clock_time = datetime.now()
        timestr = self.clock_time.strftime('%H:%M')
        dc.color(0,0,0)
        dc.box(0,294,110,310)
        dc.color(*text_color())
        dc.vtext(f'TIME {timestr}', 0, 310, 40)

    def next_update_time_in_seconds(self):
        """
        Calculate the time in seconds until the next minute.
        """
        next_time = self.clock_time.replace(second=0,microsecond=0) + timedelta(minutes=1)
        td = next_time - datetime.now()
        return max(0,td.total_seconds())

    def run(self):
        """
        Run the Clock thread, continuously updating the display with the current time.
        """
        global g_lock,g_quit_flag
        try:
            with g_lock:
                while not g_quit_flag:
                    self.draw_clock()
                    g_condition.wait(self.next_update_time_in_seconds())
        except:
            traceback.print_exc()
            with g_lock:
                g_quit_flag = True
                g_condition.notify_all()


if __name__ == '__main__':

    # Create an argument parser
    parser = argparse.ArgumentParser(description='Display electricity price on TFT screen.')

    # Add command-line arguments for price margin and COM port
    parser.add_argument('--price-margin', type=float, default=DEFAULT_PRICE_MARGIN, help='Price margin to add to the retrieved price.')
    parser.add_argument('--com-port', default=DEFAULT_COM_PORT, help='COM port for the display controller.')
    args = parser.parse_args()

    with DisplayController(args.com_port) as dc:

        dc.sync()
        dc.clear()
        dc.vtext("DASH", 0,0, 150)

        clock = Clock(dc)
        dash = Dash(dc)

        try:
            with g_lock:
                while not g_quit_flag:
                    g_condition.wait(1)

        except KeyboardInterrupt:
            pass

        finally:
            with g_lock:
                g_quit_flag = True
                g_condition.notify_all()

        clock.join()
        dash.join()


