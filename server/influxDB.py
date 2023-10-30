from datetime import datetime
import csv
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS

# You can generate a Token from the "Tokens Tab" in the UI
token = "CvMuRWwO8ldZsQk-VspC_6yx6xZIWy1ObS6Hcl1QxnNycWozKl_HQ_JyXmd7yXvhOfgneL5jJoZgqYQy6sb0xg=="
org = "lab"
bucket = "ESP32"

client = InfluxDBClient(url="http://10.118.126.244:8086", token=token, org=org)


write_api = client.write_api(write_options=SYNCHRONOUS)
query_api = client.query_api()


# using csv library
csv_result = query_api.query_csv(
    f'from(bucket:"{bucket}") |> range(start: -99h)''|> filter(fn: (r) => r["_measurement"] == "Energy_status")')
with open("output.csv", 'w', newline='') as csvfile:
    writer = csv.writer(csvfile)
    for row in csv_result:
        writer.writerow(row)
        print(row)
