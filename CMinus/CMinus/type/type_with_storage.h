#pragma once

#include "../logic/storage_object.h"

#include "type_object.h"

namespace cminus::type{
	class with_storage : public type::object, public logic::storage::object{
	public:
		using type_base_type = type::object;
		using storage_base_type = logic::storage::object;

		explicit with_storage(const std::string &name, logic::storage::object *parent = nullptr);

		virtual ~with_storage();

		virtual void print(logic::runtime &runtime, bool is_qualified) const override;
	};
}
