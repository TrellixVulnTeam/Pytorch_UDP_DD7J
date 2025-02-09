/*******************************************************************************
* Copyright 2020-2022 Intel Corporation
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
*******************************************************************************/

#ifndef BACKEND_DNNL_KERNELS_BINARY_HPP
#define BACKEND_DNNL_KERNELS_BINARY_HPP

#include <algorithm>
#include <assert.h>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <unordered_set>

#include "interface/backend.hpp"

#include "backend/dnnl/common.hpp"
#include "backend/dnnl/constant_cache.hpp"
#include "backend/dnnl/dnnl_backend.hpp"
#include "backend/dnnl/dnnl_partition_impl.hpp"
#include "backend/dnnl/passes/compile_ops.hpp"
#include "backend/dnnl/passes/constant_propagation.hpp"
#include "backend/dnnl/passes/infer_type.hpp"
#include "backend/dnnl/passes/insert_ops.hpp"
#include "backend/dnnl/passes/layout_propagation.hpp"
#include "backend/dnnl/passes/lower_down.hpp"
#include "backend/dnnl/passes/memory_planning.hpp"
#include "backend/dnnl/passes/op_executable.hpp"
#include "backend/dnnl/scratchpad.hpp"
#include "backend/dnnl/thread_local_cache.hpp"

namespace dnnl {
namespace graph {
namespace impl {
namespace dnnl_impl {

namespace bin {
enum binary_inputs { kSrc0, kSrc1 };
enum binary_outputs { kDst };
enum mem_keys {
    kOpt_src0,
    kOpt_src1,
    kOpt_dst,
};
} // namespace bin

// We support both multidirectional and unidirectional broadcast. And the
// broadcast semantics is consistent with PyTorch broadcast:
// Two tensors are “broadcastable” if the following rules hold:
// - Each tensor has at least one dimension.
// - When iterating over the dimension sizes, starting at the trailing
//   dimension, the dimension sizes must either be equal, one of them is 1, or
//   one of them does not exist.
struct binary_t : public kernel_base_t {
    using super = dnnl::binary;

private:
    dnnl::engine p_engine_;
    impl::allocator_t *g_alloc_;

    std::shared_ptr<subgraph_t> subgraph_;
    memory_planner_t memory_planner_;

    std::function<std::shared_ptr<execution_args_set_t>()> resource_ctor_;

    impl::status_t prepare_inplace_pairs_impl() override {
        inplace_pairs_ = memory_planner_.get_subgraph_inplace_pairs();
        return impl::status::success;
    }

public:
    ~binary_t() override {
        thread_local_cache_t<execution_args_set_t> res_cache;
        res_cache.remove_if_exist(reinterpret_cast<size_t>(this));
    }

    impl::status_t compile_impl(const dnnl_partition_impl_t *part,
            const impl::engine_t *g_engine,
            const std::vector<impl::logical_tensor_t> &inputs,
            const std::vector<impl::logical_tensor_t> &outputs) override {
        p_engine_ = make_dnnl_engine(*g_engine);
        g_alloc_ = g_engine->get_allocator();

        subgraph_ = std::make_shared<subgraph_t>(part->get_ops(), p_engine_);
        BACKEND_DNNL_CHECK(
                set_given_inputs_outputs(subgraph_, inputs, outputs));

        subgraph_visualizer_t vis(part->id(), [this](const value_t *val) {
            return this->memory_planner_.get_memory_info(val);
        });
        pass_pipeline_t pipeline(vis);

        BACKEND_DNNL_ADD_PASS(pipeline, lower_down);
        BACKEND_DNNL_ADD_PASS(pipeline, fuse_reciprocal_mul_to_div);
        BACKEND_DNNL_ADD_PASS(pipeline, fuse_mul_sigmoid_to_swish);

        // Because we use binary post-ops for broadcast add and sum post-ops for
        // non-broadcast add. So we have to know concret shape before fuse
        // post-ops
        BACKEND_DNNL_ADD_PASS(pipeline, infer_shape);
        BACKEND_DNNL_ADD_PASS(pipeline, split_squared_difference);
        BACKEND_DNNL_ADD_PASS(pipeline, binary_canonicalization);
        BACKEND_DNNL_ADD_PASS(pipeline, infer_shape);
        BACKEND_DNNL_ADD_PASS(pipeline, binary_broadcast_swap);

        // fuse binary post-ops need shape and type info
        BACKEND_DNNL_ADD_PASS(pipeline, infer_shape);
        BACKEND_DNNL_ADD_PASS(pipeline, infer_type);
        BACKEND_DNNL_ADD_PASS(pipeline, fuse_post_ops);

        BACKEND_DNNL_ADD_PASS(pipeline, infer_shape);

        pipeline.reset_visualize_arg(true, false);
        BACKEND_DNNL_ADD_PASS(pipeline, infer_type);
        BACKEND_DNNL_ADD_PASS(pipeline, layout_propagation);

        auto memory_plan = [&](std::shared_ptr<subgraph_t> &sg) {
            return memory_planner_.run(sg);
        };
        pipeline.reset_visualize_arg(true, true);
        BACKEND_DNNL_ADD_PASS(pipeline, memory_plan);
        BACKEND_DNNL_ADD_PASS(pipeline, compile_ops);

        // Run the added passes
        BACKEND_DNNL_CHECK(pipeline.run(subgraph_));

        // fill information for outputs logical tensors
        for (size_t i = 0; i < outputs.size(); i++) {
            auto &out = const_cast<impl::logical_tensor_t &>(outputs[i]);
            out = subgraph_->outs_[i];
        }

        resource_ctor_ = [this]() {
            return this->memory_planner_.get_exec_args_set().clone();
        };

        return impl::status::success;
    }

    impl::status_t execute_impl(const dnnl_partition_impl_t *part,
            const impl::stream_t *g_stream,
            const std::vector<impl::tensor_t> &inputs,
            const std::vector<impl::tensor_t> &outputs) override {
        UNUSED(part);
        dnnl::stream p_stream = make_dnnl_stream(p_engine_, *g_stream);

        // each thread's own local resource
        thread_local_cache_t<execution_args_set_t> res_cache;
        execution_args_set_t *res = res_cache.get_or_add(
                reinterpret_cast<size_t>(this), resource_ctor_);

        // update the data of partition in/outputs args
        for (const auto &mem_idx : res->get_mems_use_external_inputs()) {
            mem_idx.first.set_data_handle(
                    inputs[mem_idx.second].get_data_handle());
        }
        for (const auto &mem_idx : res->get_mems_use_external_outputs()) {
            mem_idx.first.set_data_handle(
                    outputs[mem_idx.second].get_data_handle());
        }

        temporary_scratchpad_t scratchpad(
                memory_planner_.total_internal_temporary_size(), p_engine_,
                *g_alloc_);
        assertm(scratchpad.size()
                        >= memory_planner_.total_internal_temporary_size(),
                "no enough scratchpad memory");
        grantor_t var_grantor = memory_planner_.internal_temporary_grantor(
                scratchpad.get_buffer());

        for (auto &mem_offkey : res->get_mems_use_internal_temporary()) {
            mem_offkey.first.set_data_handle(
                    var_grantor.get(mem_offkey.second));
        }

        for (size_t i = 0; i < subgraph_->execs_.size(); i++) {
            subgraph_->execs_[i]->execute(p_stream, res->get_exec_args()[i]);
        }

        return impl::status::success;
    }
};

} // namespace dnnl_impl
} // namespace impl
} // namespace graph
} // namespace dnnl

#endif
