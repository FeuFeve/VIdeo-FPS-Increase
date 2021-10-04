import cv2
import fnmatch
import os
import platform
import shutil
import sys
import time

from multiprocessing import Process
from natsort import os_sorted
from sys import argv
from os import path

pretty_string = ""


# FUNCTION DEFINITIONS
def prettyPrint(string, end='\n'):
    if ide != "p":
        print(string, end=end)
    else:
        global pretty_string
        pretty_string += string + end

        if op_system == "Windows":
            os.system("cls")
        else:
            os.system("clear")

        print(pretty_string)


def checkIfExe(filename):
    try:
        os.chmod(filename, 0o777)
    except FileNotFoundError:
        return False

    executable = os.access(filename, os.X_OK)
    return executable


def compileExe():
    prettyPrint("Checking for an executable file...", '')

    filename = "interpolate"
    if op_system == "Windows":
        filename += ".exe"

    already_compiled = checkIfExe(filename)
    if already_compiled:
        prettyPrint(" Found.")
        return True

    prettyPrint(" None found.")

    prettyPrint("Checking for a src directory...", '')
    if not path.exists("src"):
        prettyPrint(" None found, exiting.")
        return False

    prettyPrint(" Found.")

    prettyPrint("Compiling...", '')
    os.system("g++ src/*.cpp -o interpolate")
    prettyPrint(" Done.")
    return True


def clean(dir_converted, dir_interpolated):
    prettyPrint("Cleaning previous results...")
    deleted = False
    try:
        shutil.rmtree(os.path.join(results_path, dir_converted))
        prettyPrint(f"  Deleted {dir_converted} folder.")
        deleted = True
    except FileNotFoundError:
        pass
    try:
        shutil.rmtree(os.path.join(results_path, dir_interpolated))
        prettyPrint(f"  Deleted {dir_interpolated} folder.")
        deleted = True
    except FileNotFoundError:
        pass
    try:
        os.remove(os.path.join(results_path, f"{converted_images_dir}.avi"))
        prettyPrint(f"  Deleted {converted_images_dir}.avi video.")
        deleted = True
    except FileNotFoundError:
        pass
    try:
        os.remove(os.path.join(results_path, f"{dir_interpolated}.avi"))
        prettyPrint(f"  Deleted {dir_interpolated}.avi video.")
        deleted = True
    except FileNotFoundError:
        pass
    if not deleted:
        prettyPrint("  Nothing to clean.")
    prettyPrint("Done.")


def getFrame(video, count, sec, path):
    video.set(cv2.CAP_PROP_POS_MSEC, sec * 1000)
    has_frames, image = video.read()
    if has_frames:
        cv2.imwrite(f"{path}/image{str(count)}.ppm", image)  # save frame as JPG file
    return has_frames


def convertVideoToPpmImages(fps):
    prettyPrint(f"Converting the video to {fps} fps PPM images...", end='')
    video = cv2.VideoCapture(video_name)

    frame_rate = 1 / fps
    sec = 0
    count = 1

    path = os.path.join(results_path, f"video{str(fps)}fps_converted")
    os.mkdir(path)

    success = getFrame(video, count, sec, path)
    while success:
        count = count + 1
        sec = sec + frame_rate
        sec = round(sec, 2)
        success = getFrame(video, count, sec, path)

    prettyPrint(" Done.")
    return path


def convertPpmImagesToVideo(interpolated_video_dir, fps):
    prettyPrint(f"Generating a '.avi' video from the '{interpolated_video_dir}' images...", end='')
    path_in = os.path.join(results_path, interpolated_video_dir)
    path_out = os.path.join(results_path, f"{interpolated_video_dir}.avi")

    # Get filenames
    frame_array = []
    files = os_sorted(os.listdir(path_in))

    # Init height, width, layers and size with the first image
    img = cv2.imread(os.path.join(path_in, files[0]))
    height, width, layers = img.shape
    size = (width, height)

    # reading each files and add them to an image array
    for file in files:
        file_path = os.path.join(path_in, file)
        img = cv2.imread(file_path)
        frame_array.append(img)

    out = cv2.VideoWriter(path_out, cv2.VideoWriter_fourcc(*'DIVX'), fps, size)
    for i in range(len(frame_array)):
        out.write(frame_array[i])
    out.release()
    prettyPrint(" Done.")


def cleanProcesses(processes, finished_frames, frames):
    updated_processes = []
    for j in range(len(processes)):
        processes[j].join(0)
        if processes[j].is_alive():
            updated_processes.append(processes[j])
        else:
            finished_frames += 1
            # prettyPrint(f"# Progress: {round(100 * finished_frames / (len(frames) - 1), 1)}%")

            if finished_frames % 10 == 0 or finished_frames == frames:
                prettyPrint(f" {finished_frames:4}/{frames}")
            else:
                prettyPrint(f" {finished_frames:4}/{frames}", end='')

    return updated_processes, finished_frames


