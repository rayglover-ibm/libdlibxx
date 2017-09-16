#include <string>

#include "plugin.hxx"

struct plugin_a final : plugin {
	virtual std::string name() override {
		return "plugin_a";
	}

	virtual void render() override {
		std::cout << "plugin_a::render()\n";
	}
};

extern "C" plugin* create_plugin() {
	return new plugin_a;
}

int bin_op(int a, int b) {
	return a + b;
}
