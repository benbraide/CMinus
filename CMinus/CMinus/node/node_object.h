#pragma once

#include "../logic/type_object.h"

namespace cminus::node{
	class object{
	public:
		struct index_info{
			std::size_t line;
			std::size_t column;
		};

		explicit object(object *parent);

		virtual ~object();

		virtual object *get_parent() const;

		virtual const index_info &get_index() const = 0;

		virtual std::shared_ptr<object> clone() const = 0;

		virtual void substitute(const std::string &key, std::shared_ptr<object> value);

		virtual void substitute(const object &target, std::shared_ptr<object> value);

		virtual void print(logic::runtime &runtime) const = 0;

		virtual std::shared_ptr<memory::reference> evaluate(logic::runtime &runtime) const = 0;

	protected:
		virtual void set_parent_(std::shared_ptr<object> target);

		object *parent_;
	};

	class object_with_index : public object{
	public:
		object_with_index(object *parent, const index_info &index);

		virtual ~object_with_index();

		virtual const index_info &get_index() const override;

	protected:
		index_info index_;
	};
}
