import configs
import corpus_creation
import feature_extraction
from pythonosc.udp_client import SimpleUDPClient
from pythonosc import osc_server
from pythonosc import dispatcher
import sys
import gc
import math

## Example of text query
# query='kid'

## Example of query by examples
# sound_top_left = '357589' # Trumpet - Asharp3
# sound_top_right = '328727' #  flute_note_tremolo
# sound_bottom_left = '255805' # male voice
# sound_bottom_right = '344270' # glass smash, bottle, G

# OSC config
IP_r = "0.0.0.0"
PORT_r = 9002 # port for receiving
IP_s= "host.docker.internal"
PORT_s = 9001 # port for sending
MAX_BYTES_SIZE = 1024
"""Output of command "sysctl -n net.inet.udp.maxdgram": 9216 bytes
    I don't understand why this number of bytes doesn't work, but 2048 works... - it depends on the network I'm connected to, tho
    There is a limit on Juce's side, it can be 512 sometimes
"""

# Total number of sounds (including reference sounds): nx*ny
nx = 5 
ny = 5 

def interpolation_and_content_based_search(ref_ids_list):
    # ref_ids_list order has to be: [sound_top_left, sound_top_right, sound_bottom_left, sound_bottom_right]

    # Get these four reference sounds and their MFCCs
    ref_sounds = corpus_creation.get_sounds_by_ID(ref_ids_list)

    # Interpolation of intermediate MFCCs
    interp_values, grid = corpus_creation.grid_interpolation(ref_sounds, nx, ny)

    # Content based search using MFCCs as targets
    sounds = corpus_creation.get_sounds_by_MFCCs(interp_values, ref_sounds)

    # Download the ogg previews
    for count, sound in enumerate(sounds):
        print_mod('Downloading sound with id {0} [{1}/{2}]'.format(sound.id, count + 1, len(sounds)))
        corpus_creation.retrieve_sound_preview(sound, configs.FILES_DIR)

    return sounds, grid

def text_query(query):
    # Make a text query and get 25 results
    sounds = corpus_creation.query_freesound(query, num_results=25)
    # Download the ogg previews
    for count, sound in enumerate(sounds):
        print_mod('Downloading sound with id {0} [{1}/{2}]'.format(sound.id, count + 1, len(sounds)))
        corpus_creation.retrieve_sound_preview(sound, configs.FILES_DIR)
    return sounds

def actualsize(input_obj):
    """ Function by Naser Tamimi to calculate actual size of python objects.
    This is needed here because the size of a list (sys.getsizeof()) is not the size of the elements it contains.
    https://towardsdatascience.com/the-strange-size-of-python-objects-in-memory-ce87bdfbb97f
    """
    memory_size = 0
    ids = set()
    objects = [input_obj]
    while objects:
        new = []
        for obj in objects:
            if id(obj) not in ids:
                ids.add(id(obj))
                memory_size += sys.getsizeof(obj)
                new.append(obj)
        objects = gc.get_referents(*new)
    return memory_size

