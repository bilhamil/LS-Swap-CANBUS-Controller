# LS-Swap-CANBUS-Controller

Controller and code to interoprate with a gen iv ls bcm to provide functionality for cruise control by providing a cruise indicator light and spoofing instrument cluster can bus messages, PRNDL functionality for gear selection indication, and a/c requests by mimicing hvac control messages.

Note all communication occurs over the low speed gmlan and is based on operation with a 2013 Silverado e38 ecm and bcm.

The controller is based off the arduino blue pill integrated with a custom pcb, the plans for which can be found here https://oshwlab.com/luin.uial/can-bus-multiplexer

The project is loosely based off the project developed by atc1441 here: https://github.com/atc1441/CustomCanDecoderBox
