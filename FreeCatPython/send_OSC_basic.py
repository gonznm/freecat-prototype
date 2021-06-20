import os
import random
import numpy as np
from pythonosc.udp_client import SimpleUDPClient

# OSC config
IP = "127.0.0.1"
PORT = 9001

# Basic OSC sender
# Create list of arguments to be sent: ids, paths, x, y, targetLoudness, startSamples, loudnessValues
# Example –> [1234, "/path/audio", 1.5, 1.2, 45.5, "0, 4410, 8820", "48.0,54.2,44.1"]
arguments = []
random.seed(10)
np.random.seed(10)
for path in os.listdir("/Volumes/Almacen/DirectCode/freecat-prototype/sounds"):
    if path.endswith(".ogg"):
        # Sound ID
        _, file = os.path.split(path)
        id = int(file.split("_")[0])
        arguments.append(id)
        # Path
        arguments.append('/Volumes/Almacen/DirectCode/freecat-prototype/sounds/'+path)

        # Fake coordinates
        x = random.uniform(0, 1)
        y = random.uniform(0, 1)
        arguments.append(x)
        arguments.append(y)

        # Fake target loudness
        l = random.uniform(40, 90)
        arguments.append(l)

        # Fake start samples

        startSamples = [0, 4410, 8820, 13230, 17640, 22050]
        arguments.append(str(startSamples).replace('[','').replace(']','').replace(',',''))

        # Fake loudness of each grain
        loudnessValues = np.random.randint(40,90,6)
        arguments.append(str(loudnessValues).replace('[','').replace(']',''))

print("Number of arguments: ", len(arguments) )
print("\n",arguments)
# Send OSC message to JUCE
client = SimpleUDPClient(IP, PORT)
client.send_message('/juce', arguments)
