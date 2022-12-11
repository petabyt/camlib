import ptpy

camera = ptpy.PTPy()

with camera.session():
    ids = camera.get_storage_ids()[0]
    print(ids, camera.get_storage_info(ids))
    a = (camera.get_object_handles(ids, in_root=True))
    print(a)
    print(camera.get_object_info(a[0]))
