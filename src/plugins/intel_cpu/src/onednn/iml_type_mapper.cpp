// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "iml_type_mapper.h"

namespace ov {
namespace intel_cpu {

impl_desc_type parse_impl_name(std::string impl_desc_name) {
    impl_desc_type res = impl_desc_type::unknown;

#define REPLACE_WORD(_wrd, _sub) auto pos = impl_desc_name.find(#_wrd); \
    if (pos != std::string::npos) impl_desc_name.replace(pos, std::string(#_wrd).length(), #_sub);

    REPLACE_WORD(simple, ref);
#undef REPLACE_WORD

#define SEARCH_WORD(_wrd) if (impl_desc_name.find(#_wrd) != std::string::npos) \
    res = static_cast<impl_desc_type>(res | impl_desc_type::_wrd);
#define SEARCH_WORD_2(_wrd, _key) if (impl_desc_name.find(#_wrd) != std::string::npos) \
    res = static_cast<impl_desc_type>(res | impl_desc_type::_key);

    SEARCH_WORD(ref);
    SEARCH_WORD(jit);
    SEARCH_WORD(brgconv);
    SEARCH_WORD(brgemm);
    if ((res & impl_desc_type::brgemm) != impl_desc_type::brgemm)
        SEARCH_WORD(gemm);
    SEARCH_WORD(blas);
    SEARCH_WORD(sse42);
    SEARCH_WORD_2(sse41, sse42);
    SEARCH_WORD(avx2);
    SEARCH_WORD(amx);
    SEARCH_WORD(avx512);
    SEARCH_WORD(any);
    SEARCH_WORD(_1x1);
    SEARCH_WORD(_dw);
    SEARCH_WORD(reorder);
    SEARCH_WORD(sparse);
    if ((res & impl_desc_type::avx2) != impl_desc_type::avx2 &&
        (res & impl_desc_type::avx512) != impl_desc_type::avx512)
        SEARCH_WORD(avx);
    if ((res & impl_desc_type::sse42) != impl_desc_type::sse42 &&
        (res & impl_desc_type::avx) != impl_desc_type::avx &&
        (res & impl_desc_type::avx2) != impl_desc_type::avx2 &&
        (res & impl_desc_type::avx512) != impl_desc_type::avx512)
        SEARCH_WORD(uni);

    SEARCH_WORD_2(nchw, ref);
    SEARCH_WORD_2(ncdhw, ref);
    SEARCH_WORD_2(wino, winograd);

#undef SEARCH_WORD_2
#undef SEARCH_WORD

    return res;
}

const char* impl_type_to_string(impl_desc_type type) {
#define CASE(_type) do {                    \
    if (type == _type) return #_type;       \
} while (0)
    CASE(unknown);
    CASE(undef);
    CASE(ref_any);
    CASE(reorder);
    CASE(gemm_any);
    CASE(gemm_blas);
    CASE(gemm_avx512);
    CASE(gemm_avx2);
    CASE(gemm_avx);
    CASE(gemm_sse42);
    CASE(jit_gemm);
    CASE(jit_avx512_winograd);
    CASE(jit_avx512);
    CASE(jit_avx2);
    CASE(jit_avx);
    CASE(jit_sse42);
    CASE(jit_uni);
    CASE(jit_avx512_1x1);
    CASE(jit_avx2_1x1);
    CASE(jit_avx_1x1);
    CASE(jit_sse42_1x1);
    CASE(jit_uni_1x1);
    CASE(jit_avx512_dw);
    CASE(jit_avx2_dw);
    CASE(jit_avx_dw);
    CASE(jit_sse42_dw);
    CASE(jit_uni_dw);
    CASE(jit_avx512_amx);
    CASE(jit_avx512_amx_1x1);
    CASE(jit_avx512_amx_dw);
    CASE(brgconv_avx512);
    CASE(brgconv_avx2);
    CASE(brgconv_avx);
    CASE(brgconv_sse42);
    CASE(brgconv_uni);
    CASE(brgconv_avx512_amx);
    CASE(brgconv_avx512_1x1);
    CASE(brgconv_avx2_1x1);
    CASE(brgconv_avx_1x1);
    CASE(brgconv_sse42_1x1);
    CASE(brgconv_uni_1x1);
    CASE(brgconv_avx512_amx_1x1);
    CASE(brgemm_avx512);
    CASE(brgemm_avx2);
    CASE(brgemm_avx);
    CASE(brgemm_sse42);
    CASE(brgemm_uni);
    CASE(brgemm_avx512_amx);
    CASE(brgemm_sparse_avx512_amx);

#undef CASE
    return "unknown";
}

}   // namespace intel_cpu
}   // namespace ov
