#include <stdio.h> // For printf, fprintf, stderr
#include <string.h> // For memcmp
#include <unistd.h> // For getbufsiz
#include <mach/mach.h> // For everything else

#include "kern_utils.h"

// Returns the offset of the beginning of the pattern, if found.
// Errors result in a return value of 0.
//
// TODO: Consider making this function return kern_return_t and
// instead write the address to a vm_address_t* parameter.
vm_address_t kern_find(vm_map_t kern, const char* pattern, vm_size_t pattern_length) {
	vm_address_t addr, buf, ret = 0;
	// 0xc00 because size must be less than 0x1000.
	vm_size_t read_size, bufsiz = 0xc00;
	
	// Allocate a read buffer in the current task's memory space
	kern_return_t kr = vm_allocate(mach_task_self(), &buf, bufsiz, VM_PROT_READ | VM_PROT_WRITE);
	if(kr != KERN_SUCCESS) {
		fprintf(stderr, "vm_allocate failed: %x\n", kr);
		return 0;
	}
	
	printf("Searching for address to patch...\n");
	
	// Go through the entire kernel memory looking for pattern
	for(addr = 0x80002000; addr < 0x80A00000; addr += bufsiz) {
		// Read bufsiz bytes from the kernel into our buffer
		kr = vm_read_overwrite(kern, addr, bufsiz, buf, &read_size);
		if(kr != KERN_SUCCESS) {
			fprintf(stderr, "vm_read_overwrite failed: %x\n", kr);
			return 0;
		}
		
		int i, j;
		// Loop through the kernel data.
		for(i = 0; i < read_size; i++) {
			// FIXME: This will fail to find data that overlaps
			// page boundaries.
			if(memcmp((void*)(buf + i), pattern, pattern_length) == 0) {
				ret = addr + i;
				break;
			}
		}
		
		// Found match; exit loop
		if(ret) {
			fprintf(stderr, "Found match at %x.\n", ret);
			break;
		}
	}
	
	// Done searching; try to free up resources like a good little memory manager.
	kr = vm_deallocate(mach_task_self(), buf, bufsiz);
	if(kr != KERN_SUCCESS) {
		fprintf(stderr, "vm_deallocate failed: %x\nAttempting to continue...\n", kr);
	}
	
	if(ret == 0) {
		fprintf(stderr, "kern_find was unable to find a match\n");
	}
	
	return ret;
}


// Patches the running kernel after boot
kern_return_t kern_patch(vm_map_t kern, struct patch data) {
	// Try to find the address of the pattern
	vm_address_t addr = kern_find(kern, data.pattern, data.pattern_length);
	if(addr == 0) {
		fprintf(stderr, "kern_find failed\n");
		return KERN_FAILURE;
	}
	return vm_write(kern, addr, (vm_offset_t)data.replace, data.replace_length);
}
