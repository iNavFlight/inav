# "Soft" PWM demo

The PWM in this demo is "soft", meaning that it does use a hardware timer to run, but the output is not directly routed to a pin; a callback functions which turn a LED on and off are used instead.

## Notes

The brightness percentages it goes only up to 9900 (99%) instead of 10000 (100%); otherwise there is a noticeable blink in the top of the cycle. The reason is that the on/off callback functions take too long to execute for this kind of (relatively fast) timer. Likewise, '0' (0%) in the cycle actually means that the LED doesn't get turned off, because the channel notification function that turns the LED off doesn't get called. So in a setup like this one, '1' should be the minimum; although it does mean that the LED is never fully off.
