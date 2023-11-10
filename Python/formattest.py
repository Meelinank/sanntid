rvrTemps        = 0
rvrLightSensor  = 1
rvrAmbientLight = 2
rvrBattery      = 3

formatedMessage = "{"+"{},{},{},{}".format(rvrTemps,rvrLightSensor,rvrAmbientLight,rvrBattery)+"}"
print(formatedMessage)