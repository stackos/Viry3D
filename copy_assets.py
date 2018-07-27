import os
import shutil

def get_files(dir):
    files = []
    dirs = []
    names = os.listdir(dir)
    for i in range(0, len(names)):
        name = (dir + '/' + names[i]).replace('\\', '/')
        if os.path.isfile(name):
            if not name.endswith('.cache'):
                files.append(name)
        elif os.path.isdir(name):
            dirs.append(name)
    for i in range(0, len(dirs)):
        files = files + get_files(dirs[i])
    return files

def copy_assets(src, dest):
    assets = []
    files = get_files(src)
    for i in range(0, len(files)):
        file = files[i]
        asset = file[len(src) + 1:]
        dest_file = dest + '/' + asset
        dir, name = os.path.split(dest_file)
        if not os.path.exists(dir):
            os.makedirs(dir)
        if os.path.exists(dest_file):
            os.remove(dest_file)
        shutil.copyfile(file, dest_file)
        assets.append("Assets/" + asset)
    return assets

if __name__ == '__main__':
    assets = copy_assets('app/bin/Assets', 'app/project/android/app/src/main/assets/Assets')
    file_list = open("app/project/android/app/src/main/assets/file_list.txt", "w")
    for i in range(0, len(assets)):
        file_list.write(assets[i] + '\n')
    file_list.close()
