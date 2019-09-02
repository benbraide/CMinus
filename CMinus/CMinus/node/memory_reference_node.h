#pragma once

#include "node_object.h"

namespace cminus::node{
	class memory_reference : public object_with_index{
	public:
		memory_reference(object *parent, const index_info &index, std::shared_ptr<cminus::memory::reference> value);

		memory_reference(object *parent, std::shared_ptr<cminus::memory::reference> value);

		virtual ~memory_reference();

		virtual std::shared_ptr<object> clone() const override;

		virtual void print(logic::runtime &runtime) const override;

		virtual std::shared_ptr<memory::reference> evaluate(logic::runtime &runtime) const override;

		virtual std::shared_ptr<memory::reference> get_value() const;

	protected:
		std::shared_ptr<memory::reference> value_;
	};
}
