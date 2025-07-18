#include "kernel.h"
#include "global.h"

bool            CKernel::filesystem_open_file       (   const char *pTitle)		                        // Open File by passing pTitle via pointer 
{   
	            hFile = m_pFileSystem->FileOpen (pTitle);                               // !!! hFile has to be defined globally !!!
	            if (hFile == 0)
		            {
		            return false;							                                    // Cannot open file
		            }	
                return true;								                                    // file opened successful
}
unsigned        CKernel::filesystem_load_file       (   char *buffer, 
                                                        unsigned bufferSize, 
                                                        int mode)
{
                unsigned totalBytesRead = 0;
                unsigned bytesRead;
    
                while (totalBytesRead < bufferSize)
                    {
                    opaque = 0.5f;
                    unsigned currentChunkSize = (bufferSize - totalBytesRead < CHUNK_SIZE) ? 
                                                (bufferSize - totalBytesRead) : CHUNK_SIZE;
                    bytesRead = m_pFileSystem->FileRead(hFile, buffer + totalBytesRead, currentChunkSize);

                    if (bytesRead == FS_ERROR)
                        {
                        opaque = 1.0f;    
                        return 0;  // Read error
                        }
                    if (bytesRead == 0)
                        {
                        opaque = 1.0f;    
                        return totalBytesRead;  // EOF reached, return total bytes read
                        }
                    totalBytesRead += bytesRead;

                        display_LoadScreenTexVidShd(mode);

                    m_Watchdog.Start(TIMEOUT);         // new watchdog    
                    }
                opaque = 1.0f;    
                return 0;  // Buffer full, EOF not reached - this is NOT a success - 0 is equal to false
}
bool            CKernel::filesystem_close_file      ()	                                                 // close file ( release hFile handle ) 
{
	            if (!m_pFileSystem->FileClose (hFile))		                                    // Close File
		            {
		            return false;							                                    // Cannot close file
		            }
                return true;								                                    // file has closed successful
}
int             CKernel::filesystem_process_files   (   char* fileNameArray[], 
                                                        unsigned totalLoadedBytes[], 
                                                        char** bufferArray, 
                                                        int maxFiles, 
                                                        int successfulLoaded, 
                                                        unsigned fileSize, 
                                                        int mode)
{
                for (int i = 0; i < maxFiles; ++i) 
                    {
                    if (filesystem_open_file(fileNameArray[i]))
                        {
                        unsigned bytesRead = filesystem_load_file(bufferArray[successfulLoaded], fileSize, mode);
                        if (bytesRead)
                            {
                            totalLoadedBytes[successfulLoaded] = bytesRead;
                            successfulLoaded++;   
                            }
                        filesystem_close_file();
                        }
                    }   
                return successfulLoaded;
}
bool            CKernel::filesystem_mount           (   const char* deviceName, 
                                                        char* vshaderFileNames[], 
                                                        unsigned vStotalLoadedBytes[], 
                                                        int maxVshaderFiles,
                                                        char* fshaderFileNames[], 
                                                        unsigned fStotalLoadedBytes[], 
                                                        int maxFshaderFiles,
                                                        char* textureFileNames[], 
                                                        unsigned tXtotalLoadedBytes[], 
                                                        int maxTextureFiles,
                                                        char* videoFileNames[]  , 
                                                        unsigned vItotalLoadedBytes[], 
                                                        int maxVideoFiles)
{
                bool success = false;
                
                CDevice *pPartition = m_DeviceNameService.GetDevice(deviceName, TRUE);      // Retrieve a partition device

                if (pPartition != 0
                    && (m_pFileSystem = new CFATFileSystem) != 0
                    && m_pFileSystem->Mount(pPartition))
                    {
                    scanned_vsh = filesystem_ScanRootDir(vshaderFileNames, "vsh", maxVshaderFiles);
                    scanned_fsh = filesystem_ScanRootDir(fshaderFileNames, "fsh", maxFshaderFiles);
                    scanned_tex = filesystem_ScanRootDir(textureFileNames, "bmp", maxTextureFiles);
                    scanned_vid = filesystem_ScanRootDir(videoFileNames,   "264", maxVideoFiles);

                    VSH_LOADED_NEW = filesystem_process_files(  vshaderFileNames, vStotalLoadedBytes, m_bufferVshader, 
                                                                scanned_vsh, VSH_LOADED_NEW, VSH_SIZE, 0);  // The file system was mounted successfully                 

                    FSH_LOADED_NEW = filesystem_process_files(  fshaderFileNames, fStotalLoadedBytes, m_bufferFshader, 
                                                                scanned_fsh, FSH_LOADED_NEW, FSH_SIZE, 1);                               

                    TEX_LOADED_NEW = filesystem_process_files(  textureFileNames, tXtotalLoadedBytes, m_bufferTexture, 
                                                                scanned_tex, TEX_LOADED_NEW, TEX_SIZE, 2);                                   
                  
                    VID_LOADED_NEW = filesystem_process_files(  videoFileNames  , vItotalLoadedBytes, m_bufferVideo  , 
                                                                scanned_vid  , VID_LOADED_NEW , VID_SIZE  , 3);                                

                    m_pFileSystem->UnMount();
                    success = true;
                    }
                delete m_pFileSystem;
                m_pFileSystem = 0;

                return success;   // Handle the case where no partition device was found or mounting failed
}

