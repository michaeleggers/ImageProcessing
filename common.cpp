#include "common.h"


#include <stdint.h>
#include <string> 

#include "stb_image.h"

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

std::string com_GetExePath(void)
{
	char out_buffer[256];
	int  buffer_size = 256;
	DWORD len = GetModuleFileNameA(NULL, out_buffer, buffer_size);
	if (!len) {
		DWORD error = GetLastError();
		char errorMsgBuf[256];
		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			errorMsgBuf, (sizeof(errorMsgBuf) / sizeof(char)), NULL);

		printf("%s\n", errorMsgBuf);
	}

	// strip actual name of the .exe
	char* last = out_buffer + len;
	while (*last != '\\') {
		*last-- = '\0';
	}

	return std::string(out_buffer);
}

#elif __APPLE__

#include <mach-o/dyld.h>

std::string com_GetExePath(void)
{
	char out_buffer[256];
	uint32_t buffer_size = 256;
	
	int error = _NSGetExecutablePath(out_buffer, &buffer_size);
	if (error) {
		// TOOO: handle error
	}
	int len = strlen(out_buffer);
	char* slash = out_buffer + len + 1;
	while (len >= 0 && *slash != '/') { slash--; len--; }
	out_buffer[len + 1] = '\0';

	return std::string(out_buffer);
}

#elif __linux__

#include <unistd.h>
#include <string.h>

std::string com_GetExePath()
{
	char out_buffer[256];
	uint32_t buffer_size = 256;

	readlink("/proc/self/exe", out_buffer, buffer_size);
	int len = strlen(out_buffer);
	char * slash = out_buffer + len + 1;
	while (len >= 0 && *slash != '/') { slash--; len--; }
	out_buffer[len + 1] = '\0';
	//printf("%s\n", out_buffer);

	return std::string(out_buffer);
}

#endif


ATP_Status atp_read_file(char const* filename, ATP_File* out_File) {
	FILE* file = 0;
	file = fopen(filename, "rb");
	if (file == 0) {
		printf("Failed to open file: %s\n", filename);
		return ATP_ERROR_READ_FILE;
	}
	fseek(file, 0L, SEEK_END);
	out_File->size = ftell(file);
	fseek(file, 0L, SEEK_SET);
	out_File->data = (uint8_t*)malloc(out_File->size + 1);
	fread(out_File->data, sizeof(uint8_t), out_File->size, file);
	fclose(file);

	out_File->data[out_File->size] = '\0';

	return ATP_SUCCESS;
}

ATP_Status atp_destroy_file(ATP_File* file) {
	if (file->data != NULL) {
		free(file->data);
		file->size = 0;

		return ATP_SUCCESS;
	}

	return ATP_ERROR_NO_FILE;
}

