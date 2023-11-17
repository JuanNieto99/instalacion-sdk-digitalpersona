 
#include "fingerprint_selection.h" 
#include "fingerprint_capture.h"  
#include <errno.h> 

// Función para seleccionar y abrir un lector
DPFPDD_DEV SelectAndOpenReader(char* szReader, size_t nReaderLen, int *pDPI) {
    DPFPDD_DEV hReader = NULL;
    strncpy(szReader, "", nReaderLen);
    
    // Bucle para buscar el lector y abrirlo
 //   while(!bStop) {
        // Consulta de información de los dispositivos
        unsigned int nReaderCnt = 1;
        DPFPDD_DEV_INFO* pReaderInfo = (DPFPDD_DEV_INFO*)malloc(sizeof(DPFPDD_DEV_INFO) * nReaderCnt);
      //  while(NULL != pReaderInfo) {
 
       
            unsigned int nNewReaderCnt = nReaderCnt;
            int result = dpfpdd_query_devices(&nNewReaderCnt, pReaderInfo);

 
 
            nReaderCnt = nNewReaderCnt;
           // break;
       // }

        // Selección del lector y obtención de sus capacidades
        int result2 = 0;
        int nChoice = 0;
        if(nReaderCnt > nChoice) {
            // Apertura del lector seleccionado
            result2 = dpfpdd_open(pReaderInfo[nChoice].name, &hReader);
            if(DPFPDD_SUCCESS == result2) {
                strncpy(szReader, pReaderInfo[nChoice].name, nReaderLen);

                // Obtención de las capacidades del lector
                unsigned int nCapsSize = sizeof(DPFPDD_DEV_CAPS);

                    DPFPDD_DEV_CAPS* pCaps = (DPFPDD_DEV_CAPS*)malloc(nCapsSize);
                    if(NULL == pCaps) {
                        printf("Error en malloc()");                      
                    }
                    pCaps->size = nCapsSize;
                    result2 = dpfpdd_get_device_capabilities(hReader, pCaps);

                    // Manejo de errores al obtener las capacidades del dispositivo
                    if(DPFPDD_SUCCESS != result2 && DPFPDD_E_MORE_DATA != result2) {
                        printf("Error en dpfpdd_get_device_capabilities()");
                        free(pCaps);                       
                    }
                    if(DPFPDD_E_MORE_DATA == result2) {
                        nCapsSize = pCaps->size;
                        free(pCaps);                      
                    }

                    *pDPI = pCaps->resolutions[0];
                    free(pCaps);
                    
                
            } else {
                printf("Error en dpfpdd_open()");
            }
    
        }

        if(NULL != pReaderInfo) free(pReaderInfo);
        pReaderInfo = NULL;
        nReaderCnt = 0;
  //  }

    return hReader;
}

 