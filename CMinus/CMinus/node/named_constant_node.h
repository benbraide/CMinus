#pragma once

#include "../type/primitive_type.h"

#include "node_object.h"

namespace cminus::node{
	class named_constant : public object_with_index{
	public:
		enum class constant_type : char{
			nil,
			false_,
			true_,
			indeterminate,
			nullptr_,
			nan_,
		};

		named_constant(object *parent, const index_info &index, constant_type type);

		virtual ~named_constant();

		virtual std::shared_ptr<object> clone() const override;

		virtual void print(logic::runtime &runtime) const override;

		virtual std::shared_ptr<memory::reference> evaluate(logic::runtime &runtime) const override;

		virtual constant_type get_type() const;

	protected:
		constant_type type_;
	};
}
