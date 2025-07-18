#include "kernel.h"
#include "global.h"

void CKernel::GenerateH264ParserInfo( int file_index)
{
        CString bufferParser = m_H264Parser.m_DebugCharArray[file_index];
        filesystem_save_log_file( "emmc1-1", VID__LOG_NAMES[file_index], bufferParser);   
}
void CKernel::GenerateBmpParserInfo( int file_index)
{
        CString bufferParser = m_H264Parser.m_DebugCharArray[file_index];
        filesystem_save_log_file( "emmc1-1", BMP__LOG_NAMES[file_index], bufferParser);   
}
/*
void            CKernel::parser_debug               (int fromFile, int toFile)
{
    for (int i = fromFile; i < toFile; i++) 
        {
                m_H264Parser.ParseVideo(i, m_bufferVideo, VID_LOADED_BYTES );
                GenerateH264ParserInfo  (i);
        }
}
*/
void            CKernel::parser_h264               (int fromFile, int toFile)
{
    for (int i = fromFile; i < toFile; i++) 
        {
                m_H264Parser.ParseVideo(i, m_bufferVideo, VID_LOADED_BYTES );
                GenerateH264ParserInfo  (i);
        }
}

void            CKernel::parser_bmp               (int fromFile, int toFile)
{
    for (int i = fromFile; i < toFile; i++) 
        {
                m_H264Parser.ParseBPM(i, m_bufferTexture, VID_LOADED_BYTES );
                GenerateBmpParserInfo  (i);
        }
}

