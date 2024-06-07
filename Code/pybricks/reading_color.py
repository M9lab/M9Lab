from pybricks.hubs import CityHub
from pybricks.pupdevices import ColorDistanceSensor, DCMotor
from pybricks.parameters import Port, Color
from pybricks.tools import wait

print("ver 1.1.1")

# Initialize the CityHub
cityHub = CityHub()

# Initialize the sensor.
train = DCMotor(Port.A)
sensor = ColorDistanceSensor(Port.B)

while True:
	#hsv = sensor.hsv(surface=False)
    #color = sensor.color(surface=False)   
    #ambient = sensor.ambient()   
    #print(hsv, color, ambient)

    color = sensor.hsv()
    print(color)

    # Wait so we can read the value.
    wait(500)