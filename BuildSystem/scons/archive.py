#!/usr/bin/env python

import os
import shutil
import sys
import importlib
import tarfile
#import requests

"""
Archive utilities code for SCons projects

Some utility functions for downloading and unpacking .tar.gz and .zip archives
and applying unified diffs as patches
"""

sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'techtonik'))
#print(os.path.join(os.path.realpath(__file__), 'techtonik'))
patch = importlib.import_module('patch')
wget = importlib.import_module('wget')

# ----------------------------------------------------------------------------------------------- #

def split_lines(text_file_contents):
    """Splits the contents of a text file into individual lines. This will work
    on both Windows CR-LF line endings and rest-of-the-world LF line endings.
    It is necessary because Python assumes the format of the platform it's running
    on, which, however, will not work if you share working copies between Windows
    and Linux.

    @param  text_file_contents  Text file contents that will be split
    @returns An array with the indiviual lines from the text file"""

    return list(
        filter(
            len,
            text_file_contents.replace('\r', '').split('\n')
        )
    )

# ----------------------------------------------------------------------------------------------- #

def _move_files_in_subdirectory_level(scan_directory, target_directory, depth):
    """Moves all files below a certain subdirectory level of the scan directory into
    the specified target directory. Used to emulate tar's 'strip-components' behavior.

    @param  scan_directory    Directory that will be scanned for files to move
    @param  target_directory  Directory into which the files and directories will be moved
    @param  depth             Depth of the contents that will be moved to the target"""

    files = os.listdir(scan_directory)

    if (depth == 0):
        for file in files:
            source_path = os.path.join(scan_directory, file)
            target_path = os.path.join(target_directory, os.path.basename(file))
            target_directory = os.path.join(target_directory, str()) # Add trailing slash

            # If the target file or directory already exists, kill it
            if os.path.exists(target_path):
                print('Target ' + target_path + ' already exists, deleting...')
                if os.path.isdir(target_path):
                    shutil.rmtree(target_path)
                else:
                    os.remove(target_path)

            #print("Moving " + source_path + " to " + target_directory)
            shutil.move(source_path, target_directory)
    else:
        for file in files:
            file_path = os.path.join(scan_directory, file)
            if os.path.isdir(file_path):
                _move_files_in_subdirectory_level(file_path, target_directory, depth - 1)

# ----------------------------------------------------------------------------------------------- #

def download_url_in_urlfile(target, source, env):
    """Downloads from an url contained in an url list file. The first working download
    will be saved into the target filename

    @param  target  Expected to contain only one file, the target file
    @param  source  Expected to contain only one file, the url list file
    @param  env     SCons build environment"""

    urls = split_lines(source[0].get_text_contents())
    for url in urls:
        try:
            wget.download(url, out = str(target[0]), bar = None)
            if os.path.isfile(str(target[0])):
                return
        except:
            print('Download from ' + url + ' failed!')

        # If this is a page (most lkely, an error page), it's not what we're looking for
        #request = requests.head(url, allow_redirects = True)

        #request = requests.get(url, allow_redirects = True)
        #content_type = request.headers.get('content-type')
        #if 'text' in content_type.lower():
        #    continue
        #if 'html' in content_type.lower():
        #    continue

        #target_file = open(str(target[0]), 'wb')
        #target_file.write(request.content)
        #target_file.close()

    raise FileNotFoundError(
        'Could not download file ' + str(target[0])
    )

# ----------------------------------------------------------------------------------------------- #

def extract_compressed_tarball(
    tarball_path, target_directory, strip_components = 0
):
    """Extracts a .tar.gz archive using only Python code (thus allowing it to function
    even on Microsoft systems without tar and gzip). In essence, this method is identical
    to 'tar --extract --gzip --strip-components=1 --file=<tarball> --directory=<target>'.

    @param  tarball_path      Path of the .tar.gz file that will be extracted
    @param  target_directory  Directory into which the contents will be extracted
    @param  strip_components  Number of directory levels to ignore when extracting
                              (i.e. 1 if the .tar.gz contains only a dir at the top level)"""

    if not os.path.isdir(target_directory):
        os.mkdir(target_directory)

    temporary_directory = os.path.join(
        os.path.dirname(os.path.abspath(target_directory)), '_tmp'
    )

    tar_archive = tarfile.open(tarball_path)
    tar_archive.extractall(path=temporary_directory)
    tar_archive.close()

    _move_files_in_subdirectory_level(temporary_directory, target_directory, strip_components)

    shutil.rmtree(temporary_directory)

# ----------------------------------------------------------------------------------------------- #

def apply_patch(patchfile_path, target_directory = None):
    """Applies a patch in unified diff format (generated by many VCS systems like
    Subversion or Git and easily produced with standard Unix tools)

    @param  patchfile_path    Path to the unified diff file containing patching instructions
    @param  target_directory  Base directory the patch will be applied in"""

    patchset = patch.fromfile(patchfile_path)

    if target_directory is None:
        patchset.apply()
    else:
        patchset.apply(root = target_directory)

# ----------------------------------------------------------------------------------------------- #
