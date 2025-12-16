import argparse
import os
import json
import torch
import torch_npu
import subprocess
from transformers import AutoTokenizer, AutoModelForCausalLM, AutoConfig
from msmodelslim.pytorch.llm_ptq.anti_outlier import AntiOutlierConfig, AntiOutlier
from msmodelslim.pytorch.llm_ptq.llm_ptq_tools import Calibrator, QuantConfig

IN_MODEL_PATH = f"{os.environ['PROJECT_PATH']}/resource/llm_ptq/Qwen2.5-7B-Instruct"
OUT_MODEL_PATH = f"{os.environ['PROJECT_PATH']}/output/llm_ptq_autoawq_w4a16_pergroup64_awq"

config = AutoConfig.from_pretrained(pretrained_model_name_or_path=IN_MODEL_PATH, trust_remote_code=True)
config.num_hidden_layers = 28

model = AutoModelForCausalLM.from_pretrained(
    pretrained_model_name_or_path=IN_MODEL_PATH,
    config=config,
    torch_dtype=torch.bfloat16,
    trust_remote_code=True,
    device_map="auto",
).eval()

tokenizer = AutoTokenizer.from_pretrained(
    pretrained_model_name_or_path=IN_MODEL_PATH,
    config=config,
    trust_remote_code=True
)


def get_calib_dataset(tokenizer, calib_list, device='npu:0'):
    calib_dataset = []
    for calib_data in calib_list:
        inputs = tokenizer(calib_data, return_tensors='pt')
        calib_dataset.append([
            inputs.data['input_ids'].to(device),
            inputs.data['attention_mask'].to(device)
        ])
    return calib_dataset


calib_set = ["Where is the capital of China?",
             "Please make a poem:",
             "I want to learn python, how should I learn it?",
             "Please help me write a job report on large model inference optimization:",
             "What are the most worth visiting scenic spots in China?"]
dataset_calib = get_calib_dataset(tokenizer, calib_set, model.device)

anti_config = AntiOutlierConfig(w_bit=4, a_bit=16, anti_method="m3", dev_type="npu", dev_id=model.device.index)
anti_outlier = AntiOutlier(model, calib_data=dataset_calib, cfg=anti_config)
anti_outlier.process()
print('anti_outlier success!')

disable_names = []
quant_config = QuantConfig(
    a_bit=16,
    w_bit=4,
    disable_names=[],
    dev_type='npu',
    dev_id=model.device.index,
    w_sym=True,
    mm_tensor=False,
    w_method='MinMax',
    is_lowbit=True,
    open_outlier=False,
    group_size=64
)
calibrator = Calibrator(model, quant_config, calib_data=dataset_calib, disable_level='L0')
calibrator.run()
calibrator.save(OUT_MODEL_PATH, save_type=["safe_tensor"])
print('Save quant weight success!')

prompt = "Who is the author of Romeo and Juliet?"
messages = [
    {"role": "system", "content": "You are Qwen, created by Alibaba Cloud. You are a helpful assistant."},
    {"role": "user", "content": prompt}
]
text = tokenizer.apply_chat_template(
    messages,
    tokenize=False,
    add_generation_prompt=True
)
model_inputs = tokenizer([text], return_tensors="pt").to(model.device)

generated_ids = model.generate(
    **model_inputs,
    max_new_tokens=512
)
generated_ids = [
    output_ids[len(input_ids):] for input_ids, output_ids in zip(model_inputs.input_ids, generated_ids)
]

response = tokenizer.batch_decode(generated_ids, skip_special_tokens=True)[0]
print(response)

MSMODELSLIM_SOURCE_DIR = os.getenv('MSMODELSLIM_SOURCE_DIR', f"{os.environ['PROJECT_PATH']}/resource/msit/msmodelslim")
print(f"获取到代码仓路径: {MSMODELSLIM_SOURCE_DIR}")

MS_TO_VLLM_PATH = os.path.join(MSMODELSLIM_SOURCE_DIR, 'example/ms_to_vllm.py')
print(f"获取到转换脚本路径: {MS_TO_VLLM_PATH}")

# 执行转换
trans_model = os.path.join(OUT_MODEL_PATH, 'quant_model_weight_w4a16.safetensors')
trans_json = os.path.join(OUT_MODEL_PATH, 'quant_model_description_w4a16.json')
trans_save_path = os.path.join(OUT_MODEL_PATH, 'res.safetensors')
trans_w_bit = '4'
trans_target_tool = "awq"

result = subprocess.run(['python', MS_TO_VLLM_PATH,
                         '--model', trans_model,
                         '--json', trans_json,
                         '--save_path', trans_save_path,
                         '--w_bit', trans_w_bit,
                         '--target_tool', trans_target_tool
                         ], capture_output=True, text=True)
if result.returncode == 0:
    print("命令执行成功")
    print(result.stdout)
else:
    print("命令执行出错")
    print(result.stderr)
# 判断文件是否存在
if not os.path.exists(trans_save_path) or not os.path.isfile(trans_save_path):
    raise "返回结果中预期有文件 res.safetensors ，现在未找到。"
