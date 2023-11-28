#ifndef _COMMON_H_
#define _COMMON_H_

#include <string>

enum ATP_Status
{
	ATP_SUCCESS,
	ATP_ERROR_READ_FILE,
	ATP_ERROR_NO_FILE
};

struct ATP_File
{
	uint8_t* data;
	uint32_t   size;
};

std::string			com_GetExePath(void);
ATP_Status			atp_read_file(char const* filename, ATP_File* out_File);
ATP_Status			atp_destroy_file(ATP_File* file);

#endif