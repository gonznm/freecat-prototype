import configs
import corpus_creation
import feature_extraction
from pythonosc.udp_client import SimpleUDPClient
from pythonosc import osc_server
from pythonosc import dispatcher
import sys
import gc

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
    # Create list of arguments to be sent: ids, paths, x, y, targetLoudness, startSamples, loudnessValues
    # Example –> [1234, "/path/audio", 1.5, 1.2, 45.5, "0, 4410, 8820", "48.0,54.2,44.1"]
    arguments_final_list = []  # list of lists
    arguments_aux = []
    for idx, row in df.iterrows():
        # Sound ID
        arguments = []
        arguments.append(row['sound_id'])
        # Path - harcoded to be comatible with docker
        path_docker = row['path']
        path = '/Volumes/Almacen/DirectCode/freecat-prototype/FreeCatPython'+path_docker[4:]
        arguments.append(path)

        # Sound coordinates
        if query_by_examples:
            x = row['grid'][0]
            y = row['grid'][1]
        else:
            x = row['pca_norm_range'][0]
            y = row['pca_norm_range'][1]
        arguments.append(x)
        arguments.append(y)

        # Target loudness
        arguments.append(row['average_loudness'])

        # Grains start samples
        # startSamples_formatted = str(row['grains_start_samples']).replace('[','').replace(']','').replace(',','')
        # arguments.append(startSamples_formatted)

        # Loudness of each grain
        loudnessValues_formatted = str(row['grains_loudness']).replace('[','').replace(']','').replace(',','')
        arguments.append(loudnessValues_formatted)

        # Divide arguments in parts, otherwise the message is too big for UDP
        # Take only part of the sound if the retrieved sound is too long and therefore its size is too big
        # size_single_sound = actualsize(arguments)
        # size_small_args = actualsize(arguments[:4])
        # if size_single_sound> 1024:
        #     print_mod(f"Bytes of the small part: {size_small_args}")
        #     num = size_single_sound/(1024-180- size_small_args)
        #     max_len = int(len(arguments[5])/num)
        #     max_len_ = int(len(arguments[6])/num)
        #     arguments[5] = arguments[5][:max_len]
        #     arguments[6] = arguments[6][:max_len_]
        #     arguments[5] = arguments[5][:arguments[5].rfind(' ')]
        #     arguments[6] = arguments[6][:arguments[6].rfind(' ')]
        #     print_mod(f"Too big sound first: {size_single_sound} bytes. After modification: {actualsize(arguments)} bytes.")

        
        print(f"Size of the current auxiliary list: {actualsize(arguments_aux)} bytes. One sound arguments size: {actualsize(arguments)} bytes.")
        if actualsize(arguments_aux) + actualsize(arguments) > 2048: 
            """Output of command "sysctl -n net.inet.udp.maxdgram": 9216 bytes
            I don't understand why this number of bytes doesn't work, but 2048 works... - it depends on the network I'm connected to, tho?
            I think there is a limit on Juce's side
            """
            #print(f"Auxiliary list ({actualsize(arguments_aux)} bytes) added to final list of arguments. New auxiliary list starts now.")
            arguments_final_list.append(arguments_aux)
            arguments_aux = []

        arguments_aux += arguments

        # Last argument if the list
        if idx == df.shape[0]-1:
            arguments_final_list.append(arguments_aux)

        # If the information doesn't need to be splitted into parts
        if idx == df.shape[0]-1 and not arguments_final_list:
            arguments_final_list = arguments
            print("List len:",len(arguments_final_list))

    return arguments_final_list

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
    arguments = create_arguments_list(df, query_by_examples)
    # Send the message in parts
    total_num_arg = 0
    msg_sizes = []
    if type(arguments[0]) != int:
        for list in arguments:
            #print_mod("\nPart:\n",list)
            client.send_message('/juce', list)
            total_num_arg += len(list)
            msg_sizes.append(actualsize(list))

        print_mod(f"Total number of arguments: {total_num_arg}")
        print_mod(f"Total number of messages sent: {len(arguments)}")
        print_mod(f"Sizes of the messages (in chronological order): {msg_sizes} bytes.")
    else:
        #print_mod("n\Whole message:\n",arguments)
        client.send_message('/juce', arguments)
        print_mod("Size in bytes:",sys.getsizeof(arguments))
        print_mod("Number of arguments:",len(arguments))

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

# Listen to OSC messages
dispatcher = dispatcher.Dispatcher()
dispatcher.map("/juce/text", handle_text_query)
dispatcher.map("/juce/examples", handle_query_by_examples)
server = osc_server.ThreadingOSCUDPServer((IP_r, PORT_r), dispatcher)
print("Listening OSC messages on {0}".format(server.server_address))
sys.stdout.flush()
server.serve_forever()