#pragma once

#include "../logic/runtime.h"

namespace cminus::declaration{
	class object{
	public:
		virtual ~object() = default;

		virtual const std::string &get_name() const = 0;

		virtual std::size_t get_static_size() const = 0;
	};
}
