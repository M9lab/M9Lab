from pybricks.hubs import CityHub
from pybricks.pupdevices import ColorDistanceSensor, DCMotor
from pybricks.parameters import Port, Color
from pybricks.tools import wait

# Initialize the CityHub
cityHub = CityHub()
print("Battery voltage: " + str(hub.battery.voltage()))

# Initialize the sensor.
train = DCMotor(Port.A)
sensor = ColorDistanceSensor(Port.B)


# Now we use the function we just created above.
while True:

    # Here you can make your train/vehicle stop.

    if (sensor.color()==Color.RED):
	cityHub.light.on(Color.RED)
	train.dc(0)

    # Here you can make your train/vehicle go forward.

    if (sensor.color()==Color.GREEN):
	cityHub.light.on(Color.GREEN);
	train.dc(30)
	
	# Here you can make your train/vehicle go backward.
	
    if (sensor.color()==Color.WHITE):
	cityHub.light.on(Color.WHITE);
	train.dc(-1 * train.dc())	