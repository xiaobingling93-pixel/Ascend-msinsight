import os
import shutil


class PathManager:
    DATA_DIR_AUTHORITY = 0o750

    @classmethod
    def remove_path_safety(cls, path: str):
        if not os.path.exists(path):
            return
        base_name = os.path.basename(path)
        msg = f"Failed to remove path: {base_name}"
        cls.check_path_writeable(path)
        if os.path.islink(path):
            raise RuntimeError(msg)
        if os.path.exists(path):
            try:
                shutil.rmtree(path)
            except Exception as err:
                raise RuntimeError(msg) from err

    @classmethod
    def make_dir_safety(cls, path: str):
        base_name = os.path.basename(path)
        msg = f"Failed to make directory: {base_name}"
        if os.path.islink(path):
            raise RuntimeError(msg)
        if os.path.exists(path):
            return
        try:
            os.makedirs(path, mode=cls.DATA_DIR_AUTHORITY)
        except Exception as err:
            raise RuntimeError(msg) from err

    @classmethod
    def get_realpath(cls, path: str) -> str:
        if os.path.islink(path):
            msg = f"Invalid input path which is a soft link."
            raise RuntimeError(msg)
        return os.path.abspath(path)

    @classmethod
    def check_path_writeable(cls, path):
        """
        Function Description:
            check whether the path is writable
        Parameter:
            path: the path to check
        Exception Description:
            when invalid data throw exception
        """
        if os.path.islink(path):
            msg = f"Invalid path which is a soft link."
            raise RuntimeError(msg)
        base_name = os.path.basename(path)
        if not os.access(path, os.W_OK):
            msg = f"The path permission check failed: {base_name}"
            raise RuntimeError(msg)
