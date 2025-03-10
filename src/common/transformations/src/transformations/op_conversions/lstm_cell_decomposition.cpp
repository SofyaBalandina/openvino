// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "transformations/op_conversions/lstm_cell_decomposition.hpp"

#include <memory>
#include <ngraph/pattern/op/wrap_type.hpp>
#include <ngraph/rt_info.hpp>
#include <openvino/opsets/opset1.hpp>
#include <openvino/opsets/opset4.hpp>
#include <transformations/utils/utils.hpp>
#include <vector>

#include "itt.hpp"

ov::pass::LSTMCellDecomposition::LSTMCellDecomposition() {
    MATCHER_SCOPE(LSTMCellDecomposition);
    auto any_lstm = pattern::wrap_type<opset1::LSTMCell, opset4::LSTMCell>();

    matcher_pass_callback callback = [this](ngraph::pattern::Matcher& m) {
        auto lstm_cell = std::dynamic_pointer_cast<op::util::RNNCellBase>(m.get_match_root());
        if (!lstm_cell || transformation_callback(lstm_cell)) {
            return false;
        }
        const Output<Node>& X = lstm_cell->input_value(0);
        const Output<Node>& H_t = lstm_cell->input_value(1);
        const Output<Node>& C_t = lstm_cell->input_value(2);
        const Output<Node>& W = lstm_cell->input_value(3);
        const Output<Node>& R = lstm_cell->input_value(4);
        const Output<Node>& bias = lstm_cell->input_value(5);

        // Xt*(W^T)
        auto Xt_W = std::make_shared<opset4::MatMul>(X, W, false, true);
        // Ht-1*(R^T)
        auto Ht_R = std::make_shared<opset4::MatMul>(H_t, R, false, true);
        // Xt*(W^T) + Ht-1*(R^T) + Wb + Rb
        auto add = std::make_shared<opset4::Add>(Ht_R, bias);
        auto XHB = std::make_shared<opset4::Add>(Xt_W, add);

        auto axis_node = ov::opset4::Constant::create(element::u64, Shape{}, {1});
        auto split = std::make_shared<opset4::Split>(XHB, axis_node, 4);
        Output<Node> f = split->output(0);
        Output<Node> i = split->output(1);
        Output<Node> c = split->output(2);
        Output<Node> o = split->output(3);

        auto clip = lstm_cell->get_clip();
        if (clip > 0.f) {
            auto clamp_f = std::make_shared<opset4::Clamp>(f, -clip, clip);
            auto clamp_i = std::make_shared<opset4::Clamp>(i, -clip, clip);
            auto clamp_c = std::make_shared<opset4::Clamp>(c, -clip, clip);
            auto clamp_o = std::make_shared<opset4::Clamp>(o, -clip, clip);
            f = clamp_f;
            i = clamp_i;
            c = clamp_c;
            o = clamp_o;
            ngraph::copy_runtime_info(lstm_cell, {clamp_f, clamp_i, clamp_c, clamp_o});
        }

        // ft = f(Xt*(Wf^T) + Ht-1*(Rf^T) + Wbf + Rbf)
        // it = f(Xt*(Wi^T) + Ht-1*(Ri^T) + Wbi + Rbi)
        // ct = g(Xt*(Wc^T) + Ht-1*(Rc^T) + Wbc + Rbc)
        // ot = f(Xt*(Wo^T) + Ht-1*(Ro^T) + Wbo + Rbo)
        auto f_t = ngraph::op::util::activation(lstm_cell->get_activations()[0], f);
        auto i_t = ngraph::op::util::activation(lstm_cell->get_activations()[0], i);
        auto c_t = ngraph::op::util::activation(lstm_cell->get_activations()[1], c);
        auto o_t = ngraph::op::util::activation(lstm_cell->get_activations()[0], o);

        // Ct = ft (.) Ct-1 + it (.) ct
        auto mul1 = std::make_shared<opset4::Multiply>(f_t, C_t);
        auto mul2 = std::make_shared<opset4::Multiply>(i_t, c_t);
        auto out_C = std::make_shared<opset4::Add>(mul1, mul2);

        // H = ot (.) h(Ct)
        auto hC = ngraph::op::util::activation(lstm_cell->get_activations()[2], out_C);
        auto out_H = std::make_shared<opset4::Multiply>(o_t, hC);

        out_H->set_friendly_name(lstm_cell->get_friendly_name() + ".0");
        out_C->set_friendly_name(lstm_cell->get_friendly_name() + ".1");
        ngraph::copy_runtime_info(
            lstm_cell,
            {Xt_W, Ht_R, add, split, mul1, mul2, out_H, hC, out_C, axis_node, XHB, f_t, i_t, c_t, o_t});
        ngraph::replace_node(lstm_cell, {out_H->output(0), out_C->output(0)});
        return true;
    };

    auto m = std::make_shared<ngraph::pattern::Matcher>(any_lstm, matcher_name);
    register_matcher(m, callback);
}
