# temp-controller

Every good good brewer/engineer at some point has to write their own temperature control software.  This is mine.

I have 4 different STC-1000 units providing individual fermenation temp control, an Auber PID controlling a RIMS tube, and an Auber EZboil controlling mash/boil/distilling on my Unibrau eBIAB system.  They're all great, but none of them can connect to the cloud to allow for remote control/monitoring/logging.

This project aims to build a hardware temp controller that could replace all those, that can do 2-stage heating/cooling, toggle algorithms between Joshnon-style on/off, PID, and PWM output, not *require* internet access to function (local physical control/display), but provide additional cloud control.

The hardware is built around the [Particle](https://www.particle.io/) platform, who's Kickstarter's I've been supporting from the beginning.  It's been tested and run on the Photon.  See [particle/README.md]() for instructions on compiling/flashing.

The webapp is a simple VueJS frontend that talks directly to Particle's cloud to interface with the device.

This project is an incomplete work in progress.
