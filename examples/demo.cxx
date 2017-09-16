#include <iostream>
#include <memory>
#include <dlibxx.hxx>
#include <assert.h>

#include "plugin.hxx"

int main() {
	using namespace dlibxx;
	std::string libname;
	
	while (true) {
		std::cout << "Enter the name of the library you wish to load: ";
		std::cin >> libname;

		/* Load the library specified. */
		handle lib{ resolve_policy::lazy, libname.c_str() };

		/* It's possible to test whether the library is currently loaded */
		assert(bool(lib.error()) != handle::has_library_loaded(libname.c_str()));
		
		/* Check for loading errors. */
		if (lib.error()) {
			std::cerr << lib.error().value() << std::endl;
		}
		{
			/* Attempt to resolve the "bin_op" symbol. */
			auto op = lib.lookup<decltype(::bin_op)>("bin_op");
			
			/* Only call the function if symbol lookup was successful. */
			if (op) std::cout << op.value()(1, 2) << std::endl;
			else    std::cerr << lib.error().value() << '\n';
		}
		{
			/* Attempt to create a plugin instance using the "create_plugin"
			factory method from the dynamic library. */
			std::shared_ptr<plugin> p = lib.create<plugin>("create_plugin");
			
			/* Only call render if the object could successfully be created. */
			if (p) p->render();
			else   std::cerr << lib.error().value() << '\n';
		}
	}
}
