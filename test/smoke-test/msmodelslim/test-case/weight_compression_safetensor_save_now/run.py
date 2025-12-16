import json
import os
import torch
import torch.utils.data
import torch_npu
from transformers import AutoTokenizer, AutoModelForCausalLM
from safetensors.torch import load_file
from msmodelslim.pytorch.weight_compression import CompressConfig, Compressor
from msmodelslim.pytorch.llm_ptq.llm_ptq_tools import QuantConfig, Calibrator

SEQ_LEN_OUT = 12

load_path = f"{os.environ['PROJECT_PATH']}/resource/llm_ptq/llama3-8b/"
tokenizer = AutoTokenizer.from_pretrained(load_path, trust_remote_code=True, local_files_only=True,
                                          use_fast=False)

model = AutoModelForCausalLM.from_pretrained(load_path,
                                             trust_remote_code=True,
                                             torch_dtype=torch.float16).npu()

calib_list = ["Where is the capital of China?"]


def get_calib_dataset(tokenizer, calib_list):
    calib_dataset = []
    for calib_data in calib_list:
        inputs = tokenizer([calib_data], return_tensors='pt')
        print(inputs)
        calib_dataset.append([inputs.data['input_ids'].npu(), inputs.data['attention_mask'].npu()])
    return calib_dataset


dataset_calib = get_calib_dataset(tokenizer, calib_list)

quant_config = QuantConfig(w_bit=4,
                           do_smooth=False,
                           dev_type='npu',
                           is_lowbit=True,
                           use_sigma=True,
                           )
calibrator = Calibrator(model, quant_config, calib_data=dataset_calib, disable_level='L2')
calibrator.run()
calibrator.save(f"{os.environ['PROJECT_PATH']}/output/weight_compression_safetensor_save_now",
                save_type=['safe_tensor'])
print('Save quant weight success!')

# 准备待压缩权重文件和相关压缩配置，请根据实际情况进行修改
LOAD_PATH = f"{os.environ['PROJECT_PATH']}/output/weight_compression_safetensor_save_now"
weight_path = os.path.join(LOAD_PATH, "quant_model_weight_w8a8s.safetensors")  # 待压缩权重文件的路径
json_path = os.path.join(LOAD_PATH, "quant_model_description_w8a8s.json")  # 待压缩权重文件的描述文件的路径

# 使用CompressConfig接口，配置压缩参数，并返回配置实例
compress_config = CompressConfig(do_pseudo_sparse=False, sparse_ratio=1, is_debug=True,
                                 record_detail_root=LOAD_PATH, multiprocess_num=8)

sparse_weight = load_file(weight_path)
with open(json_path, 'r') as f:
    quant_model_description = json.load(f)

# 使用Compressor接口，输入加载的压缩配置和待压缩权重文件
compressor = Compressor(compress_config, weight=sparse_weight, quant_model_description=quant_model_description)
compress_weight, compress_index, compress_info = compressor.run()

# 使用export_safetensors()接口，保存压缩后的结果文件
compressor.export_safetensors(LOAD_PATH, safetensors_name=None, json_name=None)
