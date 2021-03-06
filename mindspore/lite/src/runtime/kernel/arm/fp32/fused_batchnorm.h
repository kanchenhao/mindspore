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

#ifndef MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_FP32_FUSED_BATCHNORM_H_
#define MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_FP32_FUSED_BATCHNORM_H_

#include <vector>
#include "src/lite_kernel.h"
#include "src/runtime/kernel/arm/nnacl/fused_batchnorm.h"

namespace mindspore::kernel {
class FusedBatchnormCPUKernel : public LiteKernel {
 public:
  FusedBatchnormCPUKernel(OpParameter *parameter, const std::vector<lite::tensor::Tensor *> &inputs,
                          const std::vector<lite::tensor::Tensor *> &outputs, const lite::Context *ctx,
                          const lite::Primitive *primitive)
      : LiteKernel(parameter, inputs, outputs, ctx, primitive) {
    fused_batchnorm_param_ = reinterpret_cast<FusedBatchNormParameter *>(parameter);
  }
  ~FusedBatchnormCPUKernel() override { delete fused_batchnorm_param_; }

  int Init() override;
  int ReSize() override;
  int Run() override;

 private:
  int *input_shape_{};
  FusedBatchNormParameter *fused_batchnorm_param_;
};
}  // namespace mindspore::kernel

#endif  // MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_FP32_FUSED_BATCHNORM_H_
