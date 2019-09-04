#pragma once

#include "node_object.h"

namespace cminus::node{
	class list : public object{
	public:
		list(object *parent, const std::vector<std::shared_ptr<object>> &value);

		list(object *parent, std::vector<std::shared_ptr<object>> &&value);

		virtual ~list();

		virtual const index_info &get_index() const override;

		virtual std::shared_ptr<object> clone() const override;

		virtual void print(logic::runtime &runtime) const override;

		virtual std::shared_ptr<memory::reference> evaluate(logic::runtime &runtime) const override;

		virtual const std::vector<std::shared_ptr<object>> &get_value() const;

	protected:
		std::vector<std::shared_ptr<object>> value_;
	};
}
