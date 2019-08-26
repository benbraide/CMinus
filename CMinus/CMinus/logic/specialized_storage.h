#pragma once

#include "runtime.h"

namespace cminus::logic::storage{
	class specialized : public object{
	public:
		enum class interrupt_type{
			nil,
			return_,
			break_,
			continue_,
		};

		explicit specialized(const std::string &name, object *parent = nullptr);

		virtual ~specialized();

		virtual void raise_interrupt(interrupt_type type, std::shared_ptr<memory::reference> value);

		virtual std::shared_ptr<memory::reference> get_raised_interrupt_value() const;

	protected:
		virtual bool interrupt_is_valid_(interrupt_type value) const = 0;

		virtual void invalid_interrupt_(interrupt_type type, std::shared_ptr<memory::reference> value);

		std::shared_ptr<memory::reference> interrupt_value_;
	};

	class double_layer : public specialized{
	public:
		explicit double_layer(const std::string &name, object *parent = nullptr);

		virtual ~double_layer();

		virtual void add(const std::string &name, std::shared_ptr<memory::reference> entry) override;

		virtual void remove(const std::string &name) override;

		virtual std::shared_ptr<memory::reference> find(const std::string &name, bool search_tree, const object **branch = nullptr) const override;

		virtual void refresh();

	protected:
		virtual void invalid_interrupt_(interrupt_type type, std::shared_ptr<memory::reference> value) override;

		std::shared_ptr<object> inner_layer_;
	};

	class function : public specialized{
	public:
		explicit function(std::shared_ptr<memory::reference> context, object *parent = nullptr);

		virtual ~function();

		virtual std::shared_ptr<memory::reference> find(const std::string &name, bool search_tree, const object **branch = nullptr) const override;

		virtual std::shared_ptr<memory::reference> get_context() const;

		virtual void add_unnamed(std::shared_ptr<memory::reference> entry);

	protected:
		virtual bool interrupt_is_valid_(interrupt_type value) const override;

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
