import configs
import corpus_creation
import feature_extraction
from pythonosc.udp_client import SimpleUDPClient

# Aux boolean
content_based_search = False

# approach 1 config
query='kid'
# approach 2 config
sound_top_left = '357589' # Trumpet - Asharp3
sound_top_right = '328727' #  flute_note_tremolo
sound_bottom_left = '255805' # male voice
sound_bottom_right = '344270' # glass smash, bottle, G
nx = 5 
ny = 5 # total number of sounds (including reference sounds): nx*ny

# OSC config
IP="127.0.0.1"
#IP = "192.168.1.52"
PORT = 9001

def interpolation_and_content_based_search(sound_top_left, sound_top_right, sound_bottom_left, sound_bottom_right):
    ref_ids_list = [sound_top_left, sound_top_right, sound_bottom_left, sound_bottom_right]

    # Get these four reference sounds and their MFCCs
    ref_sounds = corpus_creation.get_sounds_by_ID(ref_ids_list)

    # Interpolation of intermediate MFCCs
    interp_values, grid = corpus_creation.grid_interpolation(ref_sounds, nx, ny)

    # Content based search using MFCCs as targets
    sounds = corpus_creation.get_sounds_by_MFCCs(interp_values, ref_sounds)

    # Download the ogg previews
    for count, sound in enumerate(sounds):
        print('Downloading sound with id {0} [{1}/{2}]'.format(sound.id, count + 1, len(sounds)))
        corpus_creation.retrieve_sound_preview(sound, configs.FILES_DIR)

    return sounds, grid

def text_query(query):
    # Make a text query and get 25 results
    sounds = corpus_creation.query_freesound(query, num_results=25)
    # Download the ogg previews
    for count, sound in enumerate(sounds):
        print('Downloading sound with id {0} [{1}/{2}]'.format(sound.id, count + 1, len(sounds)))
        corpus_creation.retrieve_sound_preview(sound, configs.FILES_DIR)
    return sounds

def create_arguments_list(df):
    # Create list of arguments to be sent: ids, paths, x, y, targetLoudness, startSamples, loudnessValues
    # Example –> [1234, "/path/audio", 1.5, 1.2, 45.5, "0, 4410, 8820", "48.0,54.2,44.1"]
    arguments_part1 = []
    arguments_part2 = []
    arguments = arguments_part1
    for idx, row in df.iterrows():
        # Sound ID
        arguments.append(row['sound_id'])
        # Path - harcoded to be comatible with docker
        path_docker = row['path']
        path = '/Volumes/Almacen/DirectCode/freecat-prototype/FreeCatPython'+path_docker[4:]
        arguments.append(path)

        # Sound coordinates
        if content_based_search:
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
        startSamples_formatted = str(row['grains_start_samples']).replace('[','').replace(']','').replace(',','')
        arguments.append(startSamples_formatted)

        # Loudness of each grain
        loudnessValues_formatted = str(row['grains_loudness']).replace('[','').replace(']','').replace(',','')
        arguments.append(loudnessValues_formatted)

        # Divide arguments in two parts, otherwise it is too big
        if idx > (len(df.index)/2)-1:
            arguments = arguments_part2

    return arguments_part1,arguments_part2

# Create the corpus - 25 sounds
if content_based_search:
    sounds, grid = interpolation_and_content_based_search(sound_top_left, sound_top_right, sound_bottom_left, sound_bottom_right)
else:
    sounds = text_query(query)

# Feature extraction
df = feature_extraction.get_audios_and_analysis(sounds, configs.grain_size)
if content_based_search:
    df['grid'] = grid

# Send OSC message to JUCE
arguments_part1, arguments_part2 = create_arguments_list(df)
print("Number of arguments (part 1 and 2): ", len(arguments_part1), len(arguments_part2))
#print("\nPart 1:",arguments_part1)
#print("\nPart 2:",arguments_part2)
client = SimpleUDPClient(IP, PORT)
#client.send_message('/juce', arguments_part1)
#client.send_message('/juce', arguments_part2)
client.send_message('/juce', "hello")