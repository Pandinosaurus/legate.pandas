/* Copyright 2021 NVIDIA Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <unordered_map>

#include "category/tasks/encode.h"
#include "category/encode.h"
#include "cudf_util/allocators.h"
#include "cudf_util/column.h"
#include "util/gpu_task_context.h"

namespace legate {
namespace pandas {
namespace category {

/*static*/ void EncodeTask::gpu_variant(const Legion::Task *task,
                                        const std::vector<Legion::PhysicalRegion> &regions,
                                        Legion::Context context,
                                        Legion::Runtime *runtime)
{
  Deserializer ctx{task, regions};

  EncodeTaskArgs args;
  deserialize(ctx, args);

  const size_t size = args.in.num_elements();
  if (size == 0) {
    args.out.make_empty();
    return;
  }

  GPUTaskContext gpu_ctx{};
  auto stream = gpu_ctx.stream();

  auto in         = to_cudf_column(args.in, stream);
  auto dictionary = to_cudf_column(args.dict, stream);

  DeferredBufferAllocator mr;
  auto result = detail::encode(in, dictionary, stream, &mr);
  from_cudf_column(args.out, std::move(result), stream, mr);
}

}  // namespace category
}  // namespace pandas
}  // namespace legate
