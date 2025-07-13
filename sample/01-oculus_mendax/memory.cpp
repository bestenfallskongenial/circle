#include "kernel.h"
#include "global.h"

bool            CKernel::memory_allocate            ()
{
                // Video and frame buffers need DMA memory
                m_bufferVideo       = memory_init_dma_buffer(   VID_SD + VID_USB, VID_SIZE,
                                                                &m_videoBlockBase,
                                                                &m_videoRawBlock,
                                                                &m_videoBlockSize);
                m_bufferFrameBufferA= memory_init_dma_buffer(   1, FRAME_SIZE,
                                                                &m_frameBlockBaseA,
                                                                &m_frameRawBlockA,
                                                                &m_frameBlockSizeA);
                m_bufferFrameBufferB= memory_init_dma_buffer(   1, FRAME_SIZE,
                                                                &m_frameBlockBaseB,
                                                                &m_frameRawBlockB,
                                                                &m_frameBlockSizeB);
                m_bufferTexture     = memory_init_dma_buffer(   TEX_SD + TEX_USB, TEX_SIZE,
                                                                &m_textureBlockBase,
                                                                &m_textureRawBlock,
                                                                &m_textureBlockSize);

                m_bufferKernel      = memory_init_buffer( 2, KERNEL_SIZE );

                m_bufferVshader     = memory_init_buffer( VSH_SD + VSH_USB, VSH_SIZE );
                m_bufferFshader     = memory_init_buffer( FSH_SD + FSH_USB, VSH_SIZE );
                if (!m_bufferVideo || !m_bufferFrameBufferA || !m_bufferFrameBufferA || !m_bufferTexture ||
                    !m_bufferKernel || !m_bufferVshader || !m_bufferFshader)
                    {
                    memory_clean_up();
                    return false;
                    }
                return true;
}

void            CKernel::memory_clean_up            ()
{
                if (m_bufferVideo) 
                    { 
                    memory_clear_dma_buffer( m_bufferVideo, m_videoRawBlock); 
                    m_bufferVideo = nullptr; 
                    m_videoRawBlock = nullptr;
                    }
                if (m_bufferFrameBufferA) 
                    { 
                    memory_clear_dma_buffer( m_bufferFrameBufferA, m_frameRawBlockA); 
                    m_bufferFrameBufferA = nullptr; 
                    m_frameRawBlockA = nullptr;
                    }
                if (m_bufferFrameBufferB) 
                    { 
                    memory_clear_dma_buffer( m_bufferFrameBufferB, m_frameRawBlockB); 
                    m_bufferFrameBufferB = nullptr; 
                    m_frameRawBlockB = nullptr;
                    }    
                if (m_bufferTexture) 
                    { 
                    memory_clear_buffer( m_bufferTexture, 
                                        TEX_SD + TEX_USB); 
                    m_bufferTexture = nullptr; 
                    }
                if (m_bufferKernel) 
                    { 
                    memory_clear_buffer( m_bufferKernel, 2 ); 
                    m_bufferKernel = nullptr; 
                    }
                if (m_bufferVshader) 
                    { 
                    memory_clear_buffer( m_bufferVshader, 
                                        VSH_SD + VSH_USB); 
                    m_bufferVshader = nullptr; 
                    }
                if (m_bufferFshader) 
                    { 
                    memory_clear_buffer( m_bufferFshader, 
                                        FSH_SD + FSH_USB); 
                    m_bufferFshader = nullptr; 
                    }                                   
}

char**          CKernel::memory_init_buffer         (   size_t count, size_t bufferSize) 
{
            //  #define LOG_NAME "ALLOC-STD"

                char** buffers = (char**)malloc(count * sizeof(char*));
#ifdef ALLOC_DEBUG                   
                CLogger::Get()->Write("ALLOC-STD", LogDebug, "buffers = 0x%p", buffers);
#endif // ALLOC_DEBUG

                for (size_t i = 0; i < count; ++i) 
                {
                    buffers[i] = (char*)calloc(bufferSize, sizeof(char));
#ifdef ALLOC_DEBUG                    
                    CLogger::Get()->Write("ALLOC-STD", LogDebug, "buffers[%u] = 0x%p", (unsigned)i, buffers[i]);
#endif // ALLOC_DEBUG
                }
#ifdef ALLOC_DEBUG   
                CLogger::Get()->Write("ALLOC-STD", LogDebug, "final buffers = 0x%p count = %u bufferSize = %u", buffers, (unsigned)count, (unsigned)bufferSize);
#endif // ALLOC_DEBUG                
                msleep(100);
                return buffers;
}

char**          CKernel::memory_init_dma_buffer     (   size_t count, 
                                                        size_t bufferSize,
                                                        char** blockBaseOut,
                                                        char** rawBlockOut,
                                                        size_t* alignedSizeOut)
{
            //  #define LOG_NAME "ALLOC-STD"

                size_t total_size = count * bufferSize;
                size_t aligned_total_size = (total_size + 4095) & ~4095;

                // Allocate +4096 for manual alignment
                char* raw = new (HEAP_DMA30) char[aligned_total_size + 4096];
#ifdef ALLOC_DEBUG   
                CLogger::Get()->Write("ALLOC-STD", LogDebug, "raw = 0x%p", raw);
#endif // ALLOC_DEBUG
                char* dma_block = (char*)(((uintptr_t)raw + 4095) & ~4095);  // 4K-aligned
#ifdef ALLOC_DEBUG   
                CLogger::Get()->Write("ALLOC-STD", LogDebug, "dma_block (aligned) = 0x%p", dma_block);
#endif // ALLOC_DEBUG
                // Build slice table
                char** buffers = new char*[count];
                for (size_t i = 0; i < count; ++i)
                {
                    buffers[i] = dma_block + i * bufferSize;
#ifdef ALLOC_DEBUG   
                CLogger::Get()->Write("ALLOC-STD", LogDebug, "buffers[%u] = 0x%p", (unsigned)i, buffers[i]);
#endif // ALLOC_DEBUG            
                    memset(buffers[i], 0, bufferSize);
                }
#ifdef ALLOC_DEBUG   
                CLogger::Get()->Write("ALLOC-STD", LogDebug, "raw = 0x%p dma_block = 0x%p aligned_size = 0x%X", raw, dma_block, (unsigned)aligned_total_size);
#endif // ALLOC_DEBUG
                *blockBaseOut = dma_block;
                *rawBlockOut = raw;
                *alignedSizeOut = aligned_total_size;

                msleep(100);    
                return buffers;
}

void            CKernel::memory_clear_buffer        (   char** buffers, size_t count) 
{
                for (size_t i = 0; i < count; ++i) 
                    {
                    free(buffers[i]);
                    }
                free(buffers);
}

void            CKernel::memory_clear_dma_buffer    (   char** buffers, char* rawBlock)
{
                delete[] rawBlock;  // Raw block from new[]
                delete[] buffers;   // Slice table
}

