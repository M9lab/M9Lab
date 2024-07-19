from pybricks.hubs import CityHub
from pybricks.pupdevices import ColorDistanceSensor, DCMotor
from pybricks.parameters import Port, Color
from pybricks.tools import wait

print("ver 1.2.2")

# Initialize the CityHub
cityHub = CityHub()
print("Battery voltage: " + str(cityHub.battery.voltage()))

# Initialize the sensor.
train = DCMotor(Port.A)
sensor = ColorDistanceSensor(Port.B)


Color.WHITE = Color(h=78, s=5, v=92)
Color.RED = Color(h=357, s=97, v=75)
Color.YELLOW = Color(h=52, s=96, v=94)
Color.GREEN = Color(h=195, s=91, v=48)
Color.BLUE = Color(h=220, s=97, v=70)

# config (default)
my_colors = (Color.WHITE,Color.NONE)

# comment to read alla colours
sensor.detectable_colors(my_colors)

# Save your colors.
speed = 45
stop_time = 30000
ignore_sensor_time = 3000


# Now we use the function we just created above.
while True:

	cityHub.light.on(sensor.color())
	print(sensor.color())

	# (stop)
	if sensor.color() == Color.RED:	
		train.dc(0)

	# (start)
	if sensor.color() == Color.YELLOW:	
		train.dc(speed)

	# (invert)
	if sensor.color() == Color.GREEN:	
		speed = (-1 * speed)	
		train.dc(speed)	
		# ignore time
		wait(ignore_sensor_time)

	# (stop and go)
	if sensor.color() == Color.BLUE:
		train.dc(0)	
		wait(stop_time)
		train.dc(speed)		

	# (stop and invert)
	if sensor.color() == Color.WHITE:	
		train.dc(0)	
		wait(stop_time)
		speed = (-1 * speed)	
		train.dc(speed)	
		# ignore time
		wait(ignore_sensor_time)

	else:
	# If the sensor sees nothing	
		cityHub.light.off()
		wait(10)	
	
wait(10)	