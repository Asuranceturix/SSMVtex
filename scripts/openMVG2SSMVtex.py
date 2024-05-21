import sys
import os.path
dir = os.path.dirname(__file__)
if not dir in sys.path:
    sys.path.append(dir)
import openmvg


def main():
    imageListFileName = 'PictureNames.txt'
    CameraCalibFileName = 'CameraCalib.txt'
    imagesDir = ''
    argv = sys.argv
    if len(argv) <= 1:
        print('Usage: python3 openMVG2SSMVtex.py <JSON SCENE FILE> | <IMAGES PATH> <CALIBRATION FILE NAME> <LIST FILE NAME> |')
        exit()
    json_path = os.path.abspath(argv[1])
    if not (os.path.exists(json_path) and os.path.isfile(json_path)):
        print("Incorrect path to JSON scene.")
        exit()

    if len(argv) > 2:
        if os.path.exists(argv[2]) and os.path.isdir(argv[2]):
            imagesDir = os.path.abspath(argv[2])
            print('Selected directory containing images:', imagesDir)
        else:
            print('Invalid image directory')
            exit()
        if len(argv) > 3:
            CameraCalibFileName = argv[3]
            if len(argv) > 4:
                imageListFileName = argv[4]


    sfm_data = openmvg.sfm_data(json_path)
    print('JSON Read:', json_path)
    imageListFile = open(imageListFileName, 'w')
    print('TxT Created:', imageListFileName)
    CameraCalibFile = open(CameraCalibFileName, 'w')
    print('TxT Created:', CameraCalibFileName)

    nCams = len(sfm_data['views'])
    print('Reading', nCams, 'Views/Cameras')

    print(nCams, file=CameraCalibFile)

    for rawview in sfm_data['views']:
        view = rawview['value']['ptr_wrapper']['data']
        filename = os.path.join(imagesDir, view['filename'])
        width = view['width']
        height = view['height']
        intrinsics = sfm_data.find_intrinsics(view)
        focal_length = intrinsics['focal_length']
        principal_point = intrinsics['principal_point']
        extrinsics = sfm_data.find_extrinsics(view)
        centre = extrinsics['center']
        rotation = extrinsics['rotation']

        intrinsics_str = '{} 0 {} 0 {} {} 0 0 1'.format(focal_length,
                                                                      principal_point[0],
                                                                      focal_length,
                                                                      principal_point[1])
        dimensions_str = '{} {}'.format(width, height)

        print(filename, file=imageListFile)
        # print(intrinsics_str, file=CameraCalibFile)
        # print(' '.join(str(i) for o in rotation for i in o ), file=CameraCalibFile)
        # print(' '.join(str(e) for e in centre), file=CameraCalibFile)
        # print(dimensions_str, file=CameraCalibFile)

        # imageListFile.write(filename)
        CameraCalibFile.write(intrinsics_str)
        CameraCalibFile.write(' ')
        CameraCalibFile.write(' '.join(str(i) for o in rotation for i in o ))
        CameraCalibFile.write(' ')
        CameraCalibFile.write(' '.join(str(e) for e in centre))
        CameraCalibFile.write(' ')
        CameraCalibFile.write(dimensions_str)
        CameraCalibFile.write('\n')


    imageListFile.close()
    CameraCalibFile.close()

    print('Files generated')


if "__main__" == __name__:
    main()