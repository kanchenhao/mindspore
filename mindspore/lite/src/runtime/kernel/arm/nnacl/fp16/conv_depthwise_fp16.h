/**
 * Copyright 2020 Huawei Technologies Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_NNACL_FP16_CONV_DEPTHWISE_FP16_H_
#define MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_NNACL_FP16_CONV_DEPTHWISE_FP16_H_

#include "src/runtime/kernel/arm/nnacl/conv_parameter.h"
#include "src/runtime/kernel/arm/nnacl/fp32/conv_depthwise.h"

void ConvDwC8Fp16(float16_t *output_data, const float16_t *input_data, const float16_t *weight_data,
                  const float16_t *bias_data, const ConvParameter *conv_param, const SlidingWindowParam *sliding,
                  int task_id);

void DeconvDwC8Fp16(float16_t *output_data, const float16_t *input_data, const float16_t *weight_data,
                    const float16_t *bias_data, const ConvParameter *conv_param, const SlidingWindowParam *sliding,
                    int task_id);

#endif  // MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_NNACL_FP16_CONV_DEPTHWISE_FP16_H_
