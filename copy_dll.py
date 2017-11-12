import os
import shutil
import sys

if __name__ == '__main__':
    dir, name = os.path.split(__file__)
    os.chdir(dir)
    if len(sys.argv) >= 2:
        arch = sys.argv[1]
        shutil.copyfile('lib/src/ffmpeg/bin/' + arch + '/avcodec-57.dll', 'app/bin/avcodec-57.dll')
        shutil.copyfile('lib/src/ffmpeg/bin/' + arch + '/avformat-57.dll', 'app/bin/avformat-57.dll')
        shutil.copyfile('lib/src/ffmpeg/bin/' + arch + '/avutil-55.dll', 'app/bin/avutil-55.dll')
        shutil.copyfile('lib/src/ffmpeg/bin/' + arch + '/swresample-2.dll', 'app/bin/swresample-2.dll')
        shutil.copyfile('lib/src/ffmpeg/bin/' + arch + '/swscale-4.dll', 'app/bin/swscale-4.dll')        
