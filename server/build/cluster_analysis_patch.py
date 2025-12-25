def get_all_recipes():
    # insight_patch_for_recipes
    return []


if __name__ == '__main__':
    # insight_patch_for_cluster_analysis_main
    import multiprocessing as mp
    if platform.system() == 'Darwin':
        mp.set_start_method('fork')
    elif platform.system() == 'Windows':
        mp.freeze_support()
    cluster_analysis_main()