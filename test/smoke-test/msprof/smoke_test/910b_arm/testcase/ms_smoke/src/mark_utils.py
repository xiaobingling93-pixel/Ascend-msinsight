""" define marks """
import pytest
from random_generator import set_numpy_global_seed


def arg_mark(plat_marks, level_mark, card_mark, essential_mark):
    optional_plat_marks = ['platform_ascend', 'platform_ascend910b', 'platform_gpu', 'cpu_linux', 'cpu_windows',
                           'cpu_macos']
    optional_level_marks = ['level0', 'level1', 'level2', 'level3', 'level4']
    optional_card_marks = ['onecard', 'allcards', 'dryrun', 'dryrun_only']
    optional_essential_marks = ['essential', 'unessential']
    if not plat_marks or not set(plat_marks).issubset(set(optional_plat_marks)):
        raise ValueError("wrong plat_marks values")
    if level_mark not in optional_level_marks:
        raise ValueError("wrong level_mark value")
    if card_mark not in optional_card_marks:
        raise ValueError("wrong card_mark value")
    if essential_mark not in optional_essential_marks:
        raise ValueError("wrong essential_mark value")

    def decorator(func):
        for plat_mark in plat_marks:
            func = getattr(pytest.mark, plat_mark)(func)
        func = getattr(pytest.mark, level_mark)(func)
        func = getattr(pytest.mark, card_mark)(func)
        func = getattr(pytest.mark, essential_mark)(func)
        set_numpy_global_seed()
        return func

    return decorator
