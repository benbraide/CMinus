#pragma once

#include "runtime.h"

namespace cminus::declaration{
	class function_base;
}

namespace cminus::logic::storage{
	class specialized : public object{
	public:
		explicit specialized(const std::string &name, object *parent = nullptr);

		virtual ~specialized();

		virtual void raise_interrupt(interrupt_type type, std::shared_ptr<memory::reference> value) override;

		virtual std::shared_ptr<memory::reference> get_raised_interrupt_value() const override;

	protected:
		virtual bool interrupt_is_valid_(interrupt_type value) const = 0;

		std::shared_ptr<memory::reference> interrupt_value_;
	};

	class double_layer : public specialized{
	public:
		explicit double_layer(const std::string &name, object *parent = nullptr);

		virtual ~double_layer();

		virtual void add(logic::runtime &runtime, const std::string &name, std::shared_ptr<memory::reference> entry) override;

		virtual void add_function(logic::runtime &runtime, std::shared_ptr<declaration::function_base> entry) override;

		virtual void remove(const std::string &name) override;

		virtual bool exists(const std::string &name) const override;

		using specialized::find;

		virtual std::shared_ptr<memory::reference> find(logic::runtime &runtime, const search_options &options) const override;

		virtual void refresh();

	protected:
		virtual void invalid_interrupt_(interrupt_type type, std::shared_ptr<memory::reference> value) override;

		std::shared_ptr<object> inner_layer_;
	};

	class function : public specialized{
	public:
		function(const declaration::function_base &owner, std::shared_ptr<memory::reference> context, object *parent = nullptr);

		virtual ~function();

		using specialized::find;

		virtual std::shared_ptr<memory::reference> find(logic::runtime &runtime, const search_options &options) const override;

		virtual std::shared_ptr<memory::reference> get_context() const;

		virtual void add_unnamed(std::shared_ptr<memory::reference> entry);

		virtual const declaration::function_base &get_owner() const;

	protected:
		virtual void destroy_entry_(memory::reference *entry) override;

		virtual bool interrupt_is_valid_(interrupt_type value) const override;

		const declaration::function_base &owner_;
		std::shared_ptr<memory::reference> context_;
		std::unordered_map<memory::reference *, std::shared_ptr<memory::reference>> unnamed_entries_;
	};

	class loop : public double_layer{
	public:
		explicit loop(object *parent = nullptr);

		virtual ~loop();

	protected:
		virtual bool interrupt_is_valid_(interrupt_type value) const override;
	};

	class switch_ : public double_layer{
	public:
		explicit switch_(object *parent = nullptr);

		virtual ~switch_();

	protected:
		virtual bool interrupt_is_valid_(interrupt_type value) const override;
	};

	class selection : public double_layer{
	public:
		explicit selection(object *parent = nullptr);

		virtual ~selection();

	protected:
		virtual bool interrupt_is_valid_(interrupt_type value) const override;
	};
}
