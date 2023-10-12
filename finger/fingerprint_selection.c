 
#include "fingerprint_selection.h" 
#include "fingerprint_capture.h"  
#include <errno.h> 

// Función para seleccionar y abrir un lector
DPFPDD_DEV SelectAndOpenReader(char* szReader, size_t nReaderLen, int *pDPI) {
    DPFPDD_DEV hReader = NULL;
    strncpy(szReader, "", nReaderLen);
    int bStop = 0;

    // Bucle para buscar el lector y abrirlo
    while(!bStop) {
        // Consulta de información de los dispositivos
        unsigned int nReaderCnt = 1;
        DPFPDD_DEV_INFO* pReaderInfo = (DPFPDD_DEV_INFO*)malloc(sizeof(DPFPDD_DEV_INFO) * nReaderCnt);
        while(NULL != pReaderInfo) {
            unsigned int i = 0;
            for(i = 0; i < nReaderCnt; i++) {
                pReaderInfo[i].size = sizeof(DPFPDD_DEV_INFO);
            }

            unsigned int nNewReaderCnt = nReaderCnt;
            int result = dpfpdd_query_devices(&nNewReaderCnt, pReaderInfo);

            // Manejo de errores en la consulta de dispositivos
            if(DPFPDD_SUCCESS != result && DPFPDD_E_MORE_DATA != result) {
                printf("Error en dpfpdd_query_devices(): %d", result);
                free(pReaderInfo);
                pReaderInfo = NULL;
                nReaderCnt = 0;
                break;
            }

            if(DPFPDD_E_MORE_DATA == result) {
                DPFPDD_DEV_INFO* pri = (DPFPDD_DEV_INFO*)realloc(pReaderInfo, sizeof(DPFPDD_DEV_INFO) * nNewReaderCnt);
                if(NULL == pri) {
                    printf("Error en realloc(): ENOMEM");
                    break;
                }
                pReaderInfo = pri;
                nReaderCnt = nNewReaderCnt;
                continue;
            }

            nReaderCnt = nNewReaderCnt;
            break;
        }

        // Selección del lector y obtención de sus capacidades
        int result = 0;
        int nChoice = 0;
        if(nReaderCnt > nChoice) {
            // Apertura del lector seleccionado
            result = dpfpdd_open(pReaderInfo[nChoice].name, &hReader);
            if(DPFPDD_SUCCESS == result) {
                strncpy(szReader, pReaderInfo[nChoice].name, nReaderLen);

                // Obtención de las capacidades del lector
                unsigned int nCapsSize = sizeof(DPFPDD_DEV_CAPS);
                while(1) {
                    DPFPDD_DEV_CAPS* pCaps = (DPFPDD_DEV_CAPS*)malloc(nCapsSize);
                    if(NULL == pCaps) {
                        printf("Error en malloc()");
                        break;
                    }
                    pCaps->size = nCapsSize;
                    result = dpfpdd_get_device_capabilities(hReader, pCaps);

                    // Manejo de errores al obtener las capacidades del dispositivo
                    if(DPFPDD_SUCCESS != result && DPFPDD_E_MORE_DATA != result) {
                        printf("Error en dpfpdd_get_device_capabilities()");
                        free(pCaps);
                        break;
                    }
                    if(DPFPDD_E_MORE_DATA == result) {
                        nCapsSize = pCaps->size;
                        free(pCaps);
                        continue;
                    }

                    *pDPI = pCaps->resolutions[0];
                    free(pCaps);
                    break;
                }
            } else {
                printf("Error en dpfpdd_open()");
            }
            bStop = 1;
        }

        if(NULL != pReaderInfo) free(pReaderInfo);
        pReaderInfo = NULL;
        nReaderCnt = 0;
    }

    return hReader;
}
