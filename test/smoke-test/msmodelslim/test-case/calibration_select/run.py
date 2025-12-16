import os
import json
from tqdm import tqdm
import pandas as pd

import torch
from ascend_utils.common.security import get_valid_path, get_valid_read_path

from msmodelslim.pytorch.llm_ptq.mix_calibration.calib_select import CalibrationData
from msmodelslim.pytorch.llm_ptq.mix_calibration.dataset_processor_base import DatasetProcessorBase


# 用户自定义校准集处理demo
class CEval0ShotProcessor(DatasetProcessorBase):
    def __init__(self, dataset_path, tokenizer=None, model=None):
        super().__init__(dataset_path, tokenizer, model)
        self.choices = ("A", "B", "C", "D")
        self.shot = 0
        subject_mapping = self.get_subject_mapping()
        for task_name in tqdm(subject_mapping):
            dev_df, val_df = self.load_csv_by_task_name(task_name, self.dataset_path)
            task_len = val_df.shape[0]
            subject_prompt = []

            for i in range(task_len):
                subject_prompt.append(self.format_example(val_df, i, include_answer=False))
                self.ori_answers.append(val_df.iloc[i, len(self.choices) + 1])
            self.ori_prompts.extend([p for p in subject_prompt])
        self.ori_prompts = [prompt.encode().decode(encoding="utf8") for prompt in self.ori_prompts]

    def get_subject_mapping(self):
        subject_mapping_path = os.path.join(self.dataset_path, "subject_mapping.json")
        subject_mapping_path = get_valid_read_path(subject_mapping_path)
        with open(subject_mapping_path) as f:
            subject_mapping = json.load(f)
        return subject_mapping

    def load_csv_by_task_name(self, task_name, dataset_path):
        dev_file_path = get_valid_path(os.path.join(dataset_path, "dev", task_name + "_dev.csv"))
        dev_df = pd.read_csv(dev_file_path, header=None)[:self.shot + 1]
        val_file_path = get_valid_path(os.path.join(dataset_path, "val", task_name + "_val.csv"))
        val_df = pd.read_csv(val_file_path, header=None)

        dev_df = dev_df.iloc[1:, 1:]
        val_df = val_df.iloc[1:, 1:]
        return dev_df, val_df

    def format_example(self, df, idx, include_answer=True):
        prompt = df.iloc[idx, 0]
        k = len(self.choices)
        for j in range(k):
            prompt += "\n{}. {}".format(self.choices[j], df.iloc[idx, j + 1])
        prompt += "\nAnswer:"
        if include_answer:
            prompt += " {}\n\n".format(df.iloc[idx, k + 1])
        return prompt

    def process_data(self, indexs):
        prmpts_anses = []
        for idx in indexs:
            prmpts_anses.append({"prompt": self.ori_prompts[idx], "ans": self.ori_answers[idx]})
        return prmpts_anses

    def verify_positive_prompt(self, prompts, labels):
        prpt_ans = []
        with torch.no_grad():
            inputs = self.tokenizer(prompts, padding=True, return_tensors="pt", truncation=True,
                                    return_token_type_ids=False).to(self.model.device)
            outputs = self.model.generate(**inputs, do_sample=False, max_new_tokens=20)

            answers = []
            for idx in range(len(outputs)):
                output = outputs.tolist()[idx][len(inputs["input_ids"][idx]):]
                response = self.tokenizer.decode(output)
                answers.append(response)
            answers = [answer.lstrip()[0] if answer.lstrip() else "-1" for answer in answers]

            for ans, label, prmpt in zip(answers, labels, prompts):
                if ans == label:
                    prpt_ans.append({"prompt": prmpt, "ans": ans})

        return prpt_ans


if __name__ == "__main__":
    CONFIG_PATH = f"{os.environ['PROJECT_PATH']}/resource/calibration_data/mix_config.json"
    SAVE_PATH = f"{os.environ['PROJECT_PATH']}/output/mix_dataset_customized.json"

    sample_size = {"boolq": 4, "ceval_0_shot": 3}  # 用户自定义数据集

    customized_dataset_path = f"{os.environ['PROJECT_PATH']}/resource/llm_dataset/ceval_5_shot"
    ceval_0_shot_processor = CEval0ShotProcessor(customized_dataset_path, tokenizer=None, model=None)

    calib_select = CalibrationData(config_path=CONFIG_PATH, save_path=SAVE_PATH, tokenizer=None, model=None)
    calib_select.add_custormized_dataset_processor("ceval_0_shot", ceval_0_shot_processor)

    calib_select.set_sample_size(sample_size)
    calib_select.set_batch_size(4)
    calib_select.set_shuffle_seed(1)

    mixed_dataset = calib_select.process()
    print(mixed_dataset)
