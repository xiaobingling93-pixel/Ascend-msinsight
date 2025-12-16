import os
import xlwt

test_info = {
    'distill_mindspore': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|蒸馏（MindSpore框架）', '必须1', 'ST'],
    'sparse_width': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|宽度扩增模型稀疏训练加速', '必须1', 'ST'],
    'low_rank': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|低秩分解', '必须1', 'ST'],
    'prune_sp': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|剪枝（PyTorch框架&from Torchvision）', '必须1', 'ST'],
    'prune_pytorch': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|剪枝（PyTorch框架）', '必须1', 'ST'],
    'post_yolov': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|训练后量化-推理开发使能组开发（yolov5模型）', '必须1', 'ST'],
    'post_resnet50': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|训练后量化-推理开发使能组开发（resnet50模型）', '必须1', 'ST'],
    'distill_pytorch': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|蒸馏（PyTorch框架）', '必须1', 'ST'],
    'onnx_squant_unet': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|训练后量化-算法部&推理开发使能组共同开发（unet模型）', '必须1', 'ST'],
    'onnx_squant_dynamic': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|训练后量化-算法部&推理开发使能组共同开发（动态swin模型）', '必须1', 'ST'],
    'onnx_squant_yolox': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|训练后量化-算法部&推理开发使能组共同开发（yolov5模型）', '必须1', 'ST'],
    'onnx_squant_basic': ['基础功能（正常用例）', '基础功能测试', '其他；性能', None, '按照需求指标进行用例分解|训练后量化-算法部&推理开发使能组共同开发（resnet50模型）；预测值与真实值的接近程度|onnx模型量化后再推理的预测值与真实值的接近程度；模型推理的吞吐量或时延|onnx模型量化后推理的吞吐量', '必须1', 'ST'],
    'onnx_squant_basic_aok': ['基础功能（正常用例）', '基础功能测试', '其他；性能', None, '按照需求指标进行用例分解|训练后量化-算法部&推理开发使能组共同开发（resnet50模型）；预测值与真实值的接近程度|onnx模型量化后再推理的预测值与真实值的接>近程度；模型推理的吞吐量或时延|onnx模型量化后推理的吞吐量', '必须1', 'ST'],
    'ptq_tools_resnet': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|训练后量化（PyTorch框架）resnet模型', '必须1', 'ST'],
    'ptq_tools_unet': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|训练后量化（PyTorch框架）SD类生成模型', '必须1', 'ST'],
    'llm_ptq': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|大模型量化；预测值与真实值的接近程度|伪量化模型对话效果; 按照需求指标进行用例分解|大模型量化支持antioutlier导出rms bias', '必须1', 'ST'],
    'sparse': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|大模型稀疏量化；预测值与真实值的接近程度|伪稀疏量化模型对话效果', '必须1', 'ST'],
    'sparse_lowbit': ['基础功能（正常用例）', '基础功能测试', '其他；精度', None, '按照需求指标进行用例分解|大模型稀疏量化_lowbit；预测值与真实值的接近程度|伪稀疏量化模型对话效果', '必须1', 'ST'],
    'sparse_depth': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|深度扩增模型稀疏训练加速', '必须1', 'ST'],
    'weight_compression': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|稀疏压缩；按照需求指标进行用例分解|稀疏压缩支持多进程', '必须1', 'ST'],
    'llm_ptq_w8a16_sym': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|大模型W8A16对称量化', '必须1', 'ST'],
    'llm_ptq_w8a16_nsym': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|大模型W8A16非对称量化', '必须1', 'ST'],
    'llm_ptq_w8a8_per_token': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|大模型llm_ptq支持per-token算法', '必须1', 'ST'],
    'llm_ptq_npu': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|大模型量化支持npu', '必须1', 'ST'],
    'llm_ptq_cbq': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|大模型量化支持cbq量化算法', '必须1', 'ST'],
    'llm_ptq_multi_npu': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|量化工具支持多卡校准（稀疏量化、量化、异常值抑制）', '必须1', 'ST'],
    'llm_ptq_awq': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|大模型量化支持awq算法', '必须1', 'ST'],
    'llm_ptq_gptq': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|大模型量化支持gptq算法', '必须1', 'ST'],
    'llm_ptq_hqq': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|大模型量化支持hqq算法', '必须1', 'ST'],
    'llm_ptq_minmax': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|大模型量化支持minmax算法', '必须1', 'ST'],
    'llm_ptq_lowbit': ['基础功能（正常用例）', '基础功能测试', '其他；精度', None, '按照需求指标进行用例分解|大模型量化支持lowbit；按照需求指标进行用例分解|lowbit支持safetensor', '必须1', 'ST'],
    'llm_ptq_kvcache': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim支持kvcache int8量化', '必须1', 'ST'],
    'llm_ptq_auto_optimize_w4a8': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim集成量化自动精度调优框架w4a8', '必须1', 'ST'],
    'llm_ptq_auto_optimize_w8a8': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim集成量化自动精度调优框架w8a8', '必须1', 'ST'],
    'llm_ptq_auto_optimize_w8a16': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim集成量化自动精度调优框架w8a16', '必须1', 'ST'],
    'llm_ptq_lowbit_smooth': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|lowbit支持smooth', '必须1', 'ST'],
    'llm_ptq_w4a16_pergroup': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim量化支持W4A16pergroup量化', '必须1', 'ST'],
    'llm_ptq_bf16': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim量化支持bf16', '必须1', 'ST'],
    'ra_compression_baichuan': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim支持长序列压缩_baichuan', '必须1', 'ST'],
    'ra_compression_llama': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim支持长序列压缩_llama', '必须1', 'ST'],
    'llm_ptq_multi_model': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim支持多模态模型', '必须1', 'ST'],
    'anti_outlier_m2_m4': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|大模型量化离群值抑制m2/m4', '必须1', 'ST'],
    'llm_ptq_w8a16_MOE': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim支持moe模型权重W8A16量化', '必须1', 'ST'],
    'llm_ptq_simulate_tp_fp': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim支持通信int8量化fp16', '必须1', 'ST'],
    'llm_ptq_simulate_tp_w8a8': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim支持通信int8量化w8a8', '必须1', 'ST'],
    'llm_ptq_w8a8_per_token_moe': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim支持moe模型权重W8A8 per-token量化', '必须1', 'ST'],
    'llm_ptq_w8a8_fake_quant': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim支持加载safetensor格式的W8A8权重进行伪量化验证', '必须1', 'ST'],
    'llm_ptq_w4a16_pergroup_gptq': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim W4A16权重量化GPTQ支持per-group量化', '必须1', 'ST'],
    'llm_ptq_w4a16_pergroup_hqq': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim W4A16权重量化HQQ支持per-group量化', '必须1', 'ST'],
    'llm_ptq_w8a16_pergroup_minmax': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim W8A16权重量化MinMax支持per-group量化', '必须1', 'ST'],
    'llm_ptq_w8a16_fake_quant': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim支持加载safetensor格式的W8A16权重进行伪量化验证', '必须1', 'ST'],
    'llm_ptq_w4a16_pergroup_fake_quant': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim支持加载safetensor格式的W4A16权重进行伪量化验证', '必须1', 'ST'],
    'weight_compression_safetensor': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|稀疏压缩；按照需求指标进行用例分解|稀疏压缩支持safetensor', '必须1', 'ST'],
    'llm_ptq_nf4': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim支持QLORA的NF4量化', '必须1', 'ST'],
    'llm_ptq_ZS_3': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|大禹治水300I DUO方案llama2_7b测试', '必须1', 'ST'],
    'llm_ptq_ZS_9': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|大禹治水800I A2方案llama2_7b测试', '必须1', 'ST'],
    'llm_ptq_fa3': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|大模型量化支持fa3算法', '必须1', 'ST'],
    'llm_ptq_llava': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim支持多模态模型llava进行异常值抑制及量化', '必须1', 'ST'],
    'llm_ptq_qwenvl': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim支持多模态模型qwenvl进行异常值抑制及量化', '必须1', 'ST'],
    'quant_opensora_1_2': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim支持多模态生成模型Open-Sora 1.2量化', '必须1', 'ST'],
    'quant_sd3': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim支持多模态生成模型SD3量化', '必须1', 'ST'],
    'quant_opensora_plan_1_2': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim支持多模态生成模型Open-Sora Plan 1.2量化', '必须1', 'ST'],
    'mindspeed_w8a8': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim支持对modellink模型w8a8量化', '必须1', 'ST'],
    'layer_select': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim支持层间混精选层', '必须1', 'ST'],
    'calibration_select': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|msModelslim新增校准集自动混合模块', '必须1', 'ST'],
    'anti_outlier_m6': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|大模型量化离群值抑制m6', '必须1', 'ST'],
    'llm_ptq_mix_cfg': ['基础功能（正常用例）', '基础功能测试', '其他；精度', None, '按照需求指标进行用例分解|modelslim支持INT8混合量化', '必须1', 'ST'],
    'llm_ptq_w4a8_per_token': ['基础功能（正常用例）', '基础功能测试', '其他；精度', None, '按照需求指标进行用例分解|modelslim量化支持W4A8pertoken量化', '必须1', 'ST'],
    'llm_ptq_hooks_hunyuan': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim量化支持Hunyuan-Large模型量化', '必须1', 'ST'],
    'multi_modal_ditcache_osp1.2': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|ditcache验证', '必须1', 'ST'],
    'mllm_ptq_qwen2vl': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|mllm_ptq_qwen2vl验证', '必须1', 'ST'],
    'mllm_ptq_internvl2_8b': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|mllm_ptq_internvl2_8b验证', '必须1', 'ST'],
    'mllm_ptq_internvl2_40b': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|mllm_ptq_internvl2_40b验证', '必须1', 'ST'],
    'llm_ptq_autoawq_w4a16_pergroup64_awq': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim量化权重支持转autoAWQ格式', '必须1', 'ST'],
    'llm_ptq_autogptq_w4a16_pergroup64': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim量化权重支持转autoGPTQ格式', '必须1', 'ST'],
    'mllm_ptq_qwen2_5vl': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|mllm_ptq_qwen2_5vl验证', '必须1', 'ST'],
    'llm_ptq_w4a4': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim量化支持W4A4量化', '必须1', 'ST'],
    'multi_modal_quant_anti_flux': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim量化支持多模态模型FLUX异常值抑制量化', '必须1', 'ST'],
    'multi_modal_quant_timestep_flux': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim量化支持多模态模型FLUX时间步量化', '必须1', 'ST'],
    'multi_modal_quant_fa3_flux': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim量化支持多模态模型FLUX的FA3量化', '必须1', 'ST'],
    'multi_modal_session_quant_osp1_2': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim量化支持多模态统一量化接口的Open-Sora Plan1.2模型量化', '必须1', 'ST'],
    'multi_modal_session_quant_sd3': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim量化支持多模态统一量化接口的SD3模型量化', '必须1', 'ST'],
    'mllm_ptq_qwen2_5vl_w4a8': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|modelslim量化支持qwen2_5vl模型W4A8量化', '必须1', 'ST'],
    'modelslim_v1_qwen3_dense_w8a8_ssz': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|msmodelslim量化支持ssz', '必须1', 'ST'],
    'modelslim_v1_qwen3_dense_w8a8_histogram': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|msmodelslim量化支持histogram', '必须1', 'ST'],
    'modelslim_v1_qwen3_dense_w16a16s': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|msmodelslim量化支持w16a16s', '必须1', 'ST'],
    'modelslim_v1_qwen3_dense_w8a8_itersmooth': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|msmodelslim量化支持itersmooth', '必须1', 'ST'],
    'multimodal_sd_modelslim_v1_wan2_1_w8a8': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|msmodelslim量化支持wan2.1', '必须1', 'ST'],
    'modelslim_v1_qwen3_dense_w8a8_flexsmooth': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|msmodelslim量化支持flexsmooth', '必须1', 'ST'],
    'modelslim_v1_qwen3_dense_w8a8_kvsmooth': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|msmodelslim量化支持kvsmooth', '必须1', 'ST'],
    'modelslim_v1_qwen3_dense_w8a8': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|msmodelslim量化支持w8a8', '必须1', 'ST'],
    'modelslim_v1_qwen3_dense_c8': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|msmodelslim量化支持c8', '必须1', 'ST'],
    'modelslim_v1_qwen3_dense_w4a4_quarot_autoround': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|msmodelslim量化支持w4a4, quarot, autoround（DENSN）', '必须1', 'ST'],
    'modelslim_v1_qwen3_moe_w4a8_per_channel': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|msmodelslim量化支持w4a8 per-channel（MOE）', '必须1', 'ST'],
    'analyze_qwen3_14b': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|msmodelslim量化支持analyze功能', '必须1', 'ST'],
    'app_naive_quantization_qwen2.5_7b_w8a8': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|msmodelslim量化支持v0框架一键量化', '必须1', 'ST'],
    'weight_compression_safetensor_save_now': ['基础功能（正常用例）', '基础功能测试', '其他', None, '按照需求指标进行用例分解|量化后进行稀疏压缩', '必须1', 'ST'],
}


