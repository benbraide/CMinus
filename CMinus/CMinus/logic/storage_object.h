#pragma once

#include "../node/node_object.h"
#include "../memory/memory_reference.h"
#include "../evaluator/operator_id.h"

namespace cminus::declaration{
	class function_base;
}

namespace cminus::logic::storage{
	enum class error_code{
		nil,
		duplicate_entry,
		duplicate_function,
	};

	class exception : public std::exception{
	public:
		using base_type = std::exception;

		exception(error_code code);

		error_code get_code() const;

	private:
		error_code code_;
	};

	class object : public naming::parent{
	public:
		using parent_base_type = naming::parent;

		enum class interrupt_type{
			nil,
			return_,
			break_,
			continue_,
		};

		struct function_group_info{
			std::size_t address;
			std::shared_ptr<declaration::function_group_base> value;
		};

		struct search_options{
			const object *scope;
			std::shared_ptr<memory::reference> context;
			std::string name;
			bool search_tree;
			const object **branch;
		};

		struct namesless_search_options{
			const object *scope;
			std::shared_ptr<memory::reference> context;
			bool search_tree;
			const object **branch;
		};

		explicit object(const std::string &value, object *parent = nullptr);

		virtual ~object();

		virtual void raise_interrupt(interrupt_type type, std::shared_ptr<memory::reference> value);

		virtual std::shared_ptr<memory::reference> get_raised_interrupt_value() const;

		virtual std::shared_ptr<memory::reference> get_context() const;

		virtual void add(logic::runtime &runtime, const std::string &name, std::shared_ptr<memory::reference> entry);

		virtual void add_function(logic::runtime &runtime, std::shared_ptr<declaration::function_base> entry, std::size_t group_address = 0u);

		virtual void remove(const std::string &name);

		virtual bool exists(const std::string &name) const;

		virtual std::shared_ptr<memory::reference> find(logic::runtime &runtime, const search_options &options) const;

		virtual std::shared_ptr<memory::reference> find(logic::runtime &runtime, const std::string &name, bool search_tree) const;

		virtual std::shared_ptr<memory::reference> find_operator(logic::runtime &runtime, evaluator::operator_id id, const namesless_search_options &options) const;

		virtual std::shared_ptr<attributes::object> find_attribute(const std::string &name, bool search_tree, const object **branch = nullptr) const;

		template <typename target_type>
		target_type *get_first_of_type() const{
			if (auto this_target = dynamic_cast<target_type *>(const_cast<object *>(this)); this_target != nullptr)
				return this_target;

			if (auto object_parent = dynamic_cast<object *>(parent_); object_parent != nullptr)
				return object_parent->get_first_of_type<target_type>();

			return nullptr;
		}

	protected:
		virtual void destroy_entries_();

		virtual void destroy_entry_(memory::reference *entry);

		virtual void invalid_interrupt_(interrupt_type type, std::shared_ptr<memory::reference> value);

		virtual bool validate_(const declaration::function_base &target) const;

		virtual void extend_function_group_(logic::runtime &runtime, declaration::function_group_base &group, std::shared_ptr<declaration::function_base> entry);

		std::list<memory::reference *> entries_order_;
		std::unordered_map<std::string, std::shared_ptr<memory::reference>> entries_;

		std::unordered_map<std::string, function_group_info> function_groups_;
		std::unordered_map<std::string, std::shared_ptr<attributes::object>> attributes_;
	};

	class proxy : public object{
	public:
		explicit proxy(object &target);

		virtual ~proxy();

		virtual void print(logic::runtime &runtime, bool is_qualified) const override;

		virtual bool is_same(const naming::object &target) const override;

		virtual void add(logic::runtime &runtime, const std::string &name, std::shared_ptr<memory::reference> entry) override;

		virtual void add_function(logic::runtime &runtime, std::shared_ptr<declaration::function_base> entry, std::size_t group_address = 0u) override;

		virtual void remove(const std::string &name) override;

		virtual bool exists(const std::string &name) const override;

		virtual std::shared_ptr<memory::reference> find(logic::runtime &runtime, const search_options &options) const override;

		virtual std::shared_ptr<memory::reference> find(logic::runtime &runtime, const std::string &name, bool search_tree) const override;

		virtual std::shared_ptr<attributes::object> find_attribute(const std::string &name, bool search_tree, const object **branch = nullptr) const override;

	protected:
		object &target_;
	};

	class runtime_storage_guard{
	public:
		runtime_storage_guard(logic::runtime &runtime, std::shared_ptr<object> current);

		~runtime_storage_guard();

	private:
		logic::runtime &runtime_;
		object *old_;
		std::shared_ptr<object> new_;
	};
}

