#pragma once

#include "function_declaration.h"

namespace cminus::declaration::string{
	class default_constructor : public function{
	public:
		default_constructor(logic::runtime &runtime, logic::naming::parent *parent);

		virtual ~default_constructor();

		virtual bool is_defined() const override;

		virtual bool supports_return_statement() const override;

	protected:
		virtual void print_return_(logic::runtime &runtime) const override;

		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};

	class destructor : public function{
	public:
		destructor(logic::runtime &runtime, logic::naming::parent *parent);

		virtual ~destructor();

		virtual bool is_defined() const override;

		virtual bool supports_return_statement() const override;

	protected:
		virtual void print_return_(logic::runtime &runtime) const override;

		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};

	class empty : public function{
	public:
		empty(logic::runtime &runtime, logic::naming::parent *parent);

		virtual ~empty();

		virtual bool is_defined() const override;

	protected:
		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};

	class size : public function{
	public:
		explicit size(logic::runtime &runtime, logic::naming::parent *parent);

		virtual ~size();

		virtual bool is_defined() const override;

	protected:
		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};

	class data : public function{
	public:
		data(logic::runtime &runtime, bool read_only, logic::naming::parent *parent);

		virtual ~data();

		virtual bool is_defined() const override;

	protected:
		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};

	class resize : public function{
	public:
		resize(logic::runtime &runtime, logic::naming::parent *parent);

		virtual ~resize();

		virtual bool is_defined() const override;

	protected:
		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};

	class clear : public function{
	public:
		clear(logic::runtime &runtime, logic::naming::parent *parent);

		virtual ~clear();

		virtual bool is_defined() const override;

	protected:
		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};
}
