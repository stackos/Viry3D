import os

if __name__ == '__main__':
    os.system(r'gyp --depth=. --generator-output=./ --f=xcode -D OS="ios" -D enable_data_logging=0 viry3d.gyp')
