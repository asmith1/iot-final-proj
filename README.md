# iot-final-proj

## To do:
- //connect to wifi
- upload to database
  - change HTTP request to upload both humidity and temperature to server at the same time (might need to make 2 requests for this)
- //save last few measurements and make prediction straight in code (smooth it out)
- //set threshold and make the light go on
- //shut down in case sensors are disconnected
- Notify user if sensors are failing too much (Maybe via grafana?)
- use grafana to visualize data
- project documentation (Only missing Grafana screenshot, device picture and final review)
- presentation (Igna working on t)
- [optional] have the user control the parameters of temperature and humidity remotely

## References:

Info about sensors: http://www.cs.unibo.it/projects/iot/code/sensorKit.pdf (ours is page 20)

DHT Example: https://github.com/adafruit/DHT-sensor-library/blob/master/examples/DHTtester/DHTtester.ino

How to setup/use Grafana: https://diyprojects.io/grafana-installation-macos-influxdb-charts-mysensors-iot/

## Other Notes

- to read from the influxdb database from the command line:
curl -G 'http://localhost:8086/query' --data-urlencode "db=mydb” --data-urlencode "q=SELECT value FROM temperature LIMIT 1"

- grafana can also do alerts/send notifications so we can set it up so grafana tells us when its above the threshold (or we can do that in the code directly - might still be easier)