def create_arguments_list(df, query_by_examples):
    # Create list of arguments to be sent: ids, paths, x, y, targetLoudness, loudnessValues
    # Example –> [1234, "/path/audio", 1.5, 1.2, 45.5, "48.0874 54.2234 44.1123"]
    msgs = []  # list of arguments
    for idx, row in df.iterrows():
        arguments = []

        # Sound ID
        id = row['sound_id']
        # Path - harcoded to be compatible with docker
        path_docker = row['path']
        path = '/Volumes/Almacen/DirectCode/freecat-prototype/FreeCatPython'+path_docker[4:]

        # Sound coordinates
        if query_by_examples:
            x = row['grid'][0]
            y = row['grid'][1]
        else:
            x = row['pca_norm_range'][0]
            y = row['pca_norm_range'][1]

        # Target loudness
        ldns = row['average_loudness']

        # Loudness of each grain
        loudnessValues = row['grains_loudness']
        loudnessValues_formatted = str(loudnessValues).replace('[','').replace(']','').replace(',','')
        
        # Before adding arguments to the messages list, check its size in bytes, as it may have to be splitted
        size_small_args = actualsize([id, path, x, y, ldns])
        size_long_string = actualsize([loudnessValues_formatted])
        print_mod(f"Sending sound {id} {{{idx+1}/{df.shape[0]}}}")
        #print_mod(f"Size of the small part of the message: {size_small_args}. Size of the loudnessValues_formatted string: {size_long_string}")
        if size_long_string + size_small_args > MAX_BYTES_SIZE:
            # Divide long string of loudness values in parts, otherwise the message is too big for JUCE
            n_parts = size_long_string/(MAX_BYTES_SIZE-size_small_args-28)
            len_part = int(len(loudnessValues)/n_parts)
            print_mod(f"Dividing information in several messages. Number of parts: {n_parts} (rounded to {math.ceil(n_parts)}). Length of each part: {len_part} (total length: {len(loudnessValues)}).")
            arguments.append("New sound")
            for i in range(math.ceil(n_parts)):
                # Append same sound ID and rest of arguments
                arguments.append(id)
                arguments.append(path)
                arguments.append(x)
                arguments.append(y)
                arguments.append(ldns)
                # Take only a part of the list
                if (i+1)*len_part < len(loudnessValues):
                    l = loudnessValues[i*len_part : (i+1)*len_part]
                else:
                    l = loudnessValues[i*len_part:]
                l_formatted = str(l).replace('[','').replace(']','').replace(',','')
                # Append this part and add it to the messages list (one message per sound) 
                arguments.append(l_formatted)
                msgs.append(arguments)
                #print_mod(f"Size of the divided string: {actualsize(l_formatted)} bytes (vs. {size_long_string} bytes of the complete string). String itself: {l_formatted}.")
                print_mod(f"Size of the divided message: {actualsize(arguments)} bytes. Initial arguments: {arguments[:4]}.")
                arguments = []

        else:
            arguments.append("New sound")
            arguments.append(row['sound_id'])
            arguments.append(path)
            arguments.append(x)
            arguments.append(y)
            arguments.append(ldns)
            arguments.append(loudnessValues_formatted)
            # One message per sound   
            msgs.append(arguments)
            print_mod(f"Sound packed to be sent. Initial arguments: {arguments[:4]}")
        
    return msgs

def print_mod(string):
    # To print while the server is waiting
    print(str(string))
    sys.stdout.flush()

def download_and_analyze(query, ref_ids_list, query_by_examples):
    # Create the corpus - 25 sounds
    print_mod("\nQuerying sounds...")
    if query_by_examples:
        sounds, grid = interpolation_and_content_based_search(ref_ids_list)
    else:
        sounds = text_query(query)

    # Feature extraction
    print_mod("\nExtracting features...")
    df = feature_extraction.get_audios_and_analysis(sounds, configs.grain_size)
    if query_by_examples:
        df['grid'] = grid
    print_mod("\nFeatures extracted.")

    # Send OSC message to JUCE
    print_mod("\nSending OSC messages...")
    client = SimpleUDPClient(IP_s, PORT_s)
    client.send_message('/juce', "Start")
    msgs = create_arguments_list(df, query_by_examples)
    # Send the message in parts
    total_num_arg = 0
    msg_sizes = []
    if type(msgs[0]) != int:
        for arguments in msgs:
            #print_mod("\nPart:\n",list)
            client.send_message('/juce', arguments)
            total_num_arg += len(arguments)
            msg_sizes.append(actualsize(arguments))

        print_mod(f"Total number of arguments: {total_num_arg}")
        print_mod(f"Total number of messages sent: {len(msgs)}")
        print_mod(f"Sizes of the messages (in chronological order): {msg_sizes} bytes.")
    else:
        #print_mod("n\Whole message:\n",arguments)
        client.send_message('/juce', msgs)
        print_mod("Size in bytes:", actualsize(msgs))
        print_mod("Number of arguments:",len(msgs))

    client.send_message('/juce', "Finished")
    print_mod("\nReady!")

def handle_text_query(*args):
    print_mod("\nText query received: " + str(args))
    # Trigger process
    download_and_analyze(args[1], None, False)

def handle_query_by_examples(*args):
    print_mod("\nQuery by examples received: " + str(args))
    # Create IDs list
    ref_ids_list = [args[1], args[2], args[3], args[4]]
    # Trigger process
    download_and_analyze(None, ref_ids_list, True)

def handle_grain_size(*args):
    print_mod("\nGrain size received: " + str(args))
    # Trigger process
    configs.grain_size = args[1]
    print_mod(f"Grain size changed to: {configs.grain_size}")

# Listen to OSC messages
dispatcher = dispatcher.Dispatcher()
dispatcher.map("/juce/grain", handle_grain_size)
dispatcher.map("/juce/text", handle_text_query)
dispatcher.map("/juce/examples", handle_query_by_examples)
server = osc_server.ThreadingOSCUDPServer((IP_r, PORT_r), dispatcher)
print("Listening OSC messages on {0}".format(server.server_address))
sys.stdout.flush()
server.serve_forever()