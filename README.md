# iot-final-proj

## To do:
- //connect to wifi
- upload to database
- save last few measurements and make prediction straight in code (smooth it out)
- set threshold and make the light go on
- use grafana to visualize data

## References:

Info about sensors: http://www.cs.unibo.it/projects/iot/code/sensorKit.pdf (ours is page 20)

DHT Example: https://github.com/adafruit/DHT-sensor-library/blob/master/examples/DHTtester/DHTtester.ino

How to setup/use Grafana: https://diyprojects.io/grafana-installation-macos-influxdb-charts-mysensors-iot/

## Other Notes

- to read from the influxdb database from the command line:
curl -G 'http://localhost:8086/query' --data-urlencode "db=mydb” --data-urlencode "q=SELECT value FROM temperature LIMIT 1"

- grafana can also do alerts/send notifications so we can set it up so grafana tells us when its above the threshold (or we can do that in the code directly - might still be easier)