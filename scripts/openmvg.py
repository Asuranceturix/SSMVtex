import json
import collections

class sfm_data:
    def __init__(self, filename):
        with open(filename) as fin:
            self.sfm = json.load(fin, object_pairs_hook=collections.OrderedDict)

    def save(self, filename):
        with open(filename, "w") as fout:
            json.dump(self.sfm, fout, indent="    ")

    def __getitem__(self, item):
        return self.sfm[item]

    def find_view(self, filename):
        return next((x['value']['ptr_wrapper']['data'] for x in self.sfm['views'] if
                     x['value']['ptr_wrapper']['data']['filename'] in filename), None)

    def find_intrinsics(self, view):
        intrinsics_id = view['id_intrinsic']
        return next((x['value']['ptr_wrapper']['data'] for x in self.sfm['intrinsics'] if x['key'] == intrinsics_id),
                    None)

    def find_extrinsics(self, view):
        extrinsics_id = view['id_pose']
        extr = next((x for x in self.sfm['extrinsics'] if x['key'] == extrinsics_id), None)
        if extr is None:
            rc = collections.OrderedDict([('rotation', [[1.0, 0.0, 0.0],
                                                        [0.0, 1.0, 0.0],
                                                        [0.0, 0.0, 1.0]]),
                                          ('center', [0.0, 0.0, 0.0])])
            extr = collections.OrderedDict([('key', extrinsics_id), ('value', rc.copy())])
            self.sfm['extrinsics'].append(extr)
        return extr['value']
