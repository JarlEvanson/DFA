#include "common.h"
#include "buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initBuffer( Buffer* buffer, const char* fileName ) {
    buffer->mFile = fopen( fileName, "w" );
    if( buffer->mFile == NULL ) {
        printf( "Failed to open %s\n", fileName );
        exit( 5 );
    }
    buffer->mCurrentFile = fileName;
    buffer->mCurrBufferLoc = &buffer->mBuffer[0][0];
}

size_t writeBuffer( Buffer* buffer, const char* fmt, ... ) {
    va_list list;

    va_start( list, fmt );

    size_t bytesWritten = vsprintf( buffer->mCurrBufferLoc, fmt, list );
    buffer->mCurrBufferLoc += bytesWritten;
    if( buffer->mCurrBufferLoc - &buffer->mBuffer[0][0] > 4096 ) {
        if( fwrite( &buffer->mBuffer[0][0], 4096, 1, buffer->mFile ) ) {
            memcpy( &buffer->mBuffer[0][0], &buffer->mBuffer[1][0], 4096 );
            buffer->mCurrBufferLoc -= 4096;
        }
    }

    va_end( list );

    return bytesWritten;
}

void clearBuffer( Buffer* buffer ) {
    if( fwrite( &buffer->mBuffer[0][0], buffer->mCurrBufferLoc - &buffer->mBuffer[0][0], 1, buffer->mFile ) ) {
        buffer->mCurrBufferLoc = &buffer->mBuffer[0][0];
    }
}

void closeFile( Buffer* buffer ) {
    fclose( buffer->mFile );
}
