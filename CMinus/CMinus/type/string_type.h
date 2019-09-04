#pragma once

#include "../declaration/string_function_declarations.h"

#include "class_type.h"

namespace cminus::type{
	class string : public class_{
	public:
		explicit string(logic::runtime &runtime);

		virtual ~string();

		virtual void print(logic::runtime &runtime, bool is_qualified) const override;

		virtual void print_value(logic::runtime &runtime, std::shared_ptr<memory::reference> data) const override;

		virtual std::shared_ptr<evaluator::object> get_evaluator(logic::runtime &runtime) const override;

		virtual void init(logic::runtime &runtime);
	};
}
