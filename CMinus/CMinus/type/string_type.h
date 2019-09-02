#pragma once

#include "../declaration/string_function_declarations.h"

#include "class_type.h"

namespace cminus::type{
	class string : public class_{
	public:
		explicit string(logic::runtime &runtime, logic::storage::object *parent = nullptr);

		virtual ~string();
	};
}
