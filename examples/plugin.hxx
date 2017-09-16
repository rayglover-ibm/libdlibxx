#pragma once
#include <string>
#include <iostream>

extern "C" int bin_op(int a, int b);

/* A rendering plugin interface. */
struct plugin
{
	virtual ~plugin() {}

	virtual std::string name () = 0;
	virtual void render() {
		std::cout << "Default rendering\n";
	}
};
