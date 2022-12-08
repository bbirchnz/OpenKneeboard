// Copyright ⓒ 2018-2022 ThePhD.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//  See https://github.com/ThePhD/out_ptr/blob/master/docs/out_ptr.adoc for documentation.

#pragma once

#ifndef ZTD_OUT_PTR_CLEVER_INOUT_PTR_HPP
#define ZTD_OUT_PTR_CLEVER_INOUT_PTR_HPP

#include <ztd/out_ptr/detail/clever_inout_ptr_impl.hpp>
#include <ztd/out_ptr/detail/marker.hpp>

#include <cstdlib>
#include <cstddef>
#include <type_traits>
#include <memory>
#include <exception>
#include <utility>
#include <tuple>

namespace ztd {
namespace out_ptr {
namespace op_detail {

	template <typename Smart, typename Pointer, typename... Args>
	class ZTD_OUT_PTR_TRIVIAL_ABI_I_ clever_inout_ptr_t : public clever_inout_ptr_impl<Smart, Pointer, std::tuple<Args...>, ztd::out_ptr::op_detail::make_index_sequence<std::tuple_size<std::tuple<Args...>>::value>> {
	private:
		using list_t = ztd::out_ptr::op_detail::make_index_sequence<std::tuple_size<std::tuple<Args...>>::value>;
		using core_t = clever_inout_ptr_impl<Smart, Pointer, std::tuple<Args...>, list_t>;

	public:
		clever_inout_ptr_t(Smart& s, Args... args) noexcept
		: core_t(s, std::forward_as_tuple(std::forward<Args>(args)...)) {
		}
	};

	template <typename Pointer, typename Smart, typename... Args>
	clever_inout_ptr_t<Smart, Pointer, Args...> clever_inout_ptr_tagged(std::false_type, Smart& s, Args&&... args) noexcept(::std::is_nothrow_constructible<clever_inout_ptr_t<Smart, Pointer, Args...>, Smart&, Args...>::value) {
		using P = clever_inout_ptr_t<Smart, Pointer, Args...>;
		return P(s, std::forward<Args>(args)...);
	}

	template <typename, typename Smart, typename... Args>
	clever_inout_ptr_t<Smart, pointer_of_t<Smart>, Args...> clever_inout_ptr_tagged(std::true_type, Smart& s, Args&&... args) noexcept(::std::is_nothrow_constructible<clever_inout_ptr_t<Smart, pointer_of_t<Smart>, Args...>, Smart&, Args...>::value) {
		using Pointer = pointer_of_t<Smart>;
		using P	    = clever_inout_ptr_t<Smart, Pointer, Args...>;
		return P(s, std::forward<Args>(args)...);
	}

	template <typename Pointer = marker, typename Smart, typename... Args>
	auto clever_inout_ptr(Smart& p, Args&&... args) noexcept(noexcept(clever_inout_ptr_tagged<Pointer>(::std::is_same<Pointer, marker>(), p, std::forward<Args>(args)...)))
		-> decltype(clever_inout_ptr_tagged<Pointer>(::std::is_same<Pointer, marker>(), p, std::forward<Args>(args)...)) {
		return clever_inout_ptr_tagged<Pointer>(::std::is_same<Pointer, marker>(), p, std::forward<Args>(args)...);
	}

}}} // namespace ztd::out_ptr::op_detail

#endif // ZTD_OUT_PTR_CLEVER_INOUT_PTR_HPP
