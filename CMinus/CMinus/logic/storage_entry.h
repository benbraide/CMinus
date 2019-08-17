#pragma once

#include <variant>

#include "../memory/memory_reference.h"

namespace cminus::logic::storage{
	class object;

	class entry{
	public:
		using value_type = std::variant<
			std::shared_ptr<memory::reference>,
			std::shared_ptr<type::object>,
			std::shared_ptr<object>
		>;

		explicit entry(std::shared_ptr<memory::reference> value);

		explicit entry(std::shared_ptr<type::object> value);

		explicit entry(std::shared_ptr<object> value);

		memory::reference *get_reference_value() const;

		type::object *get_type_value() const;

		object *get_storage_value(bool no_type) const;

	private:
		value_type value_;
	};
}
