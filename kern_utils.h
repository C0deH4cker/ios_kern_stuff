#include <mach/mach.h>

struct patch {
	vm_size_t pattern_length;
	const char* pattern;
	vm_size_t replace_length;
	const char* replace;
};

vm_address_t kern_find(vm_map_t kern, const char* pattern, vm_size_t pattern_length);
kern_return_t kern_patch(vm_map_t kern, struct patch data);
