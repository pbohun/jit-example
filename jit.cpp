#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/mman.h>

int main(int argc, char *argv[]) {
	/*
		Let's write the machine code for:
		mov eax, 0
		ret
	*/
	unsigned char code[] = {0xb8, 0x00, 0x00, 0x00, 0x00, 0xc3};

	if (argc < 2) {
	fprintf(stderr, "Usage: %s <integer>\n", argv[0]);
	return 1;
	}

	/*
		Let's modify our machine code to move the user's 
		value into eax like so:

		mov eax, <user's value>
		ret
	*/
	int num = atoi(argv[1]);
	memcpy(&code[1], &num, 4);

	/*
		Let's create some readable/writable memory. Instead of malloc
		we use mmap. Windows doesn't have mmap, so this code only works
		on POSIX compatible systems (use VirtualAlloc on Windows).
	*/
	void *mem = mmap(NULL, sizeof(code), PROT_READ | PROT_WRITE,
		   MAP_ANON | MAP_PRIVATE, -1, 0);
	memcpy(mem, code, sizeof(code));

	/*
		Now we use the POSIX mprotect to change our memory protections. 
		Technically you could mmap with PROT_WRITE and PROT_EXEC to begin
		with, but this is not good for security reasons. On Windows 
		you'll want to use VirtualProtect.
	*/
	mprotect(mem, sizeof(code), PROT_READ | PROT_EXEC); 

	/*
		Right now we have a void*. We must reinterpret_cast to a 
		function pointer with the proper signature.
	*/
	int (*func)() = reinterpret_cast<int(*)()>(mem);

	/*
		Let's use our function!
	*/
	printf("Your number was: %d\n", func());

	/*
		When you're done with the memory, you want to unmap it with
		munmap(<ptr_to_buffer>, <page_size>). The memory will automatically
		be unmapped when the program exits. If you want to free the memory
		while running your program, then you must mmap a multiple of your
		page size to begin with (probably 4096 bytes). Since we did not 
		mmap this amout to being with, we will leave the cleanup to be 
		automated by the end of the process.
	*/
	return 0;
}