bool            CKernel::filesystem_save_log_file   (   const char* deviceName, 
                                                        const char* filename, 
                                                        const CString& str_to_save)
{
                bool success = false;   
                
                CDevice *pPartition = m_DeviceNameService.GetDevice(deviceName, TRUE);  // Get partition device
                
                if (pPartition != 0 && (m_pFileSystem = new CFATFileSystem) != 0  && m_pFileSystem->Mount(pPartition))
                {
                    unsigned hFile = m_pFileSystem->FileCreate(filename);   // Create and write shader log file
                    if (hFile != 0)
                    {
                        
                        if (m_pFileSystem->FileWrite(hFile, (const char*)str_to_save, str_to_save.GetLength()) == str_to_save.GetLength())   // Write the shader log
                        {
                            success = true;
                        }
                        m_pFileSystem->FileClose(hFile);
                    }
                    m_pFileSystem->UnMount();   // Unmount filesystem
                }
                delete m_pFileSystem;           // Cleanup filesystem object
                m_pFileSystem = 0;
                
                return success;
}

bool            CKernel::filesystem_IsValidFileType (   const char* pFileName, const char* extension)
{
                CString fileName(pFileName);
                int dotPos = fileName.Find('.');
                if (dotPos == -1 || dotPos == 0 || dotPos > 8) 
                    {
                    return FALSE;
                    }
                CString suffix((const char*)fileName + dotPos + 1);
                return suffix.Compare(extension) == 0;
}

unsigned        CKernel::filesystem_ScanRootDir     (   char** fileArray, 
                                                        const char* extension, 
                                                        unsigned maxFiles)
{
                TDirentry Direntry;
                TFindCurrentEntry CurrentEntry;
                unsigned count = 0;
                
                unsigned nEntry = m_pFileSystem->RootFindFirst(&Direntry, &CurrentEntry);
                if(nEntry == 0)  // Initial directory access failed
                    {
                    return 0;    // Return 0 to indicate failure/no files found
                    }

                while (nEntry != 0 && count < maxFiles) 
                    {
                    if (!(Direntry.nAttributes & FS_ATTRIB_SYSTEM)) 
                    {
                        if (filesystem_IsValidFileType(Direntry.chTitle, extension)) 
                        {
                            fileArray[count] = new char[strlen(Direntry.chTitle) + 1];
                            strcpy(fileArray[count], Direntry.chTitle);
                            count++;
                        }
                    }
                    nEntry = m_pFileSystem->RootFindNext(&Direntry, &CurrentEntry);
                    }
                return count;    // Return actual number of files found and loaded
}

bool            CKernel::filesystem_update_USB      (   const char* deviceType)
{
                if (m_USBHCI.UpdatePlugAndPlay())   // Update the tree of connected USB devices
                    {
                    CDevice *pDevice = m_DeviceNameService.GetDevice(deviceType, TRUE);
                    if (pDevice != nullptr)
                        {
                        m_bStorageAttached = true;

                        pDevice->RegisterRemovedHandler(filesystem_remove_USB, this);
                        return true;
                        }
                    }
                return false;
}
void            CKernel::filesystem_remove_USB      (   CDevice *pDevice, void *pContext)
{
	            CKernel *pThis = (CKernel *) pContext;
	            assert (pThis != 0);
	        //  assert (pThis->m_bStorageAttached);
	            pThis->m_bStorageAttached = FALSE;
}

bool            CKernel::filesystem_load_kernel     (   const char* deviceName, 
                                                        const char* filename, 
                                                        unsigned bufferIndex)
{
                while(bufferIndex == 1 && filesystem_update_USB("umsd1") == false)
                {
                    m_Timer.MsDelay(100);
                }

                CDevice *pPartition = m_DeviceNameService.GetDevice(deviceName, TRUE);
                        
                if (pPartition != 0 && (m_pFileSystem = new CFATFileSystem) != 0 && m_pFileSystem->Mount(pPartition))
                {
                    if (filesystem_open_file(filename))
                    {

                        loaded_bytes_kernel[bufferIndex] = filesystem_load_file(m_bufferKernel[bufferIndex], KERNEL_SIZE, 4);
                        if (loaded_bytes_kernel[bufferIndex] > 0)
                        {
                            filesystem_close_file();
                            m_pFileSystem->UnMount();
                            delete m_pFileSystem;
                            m_pFileSystem = 0;
                            return true;
                        }
                        filesystem_close_file();
                    }
                    m_pFileSystem->UnMount();
                }
                delete m_pFileSystem;
                m_pFileSystem = 0;
                return false;
}

bool            CKernel::filesystem_save_kernel     (   const char* deviceName, 
                                                        const char* filename, 
                                                        unsigned bufferIndex)
{
                bool success = false;   
                
                CDevice *pPartition = m_DeviceNameService.GetDevice(deviceName, TRUE);
                
                if (pPartition != 0 && (m_pFileSystem = new CFATFileSystem) != 0 && m_pFileSystem->Mount(pPartition))
                {
                    unsigned hFile = m_pFileSystem->FileCreate(filename);
                    if (hFile != 0)
                    {
                        if (m_pFileSystem->FileWrite(hFile, m_bufferKernel[bufferIndex], loaded_bytes_kernel[bufferIndex]) == loaded_bytes_kernel[bufferIndex])
                        {
                            success = true;
                        }
                        m_pFileSystem->FileClose(hFile);
                    }
                    m_pFileSystem->UnMount();
                }
                delete m_pFileSystem;
                m_pFileSystem = 0;
                
                return success;
}

