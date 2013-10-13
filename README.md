# Lizard

`Lizard` energy monitor Arduino sketch for jeelib compatible boards

## Hub

The hub receives data over RFM12B and makes it available via a JSON API, it's
not particularly smart about it.

It runs on an Arduino (specifically, jeenode) compatible board with an RFM12B
and an ENC28J60.

I'm using a [nanode "gateway"](http://www.nanode.eu/), with the extra jeeport
header soldered on for a [RFM12B board](http://jeelabs.net/projects/hardware/wiki/RFM12B_Board)

### Setup

1. Fill in the mymac variable with your device's mac address.
2. It's set up to run with a static IP of `192.168.0.100`, and gateway located
at `192.168.0.1`. You may need to change these for your particular network.
    - You may need to change these for your particular network.
    - To use DHCP, in `setupNetwork()`, comment out the static setup and
    uncomment the DHCP setup.
3. Upload!
4. Visit `http://192.168.0.1/who`, you should recieve the response:
    `{"hub":true}`

## Lizard

TODO.

### Setup

TODO.

#### Calbration

TODO.

