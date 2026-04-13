# Sensors

## Temperature Correction
Temperature related drift for the Nav Accelerometer Z axis and Barometer can be corrected using settings `acc_temp_correction` and `baro_temp_correction`. The settings are temperature correction factor in cms-2/K and cm/K respectively, limited to -50 to +50, and are based on a linear correction characteristic. It is possible to perform an auto calibration of the required correction value using a setting of 51 with the auto calibration ending after a 5 minute timeout or on first arm. It's also possible to simply enter a value and see how the sensor drifts as the FC warms up, e.g. a BMP280 barometer requires a value around 20 cm/K for the `baro_temp_correction` setting.

### Barometer Calibration
Best calibrated by checking barometer altitude in the Configurator Sensor tab, powered just from USB or the main battery. First check Barometer altitude drift with `baro_temp_correction` set to 0 starting with a cold FC. Then change the setting to 51, power off and allow the FC to cool then power on again and recheck the barometer altitude in the Configurator Sensors tab. The barometer altitude will drift as before until 5 mins after bootup at which point, if the calibration has worked, the barometer altitude should fall close to 0. This will be accompanied by a "success" beep (may need battery power for this rather than just USB). The calibrated setting can be saved by switching to the CLI and hitting "Save Settings" (or use the Save stick command or CMS). Power off and allow the FC to cool down then recheck barometer altitude again, it should show much reduced altitude drift as it warms up.

### Nav Acceleromter Z Axis Calibration
The Nav accelerometer Z axis is calibrated in a similar way to the Barometer except it isn't so easy to check while calibrating given the lack of direct feedback. The acc Z value in the Configurator Sensors tab isn't the corrected value so can't be used. Instead set `debug_mode` to `ALTITUDE` and check the behaviour of `Debug 3` in the Configurator Sensors Tab for Debug output. It will drift during calibration then show a sudden change when calibration is finished. After saving and rebooting check `Debug 3` in the Configurator Sensors tab, it should show much less drift from zero if the calibration was successful. Successful calibration can also be checked by making sure `acc_temp_correction` shows a value other than 0 or 51.







