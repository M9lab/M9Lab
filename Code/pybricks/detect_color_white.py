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
stop = False

#Color.BLUE = Color(h=215, s=96, v=66)

my_colors = (Color.WHITE,Color.NONE)

# Save your colors.
sensor.detectable_colors(my_colors)


# color() works as usual, but now it returns one of your specified colors.
while True:
    color = sensor.color()
    colorh = sensor.hsv()

    # Print the results.
    print(color)

    
    # Check which one it is.
    if color == Color.WHITE:
        
        if stop == False:
            print('ailo')
            train.stop()        
            stop = True
            wait(7000)	        
        else:
            dir = -1 * dir            
            train.dc(dir)
            stop = False
            wait(1000)	  
               
    else:
        # If the sensor sees nothing
        # nearby, just wait briefly.
        wait(10)


wait(10)