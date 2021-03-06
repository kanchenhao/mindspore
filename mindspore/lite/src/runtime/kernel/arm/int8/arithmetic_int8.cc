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

#include "src/runtime/kernel/arm/int8/arithmetic_int8.h"
#include "src/runtime/kernel/arm/nnacl/int8/arithmetic_int8.h"
#include "src/runtime/kernel/arm/nnacl/arithmetic_common.h"
#include "schema/model_generated.h"
#include "src/kernel_registry.h"
#include "src/runtime/runtime_api.h"
#include "include/errorcode.h"

using mindspore::kernel::KERNEL_ARCH::kCPU;
using mindspore::lite::KernelRegistrar;
using mindspore::lite::RET_PARAM_INVALID;
using mindspore::lite::RET_ERROR;
using mindspore::lite::RET_OK;

using mindspore::schema::PrimitiveType_Equal;
using mindspore::schema::PrimitiveType_NotEqual;
using mindspore::schema::PrimitiveType_LessEqual;
using mindspore::schema::PrimitiveType_Greater;
using mindspore::schema::PrimitiveType_GreaterEqual;
using mindspore::schema::PrimitiveType_Less;

namespace mindspore::kernel {
namespace {
int ArithmeticsInt8Launch(int thread_id, LiteParallelGroupEnv *penv, void *cdata) {
  auto arithmetic_kernel = reinterpret_cast<ArithmeticInt8CPUKernel *>(cdata);
  auto error_code = arithmetic_kernel->DoArithmetic(thread_id);
  if (error_code != RET_OK) {
    MS_LOG(ERROR) << "ArithmeticsRun error thread_id[" << thread_id << "] error_code[" << error_code << "]";
    return RET_ERROR;
  }
  return RET_OK;
}
}  // namespace

ArithmeticInt8CPUKernel::~ArithmeticInt8CPUKernel() {
  auto param = reinterpret_cast<ArithmeticParameter *>(opParameter);
  if (!param->broadcasting_) {
    return;
  }
  if (context_->allocator != nullptr) {
    if (tile_data0_ != nullptr) {
      context_->allocator->Free(tile_data0_);
    }
    if (tile_data1_ != nullptr) {
      context_->allocator->Free(tile_data1_);
    }
  } else {
    if (tile_data0_ != nullptr) {
      free(tile_data0_);
    }
    if (tile_data1_ != nullptr) {
      free(tile_data1_);
    }
  }
  tile_data0_ = nullptr;
  tile_data1_ = nullptr;
}

int ArithmeticInt8CPUKernel::Init() {
  switch (opParameter->type_) {
    case PrimitiveType_Equal:
      arithmetic_run_ = ElementEqual;
      break;
    case PrimitiveType_NotEqual:
      arithmetic_run_ = ElementNotEqual;
      break;
    case PrimitiveType_Less:
      arithmetic_run_ = ElementLess;
      break;
    case PrimitiveType_LessEqual:
      arithmetic_run_ = ElementLessEqual;
      break;
    case PrimitiveType_Greater:
      arithmetic_run_ = ElementGreater;
      break;
    case PrimitiveType_GreaterEqual:
      arithmetic_run_ = ElementGreaterEqual;
      break;
    default:
      MS_LOG(ERROR) << "Error Operator type " << opParameter->type_;
      arithmetic_run_ = nullptr;
      return RET_PARAM_INVALID;
  }
  auto data_size = outputs_[0]->Size();
  auto param = reinterpret_cast<ArithmeticParameter *>(opParameter);
  if (param->broadcasting_) {
    if (context_->allocator != nullptr) {
      tile_data0_ = reinterpret_cast<int8_t *>(context_->allocator->Malloc(data_size));
      tile_data1_ = reinterpret_cast<int8_t *>(context_->allocator->Malloc(data_size));
    } else {
      tile_data0_ = reinterpret_cast<int8_t *>(malloc(data_size));
      tile_data1_ = reinterpret_cast<int8_t *>(malloc(data_size));
    }
  } else {
    tile_data0_ = nullptr;
    tile_data1_ = nullptr;
  }
  return RET_OK;
}

int ArithmeticInt8CPUKernel::ReSize() { return RET_OK; }

int ArithmeticInt8CPUKernel::DoArithmetic(int thread_id) {
  auto input0_data = reinterpret_cast<int8_t *>(inputs_[0]->Data());
  auto input1_data1 = reinterpret_cast<int8_t *>(inputs_[1]->Data());
  auto output_data = reinterpret_cast<int8_t *>(outputs_[0]->Data());
  auto element_num = outputs_[0]->ElementsNum();
  auto param = reinterpret_cast<ArithmeticParameter *>(opParameter);
  if (param->broadcasting_ && arithmetic_run_ != nullptr) {
    MS_ASSERT(thread_count_ != 0);
    int stride = UP_DIV(element_num, thread_count_);
    int count = MSMIN(stride, element_num - stride * thread_id);
    if (count <= 0) {
      return RET_OK;
    }

    int error_code = arithmetic_run_(tile_data0_ + stride * thread_id, tile_data1_ + stride * thread_id,
                                     output_data + stride * thread_id, count);
    if (error_code != RET_OK) {
      MS_LOG(ERROR) << "Arithmetic run fail! ret: " << error_code;
      return RET_ERROR;
    }
  } else if (arithmetic_run_ != nullptr) {
    int error_code = arithmetic_run_(input0_data, input1_data1, output_data, element_num);
    if (error_code != RET_OK) {
      MS_LOG(ERROR) << "Arithmetic run fail!ret: " << error_code;
      return RET_ERROR;
    }
  } else {
    MS_LOG(ERROR) << "arithmetic_run function is nullptr!";
    return RET_ERROR;
  }
  return RET_OK;
}

int ArithmeticInt8CPUKernel::Run() {
  auto param = reinterpret_cast<ArithmeticParameter *>(opParameter);
  if (param->broadcasting_) {
    auto input_data0 = reinterpret_cast<int8_t *>(inputs_[0]->Data());
    auto input_data1 = reinterpret_cast<int8_t *>(inputs_[1]->Data());
    TileDimensionsInt8(input_data0, input_data1, tile_data0_, tile_data1_, param);
  }
  int error_code = LiteBackendParallelLaunch(ArithmeticsInt8Launch, this, thread_count_);
  if (error_code != RET_OK) {
    MS_LOG(ERROR) << "Arithmetic launch function fail! ret: " << error_code;
    return RET_ERROR;
  }
  return RET_OK;
}

kernel::LiteKernel *CpuArithmeticInt8KernelCreator(const std::vector<lite::tensor::Tensor *> &inputs,
                                                   const std::vector<lite::tensor::Tensor *> &outputs,
                                                   OpParameter *parameter, const lite::Context *ctx,
                                                   const kernel::KernelKey &desc, const lite::Primitive *primitive) {
  if (parameter == nullptr) {
    MS_LOG(ERROR) << "Input parameter is null!";
    return nullptr;
  }
  auto kernel = new (std::nothrow) ArithmeticInt8CPUKernel(parameter, inputs, outputs, ctx, primitive);
  if (kernel == nullptr) {
    MS_LOG(ERROR) << "Create ArithmeticInt8CPUKernel failed, name: " << parameter->name_;
    return nullptr;
  }
  auto ret = kernel->Init();
  if (ret != RET_OK) {
    MS_LOG(ERROR) << "Init kernel failed, name: " << parameter->name_ << ", type: "
                  << schema::EnumNamePrimitiveType(static_cast<schema::PrimitiveType>(parameter->type_));
    delete kernel;
    return nullptr;
  }
  return kernel;
}

REG_KERNEL(kCPU, kNumberTypeInt8, PrimitiveType_Equal, CpuArithmeticInt8KernelCreator)
REG_KERNEL(kCPU, kNumberTypeInt8, PrimitiveType_NotEqual, CpuArithmeticInt8KernelCreator)
REG_KERNEL(kCPU, kNumberTypeInt8, PrimitiveType_Less, CpuArithmeticInt8KernelCreator)
REG_KERNEL(kCPU, kNumberTypeInt8, PrimitiveType_LessEqual, CpuArithmeticInt8KernelCreator)
REG_KERNEL(kCPU, kNumberTypeInt8, PrimitiveType_Greater, CpuArithmeticInt8KernelCreator)
REG_KERNEL(kCPU, kNumberTypeInt8, PrimitiveType_GreaterEqual, CpuArithmeticInt8KernelCreator)

}  // namespace mindspore::kernel
