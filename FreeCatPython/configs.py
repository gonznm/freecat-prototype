# Global configs
FREESOUND_API_KEY = 'M7lHLqRJs0jWdIp9b4jVLnhyFoG3MciqO2Tv3ku3'  # Personal Freesound API key
FILES_DIR = '/tmp/sounds/'  # Folder to store the downloaded files. Will be relative to the current folder.
FREESOUND_METADATA_FIELDS = ['id', 'name', 'username', 'previews', 'license', 'tags', 'analysis', 'analysis_frames']  # Freesound metadata properties to store
FREESOUND_METADATA_FILTER = 'duration:[0.5 TO 5]'
FREESOUND_DESCRIPTORS_DURATION_FILTER = 'sfx.duration:[0.5 TO 5]'
FREESOUND_METADATA_DESCRIPTORS = ['lowlevel.mfcc.mean', 'lowlevel.average_loudness']
grain_size = 4410 # 100 ms in samples (fs=44100)