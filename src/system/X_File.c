// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#include <stdio.h>

#include "X_File.h"
#include "error/X_log.h"
#include "error/X_error.h"
#include "memory/X_alloc.h"

#define ASSERT_OPEN_FOR_READING(_file) x_assert(x_file_is_open_for_reading(_file), "Attemping to read from unopened file")

static inline void determineFileSize(X_File* file)
{
    fseek(file->file, 0, SEEK_END);
    file->size = ftell(file->file);
    rewind(file->file);
}

_Bool x_file_open_reading(X_File* file, const char* fileName)
{
    file->file = fopen(fileName, "rb");
    file->flags = 0;
    file->size = 0;
    
    if(!file->file)
    {
        x_log_error("Failed to open file '%s' for reading", fileName);
        return 0;
    }
    
    x_log("Opened file '%s' for reading", fileName);
    file->flags = X_FILE_OPEN_FOR_READING;
    determineFileSize(file);
    
    return 1;
}

void x_file_close(X_File* file)
{
    if(!x_file_is_open(file))
    {
        x_log_error("Attempting to close unopened file");
        return;
    }
    
    fclose(file->file);
    file->file = NULL;
    file->flags = 0;
    file->size = 0;
}

unsigned char* x_file_read_contents(const char* fileName)
{
    X_File file;
    if(!x_file_open_reading(&file, fileName))
        return NULL;
    
    unsigned char* data = x_malloc(file.size);
    fread(data, 1, file.size, file.file);
    
    x_file_close(&file);
    return data;
}

int x_file_read_char(X_File* file)
{
    ASSERT_OPEN_FOR_READING(file);
    return fgetc(file->file);
}

void x_file_read_cstr(X_File* file, char* dest)
{
    ASSERT_OPEN_FOR_READING(file);
    
    int c;
    while(!feof(file->file) && (c = fgetc(file->file)) != '\0')
        *dest++ = c;
    
    *dest = '\0';
}

int x_file_read_le_int(X_File* file)
{
    ASSERT_OPEN_FOR_READING(file);
    
    unsigned int val = 0;
    for(int i = 0; i < 4; ++i)
        val |= fgetc(file->file) << (i * 8);
    
    return val;
}

int x_file_read_be_int(X_File* file)
{
    ASSERT_OPEN_FOR_READING(file);
    
    unsigned int val = 0;
    for(int i = 3; i >= 0; --i)
        val |= fgetc(file->file) << (i * 8);
    
    return val;
}

void x_file_seek(X_File* file, size_t pos)
{
    x_assert(x_file_is_open(file), "Trying to seek on unopened file");
    fseek(file->file, pos, SEEK_SET);
}

void x_file_read_fixed_length_str(X_File* file, int strLength, char* dest)
{
    ASSERT_OPEN_FOR_READING(file);
    
    dest += fread(dest, 1, strLength, file->file);
    *dest = '\0';
}

void x_file_read_buf(X_File* file, int bufSize, char* dest)
{
    ASSERT_OPEN_FOR_READING(file);
    fread(dest, 1, bufSize, file->file);
}

void x_file_read_vec3(X_File* file, X_Vec3* dest)
{
    ASSERT_OPEN_FOR_READING(file);
    dest->x = x_file_read_le_int(file);
    dest->y = x_file_read_le_int(file);
    dest->z = x_file_read_le_int(file);
}