def threadInterpolate2Frames(index, frame1, frame2, op_sys, converted_video_path, interpolated_video_path,
                             added_frames):

    # Launch the executable on the "frame1" & "frame2" to generate "added_frames" new frames in the working directory
    relative_converted_video_path = converted_video_path.replace(os.getcwd() + "\\", '')
    if op_sys == "Windows":
        exe_file = "interpolate.exe"
    else:
        exe_file = "./interpolate"

    os.system(f"{exe_file} {relative_converted_video_path}/{frame2} {relative_converted_video_path}/{frame1} "
              f"thread{os.getpid()}_temp.ppm {added_frames}")

    # Move the newly created frames from the working directory to the final directory, and rename them so they are
    # placed in the right order (eg: "image1a.ppm", "image1b.ppm", ...)
    frame_index = ord('a')
    for other_file in os_sorted(os.listdir(os.getcwd())):
        if fnmatch.fnmatch(other_file, f"*thread{os.getpid()}_temp.ppm"):
            shutil.move(other_file, f"{interpolated_video_path}/image{index}{chr(frame_index)}.ppm")
            frame_index += 1


# Input:
# - converted_video_path: the current working directory's absolute path (eg: C:\Python\Projects\Interpolation)
# - interpolated_video_dir: the name of the directory in which the results will be stored (eg: video30to60fps)
#       (here the final directory would then be located at "C:\Python\Projects\Interpolation\results\video30to60fps)
# - added_frames: the number of frames to create between each already existing frame
#       (eg: if added_frames = 2, "image1a.ppm" and "image1b.ppm" will be created between "image1.ppm" and "image2.ppm"
def generateInterpolatedFrames(converted_video_path, interpolated_video_dir, added_frames):
    file_list = os_sorted(os.listdir(converted_video_path))
    prettyPrint(f"Generating {added_frames} new frames between each {len(file_list)} existing frames for "
                f"'{interpolated_video_dir}'...")
    # Get the absolute path of the directory in which all the frames will be placed, and create it
    interpolated_video_path = os.path.join(results_path, interpolated_video_dir)
    try:
        os.mkdir(interpolated_video_path)
    except FileExistsError:
        pass

    processes = []  # Will contain current working processes
    max_processes = os.cpu_count()
    finished_frames = 0

    # Browse all the converted frames
    frame_id = 0
    previous_frame = ""
    while frame_id < len(file_list):

        frame = file_list[frame_id]

        # Copy the current frame to the final directory
        shutil.copy(f"{converted_video_path}/{frame}", f"{interpolated_video_path}/{frame}")

        if previous_frame == "":
            previous_frame = frame
            frame_id += 1
            continue

        if len(processes) < max_processes:
            process = Process(
                target=threadInterpolate2Frames,
                args=(frame_id, previous_frame, frame, op_system, converted_video_path, interpolated_video_path,
                      added_frames)
            )
            process.daemon = True
            process.start()
            processes.append(process)
            frame_id += 1
        else:
            processes, finished_frames = cleanProcesses(processes, finished_frames, len(file_list) - 1)

        previous_frame = frame

    # Wait for the remaining processes to finished their work
    while len(processes) > 0:
        processes, finished_frames = cleanProcesses(processes, finished_frames, len(file_list) - 1)

    prettyPrint("Done.")
    return interpolated_video_path


# GLOBAL VARIABLES
video_name = "video60fps.mp4"
results_path = os.path.join(os.getcwd(), "results")

# MAIN
if __name__ == '__main__':

    start_time = time.time()

    # Verify if the OS is either Windows or Linux (exit if it's not the case)
    op_system = platform.system()
    if op_system != "Windows" and op_system != "Linux":
        prettyPrint("Only Windows and Linux OS are supported.")
        sys.exit()

    # Verify if the arguments are correct
    fps_from, fps_to, ide = 0, 0, ""
    error = False
    try:
        if len(argv) == 4:
            program_path, fps_from, fps_to, ide = argv
        else:
            program_path, fps_from, fps_to = argv
        fps_from = int(fps_from)
        fps_to = int(fps_to)
        if not (0 < fps_from < fps_to and fps_to % fps_from == 0):
            error = True
    except ValueError:
        error = True

    if error:
        prettyPrint("Missing or incorrect arguments.")
        prettyPrint("Must be \"python3 interpolate.py fps_from fps_to\", with fps_from and fps_to > 0.")
        prettyPrint("fps_to must be a multiple of fps_from and superior to fps_from.")
        prettyPrint("E.g: \"python3 interpolate.py 10 30\"")
        sys.exit()

    if not compileExe():  # Try to compile an executable for the C++ algorithm, exit if there is an error
        exit(-1)

    try:
        os.mkdir("results")
    except FileExistsError:
        pass

    converted_images_dir = f"video{fps_from}fps_converted"
    interpolated_images_dir = f"video{fps_from}to{fps_to}fps"

    # Remove the previous results
    clean(converted_images_dir, interpolated_images_dir)

    # Convert the 60 fps video into ppm images with "fps_from" fps and place them into their associated folder
    # into "results/"
    converted_images_path = convertVideoToPpmImages(fps_from)

    # Execute the algorithm "nb of images - 1" times. Each time, get the resulting files, rename them and place them in
    # their associated "results/" folder
    generateInterpolatedFrames(converted_images_path, interpolated_images_dir, int(fps_to / fps_from) - 1)

    # Assemble the videos
    convertPpmImagesToVideo(converted_images_dir, fps_from)  # For reference
    convertPpmImagesToVideo(interpolated_images_dir, fps_to)  # Results

    prettyPrint(f"Done in: {round(time.time() - start_time, 2)}s.")
