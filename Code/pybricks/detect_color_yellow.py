from pybricks.hubs import CityHub
from pybricks.pupdevices import ColorDistanceSensor, DCMotor
from pybricks.parameters import Port, Color, Button
from pybricks.tools import wait

# Initialize the CityHub
cityHub = CityHub()
print("Battery voltage: " + str(cityHub.battery.voltage()))

# Initialize the sensor.
train = DCMotor(Port.A)
sensor = ColorDistanceSensor(Port.B)

dir = 45

my_colors = (Color.YELLOW,Color.NONE)

# Save your colors.
sensor.detectable_colors(my_colors)


# color() works as usual, but now it returns one of your specified colors.
while True:
    color = sensor.color()    

    # Print the results.
    print(color)
	cityHub.light.on(Color)

    
    # Check which one it is.
    if color == Color.YELLOW:

        train.stop() 
		# time before restart opposote direction
		wait(20000)	     
		dir = -1 * dir            
        train.dc(dir)		
		
		# ignore time
		wait(5000)	          
               
    else:
        # If the sensor sees nothing
        # nearby, just wait briefly.
		cityHub.light.off()
        wait(10)


wait(10)