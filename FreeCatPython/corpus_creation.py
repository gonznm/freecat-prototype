## Functions used in the corpus creation
import numpy as np
import pandas as pd
import os
import freesound
from scipy.interpolate import griddata

import configs

freesound_client = freesound.FreesoundClient()
freesound_client.set_token(configs.FREESOUND_API_KEY)
if not os.path.exists(configs.FILES_DIR): os.mkdir(configs.FILES_DIR)

def query_freesound(query, num_results=1):
    """Queries freesound with the given query and filter values.
    It requests the specified analysis descriptors of each sound.
    """
    pager = freesound_client.text_search(
        query = query,
        filter = configs.FREESOUND_METADATA_FILTER,
        fields = ','.join(configs.FREESOUND_METADATA_FIELDS),
        descriptors = ','.join(configs.FREESOUND_METADATA_DESCRIPTORS),
        page_size = num_results
    )

    sounds = [sound for sound in pager]
    if len(sounds)<24: # minimum 25 results to consider the query
        return False
    else:
        return sounds

def retrieve_sound_preview(sound, directory):
    """ Download the high-quality (compared to the MP3 preview) OGG 
    sound preview of a given Freesound sound object to the given directory.
    """
    return freesound.FSRequest.retrieve(
        sound.previews.preview_hq_ogg,
        freesound_client,
        os.path.join(directory, sound.previews.preview_hq_ogg.split('/')[-1])
    )

def get_sounds_by_ID(ids_list):
    """ Given a list of sound IDs from Freesound, it returns a list of sound 
    objects with the specified fields and descriptors.
    """
    sounds = []
    for id in ids_list:
        sound = freesound_client.get_sound(
            id,
            fields = ','.join(configs.FREESOUND_METADATA_FIELDS),
            descriptors = ','.join(configs.FREESOUND_METADATA_DESCRIPTORS)
        )
        sounds.append(sound)
    return sounds

def grid_interpolation(ref_sounds, nx, ny):
    # MFCC coefficients of the reference sounds
    known_values = np.asarray([sound.analysis.lowlevel.mfcc.mean for sound in ref_sounds])
    # Fixed positions of the reference sounds on the corners of the quadrilateral
    known_points = np.array([(1, 0),(1, 1),(0, 0),(0, 1)]) 

    # Build grid of (nx,ny) points
    x = np.linspace(0, 1, nx)
    y = np.linspace(0, 1, ny)
    xv, yv = np.meshgrid(x, y)
    xv = xv.flatten()
    yv = yv.flatten()
    grid = []
    interp_points = []
    for i in range(len(xv)):
        grid.append([xv[i], yv[i]])
        # The interpolation points should not include the corners themselves
        if xv[i]==known_points[0][0] and yv[i]==known_points[0][1] \
          or xv[i]==known_points[1][0] and yv[i]==known_points[1][1] \
          or xv[i]==known_points[2][0] and yv[i]==known_points[2][1] \
          or xv[i]==known_points[3][0] and yv[i]==known_points[3][1]:
            continue
        else:
          interp_points.append([xv[i], yv[i]])

    # Make the interpolation
    interp_values = griddata(known_points, known_values, np.asarray(interp_points), method='linear', fill_value=0)
    return interp_values, grid

def gen_RMS_grid(nx, ny):
    # Build grid of (nx,ny) points
    x = np.linspace(0, 1, nx)
    y = np.linspace(0, 1, ny)
    xv, yv = np.meshgrid(x, y)
    xv = xv.flatten()
    yv = yv.flatten()
    grid = []
    grid_single_values = []
    for i in range(len(xv)):
        grid.append([xv[i], yv[i]])
        grid_single_values.append( np.sqrt((xv[i]+yv[i])**2) )

    d = {'grid': grid, 'single_val': grid_single_values}
    grid_df = pd.DataFrame(data=d)

    # Return the grid sorted by the RMS value of each pair of coordinates
    return grid_df.sort_values(by=['single_val'])['grid'].values

def get_sounds_by_MFCCs(mfccs_array, avoid_sounds):
    """ Performs a content based search using MFCCs as targets – one result per search.
    :mfccs_array: arrays with MFCCs coefficients (13 coefficients per sound).
    :avoid_sounds: sounds that we don't want to get returned by the content based search.
    """
    sounds = []
    for mfccs in mfccs_array:
        mfccs_str = ','.join(map(str, mfccs.tolist()))
        pager = freesound_client.content_based_search(
            target = f'lowlevel.mfcc.mean:{mfccs_str}',
            descriptors_filter = configs.FREESOUND_DESCRIPTORS_DURATION_FILTER,
            fields = ','.join(configs.FREESOUND_METADATA_FIELDS),
            descriptors = ','.join(configs.FREESOUND_METADATA_DESCRIPTORS),
            page_size = 2
        )
        results = [sound for sound in pager]

        # Avoid getting the same sounds we are using as reference
        if results[0] in avoid_sounds:
            sounds.append(results[1])
        else:
            sounds.append(results[0])

    # Return sounds in the correct order
    return [avoid_sounds[0]] + sounds[0:3] + [avoid_sounds[1]] + sounds[3:18] + [avoid_sounds[2]] + sounds[18:] + [avoid_sounds[3]]