#include <stdio.h>
#include <mach/mach.h>

#include "kern_utils.h"


// Credits go to br0x and comex
static struct patch cck_ios5 = {
	6, "\x32\x20\x00\x21\x20\x22",
	1, "\xfa"
};


int main(int argc, char* argv[]) {
	vm_map_t kernel_task = 0;
	kern_return_t kr = task_for_pid(mach_task_self(), 0, &kernel_task);
	if(kr != 0 || kernel_task == 0) {
		fprintf(stderr, "task_for_pid failed: %x\n\tEither you are missing a kernel patch or you don't have the proper entitlements set.\n", kr);
		return 1;
	}
	
	printf("Successfully got kernel_task.\nPatching...\n");
	
	kr = kern_patch(kernel_task, cck_ios5);
	if(kr != KERN_SUCCESS) {
		fprintf(stderr, "kern_patch failed: %x\n", kr);
		return 1;
	}
	
	printf("Kernel successfully patched! All done!\n");
	return 0;
}
