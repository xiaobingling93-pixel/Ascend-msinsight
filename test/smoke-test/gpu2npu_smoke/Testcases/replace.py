import os

def replace_string_in_file(file_path, old_string, new_string):

    with open(file_path, 'r', encoding='utf-8') as file:
        filedata = file.read()


    filedata = filedata.replace(old_string, new_string)


    with open(file_path, 'w', encoding='utf-8') as file:
        file.write(filedata)

def replace_in_files(directory, file_extension, old_string, new_string):
    for filename in os.listdir(directory):
        if filename.endswith(file_extension):
            file_path = os.path.join(directory, filename)
            replace_string_in_file(file_path, old_string, new_string)
            print(f"Updated file: {file_path}")


directory_path = '/home/l30044004/smoke_gpu2npu/gpu2npu_smoke-master/Testcases/gpu2npu/'
old_string = 'gpu2npu_path=/root/msfmktransplt_master_0205/src/ms_fmk_transplt'
new_string = 'gpu2npu_path=/home/l30044004/smoke_gpu2npu/ms_fmk_transplt'


replace_in_files(directory_path, '.sh', old_string, new_string)
