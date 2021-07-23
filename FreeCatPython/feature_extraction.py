## Functions used in the feature extraction
import pandas as pd
import numpy as np
import os
import essentia.standard as ess
from sklearn.preprocessing import StandardScaler
from sklearn.decomposition import PCA

import configs

def analyze_segments(audio, grain_size):

    if len(audio)<grain_size:
        segment_start_end_samples = [(0,len(audio))]
    else:
      # Calculate the start and end samples for each equally-spaced audio segment
      segment_start_samples = range(0, len(audio), grain_size)
      segment_start_end_samples = zip(segment_start_samples[:-1], segment_start_samples[1:])

    # Iterate over audio frames and analyze each one
    startSamples = []
    loudnessValues = []
    for i, (start, end) in enumerate(segment_start_end_samples):
        # Get corresponding audio segment
        grain = audio[start:end]

        # Extract loudness
        loudness = ess.Loudness()(grain)/grain_size
        loudness_dB = np.around(10*np.log10(loudness), decimals=1)

        # Add analysis results to output lists
        startSamples.append(start)
        loudnessValues.append(loudness_dB)

    return startSamples, loudnessValues

def get_audios_and_analysis(sounds, grain_size):
    """ Load all available analysis and audios into a dataframe.
    """
    df = pd.DataFrame(columns=['sound_id', 'path','average_loudness','grains_start_samples', 'grains_loudness','mfccs_mean', 'loudness_FS','mfcss_normalized','pca', 'pca_norm_range'])
    feats = []
    i = 0
    for sound in sounds:
        if sound.analysis:
            file_path = os.path.join(configs.FILES_DIR, sound.previews.preview_hq_ogg.split('/')[-1])
            # Load audio
            audio = ess.MonoLoader(filename=file_path)()
            # Calculate loudness - in logaritmic scale and normalized
            average_loudness = 10*np.log10(ess.Loudness()(audio)/len(audio))
            # Divide the audio in fixed-size grains and analyze them
            grains_start_samples, grains_loudness = analyze_segments(audio, grain_size)

            # Get Essentia's Freesound extractor precomputed features
            mfcc_mean = sound.analysis.lowlevel.mfcc.mean
            loudness_FS = sound.analysis.lowlevel.average_loudness
            # Store them in a dataframe
            df.loc[i] = [sound.id, file_path, average_loudness, grains_start_samples,  grains_loudness, mfcc_mean, 10*np.log10(loudness_FS), 0, 0, 0]
            feats.append(mfcc_mean)
            i += 1
        else:
            print(f'Sound {sound.id} skipped because the analysis is not available.')

    # Normalize features - make features look like standard normally 
    # distributed data, as PCA maximizes the variance and can be fooled by different scales
    mfcss_norm = StandardScaler().fit_transform(feats)
    for i, feat in enumerate(mfcss_norm):
        df['mfcss_normalized'].loc[i] = feat
    
    # Dimensionality reduction - Principal Components Analysis
    pca = PCA(n_components=2).fit_transform(mfcss_norm)
    # add it to the existing df
    for i, tuple in enumerate(pca):
        df['pca'].loc[i] = tuple

    # Normalize to have coordinates in a range of 
    max_value = np.max(np.max(np.array(pca)))
    #print("Max value of all PCAs:", max_value)
    for i, tuple in enumerate(pca):
        # Normalize by max value and put in the range of 0-1
        df['pca_norm_range'].loc[i] = (np.asarray(tuple)/max_value + 1)/2
    
    print(f'\nTotal number of sounds used at the end: {len(df.index)}')
    return df