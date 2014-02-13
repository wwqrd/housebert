# Housebert

Energy monitor Arduino sketch for jeelib compatible boards.

The setup consists of two boards, one running the __base station__ software, and the other running the __sensor node__ software.

## Requirements

1. Two jeenode compatible boards, with RFM12B circuits.
  - The __base station__ also requires an ENC28J60
  - The __sensor node__ requires a breakout board for your CT sensor.

I'm using a [nanode "gateway"](http://www.nanode.eu/), with the extra jeeport header soldered on for a [RFM12B board](http://jeelabs.net/projects/hardware/wiki/RFM12B_Board), and a plain old jeenode with a bespoke breakout board for the CT sensor.

## Base Station (base\_station/base\_station.ino)

The base station receives data over RFM12B and makes it available via a JSON API, it's not particularly smart about it.

### Setup

1. Fill in the mymac variable with your device's mac address.
2. It's set up to run with a static IP of `192.168.0.100`, and gateway located
at `192.168.0.1`. You may need to change these for your particular network.
    - You may need to change these for your particular network.
    - To use DHCP, in `setupNetwork()`, comment out the static setup and
    uncomment the DHCP setup.
3. Upload!
4. Visit `http://192.168.0.1/who`, you should recieve the response:
    `{"housebert":true}`

## Sensor Node (sensor\_node/sensor\_node.ino)

TODO

