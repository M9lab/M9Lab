from pybricks.hubs import CityHub
from pybricks.pupdevices import ColorDistanceSensor, DCMotor
from pybricks.parameters import Port, Color
from pybricks.tools import wait

# Initialize the CityHub
cityHub = CityHub()
print("Battery voltage: " + str(cityHub.battery.voltage()))

# Initialize the sensor.
train = DCMotor(Port.A)
sensor = ColorDistanceSensor(Port.B)

# config (default)
my_colors = (Color.RED,Color.GREEN,Color.YELLOW,Color.WHITE,Color.BLUE,Color.NONE)
sensor.detectable_colors(my_colors)

# Save your colors.
speed = 45
stop_time = 20000
ignore_sensor_time = 3000


# Now we use the function we just created above.
while True:

	cityHub.on(sensor.color());

    # (stop)
    if (sensor.color()==Color.RED):	
	train.dc(0)

    # (start)
    if (sensor.color()==Color.GREEN):	
	train.dc(speed)
	
	# (invert)
    if (sensor.color()==Color.WHITE):	
	train.dc(-1 * speed)	
	# ignore time
	wait(ignore_sensor_time)
	
	# (stop and go)
    if (sensor.color()==Color.BLUE):
	train.dc(0)	
	wait(stop_time)
	train.dc(speed)		
	
	# (stop and invert)
    if (sensor.color()==Color.YELLOW):	
	train.dc(0)	
	wait(stop_time)
	train.dc(-1 * speed)	
	# ignore time
	wait(ignore_sensor_time)

	else:
	# If the sensor sees nothing	
	cityHub.light.off()
	wait(10)	
	
wait(100)	