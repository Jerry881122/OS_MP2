#include "syscall.h"

int main(void)
{
	char test[] = "abcdefghijklmnopqrstuvwxyz";
	int success = Create("none__none");
	OpenFileId fid;
	int i;
	if (success != 1) MSG("Failed on creating file");
	fid = Open("none__none");
	if (fid == -1) MSG("Failed on opening file");
	for (i = 0; i < 26; ++i) {
		int count = Write(test + i, 1, fid);
		if (count != 1) MSG("Failed on writing file");
	}
	success = Close(fid);
	if (success != 1) MSG("Failed on closing file");
	Halt();
}