def write_result_data(calculated_xf_style, sheet, xf_style, case_result, case_type):
    i = 1
    case_names = case_result.keys()
    if case_type == "modelslim":
        for case_name in case_names:
            # 处理test_info
            case_data = test_info.get(case_name)
            if case_data is None:
                # 填充默认值或跳过
                print(f"Warning: test_info missing for case {case_name}")
                case_data = ["N/A"] * 7  # 假设每个用例有7个字段
            result = case_result.get(case_name)
            metric_result = result.get("metric_result")
            sheet.write(i, 0, case_name, xf_style)
            sheet.write(i, 1, result.get("success"), xf_style)
            sheet.write(i, 2, case_data[0], xf_style)
            sheet.write(i, 3, case_data[1], xf_style)
            sheet.write(i, 4, case_data[2], xf_style)
            sheet.write(i, 5, case_data[3], xf_style)
            sheet.write(i, 6, case_data[4], xf_style)
            sheet.write(i, 7, case_data[5], xf_style)
            sheet.write(i, 8, case_data[6], xf_style)
            sheet.write(i, 9, result.get("execution_time", "no time"), xf_style)
            i += 1


def create_excel(case_result, excel_name, workspace_path, case_type):
    try:
        work_book = xlwt.Workbook(encoding='utf-8')
        sheet = work_book.add_sheet('acc_fps_result')

        # 设置列宽和行高
        sheet.row(0).height_mismatch = True
        sheet.row(0).height = 20 * 30

        for i in range(10):
            sheet.col(i).width = 256 * 15

        title_style = xlwt.XFStyle()
        title_pattern = xlwt.Pattern()
        title_font = xlwt.Font()
        alignment = xlwt.Alignment()
        borders = xlwt.Borders()
        calculated_xf_style = xlwt.XFStyle()

        # 设置表头背景填充
        title_pattern.pattern = xlwt.Pattern.SOLID_PATTERN
        title_pattern.pattern_fore_colour = 41

        # 设置表头字体
        title_font.bold = True

        # 设置表头对齐方式
        alignment.horz = 0x02
        alignment.vert = 0x01

        # 设置单元格格式
        borders.top = 1
        borders.bottom = 1
        borders.left = 1
        borders.right = 1

        title_style.pattern = title_pattern
        title_style.font = title_font
        title_style.alignment = alignment
        title_style.borders = borders

        xf_style = xlwt.XFStyle()
        xf_style.alignment = alignment
        xf_style.borders = borders

        # 设置计算列属性
        calculated_xf_style.alignment = alignment
        calculated_xf_style.borders = borders
        # 设置单元格格式
        calculated_xf_style.num_format_str = '0.00%'
        # excel表头
        if case_type == "modelslim":
            sheet.write(0, 0, "用例名称", xf_style)
            sheet.write(0, 1, "结果", xf_style)
            sheet.write(0, 2, "用例场景", xf_style)
            sheet.write(0, 3, "用例场景细分", xf_style)
            sheet.write(0, 4, "用例细分1", xf_style)
            sheet.write(0, 5, "用例细分2", xf_style)
            sheet.write(0, 6, "用例描述", xf_style)
            sheet.write(0, 7, "用例重要性", xf_style)
            sheet.write(0, 8, "粒度", xf_style)
            sheet.write(0, 9, "执行耗时", xf_style)
        # 数据写入execl
        write_result_data(calculated_xf_style, sheet, xf_style, case_result, case_type)
        excel_path = os.path.join(workspace_path, excel_name)
        work_book.save(excel_path)
        print("create execl success")
    except Exception as e:
        print("create execl fail: {}".format(e))
