import os
import shutil
import glob


def remove_path_safety(path: str):
    """
    Function Description:
        remove path safety
    Parameter:
        path: the path to remove
    Exception Description:
        when invalid data throw exception
    """
    if not os.path.exists(path):
        print(f"Remove {path} not exist, remove faild")
        return

    if os.path.islink(path):
        msg = f"Failed to remove path: {path}, is a soft link"
        print(msg)
        return

    try:
        if os.path.isfile(path):
            os.remove(path)
        elif os.path.isdir(path):
            shutil.rmtree(path)
    except PermissionError as err:
        print(f"Permission denied while removing path: {path}")
        return
    except Exception as err:
        print(f"Failed to remove path: {path}, err: {err}")
        return


def simplify_profiler_data(msprof_profiler_path: str):
    """
    Simplify the profiler data.
    """
    db_path = glob.glob(f"{msprof_profiler_path}/*.db")[0]
    msprof_device_path = glob.glob(f"{msprof_profiler_path}/device_*")[0]
    SIMPLIFY_CACHE = (
        # PROF_XXX/mindstudio_profiler_output
        os.path.join(msprof_profiler_path, "mindstudio_profiler_output"),
        # PROF_XXX/mindstudio_profiler_log
        os.path.join(msprof_profiler_path, "mindstudio_profiler_log"),
        # PROF_XXX/host/sqlite
        os.path.join(msprof_profiler_path, "host", "sqlite"),
        # PROF_XXX/host/data/all_file.complete
        os.path.join(msprof_profiler_path, "host", "data", "all_file.complete"),
        # PROF_XXX/device_x/sqlite
        os.path.join(msprof_device_path, "sqlite"),
        # PROF_XXX/device_x/data/all_file.complete
        os.path.join(msprof_device_path, "data", "all_file.complete"),
        db_path
    )

    for cache_path in SIMPLIFY_CACHE:
        remove_path_safety(cache_path)
