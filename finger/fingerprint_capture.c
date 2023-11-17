
#include "fingerprint_capture.h"  
#include <errno.h> 
#include <signal.h>
#include <sys/time.h>
#include <stdio.h>  

#pragma pack(push, 1) // Asegura el alineamiento de 1 byte
typedef struct {
    unsigned short type;
    unsigned int size;
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int offset;
    unsigned int header_size;
    int width;
    int height;
    unsigned short planes;
    unsigned short bits_per_pixel;
    unsigned int compression;
    unsigned int image_size;
    int x_pixels_per_meter;
    int y_pixels_per_meter;
    unsigned int colors_used;
    unsigned int colors_important;
} BMPHeader;
#pragma pack(pop)

DPFPDD_DEV g_hReader = NULL;

// Función para manejar señales
void signal_handler(int nSignal) {
    
    printf("apagando huellero .\n");

    if(SIGINT == nSignal){ 
        if(NULL != g_hReader) dpfpdd_cancel(g_hReader);
    }
}

// Función para capturar la huella y extraer características
char* CaptureFinger(const char* szFingerName, DPFPDD_DEV hReader, int dpi, DPFJ_FMD_FORMAT nFtType, unsigned char** ppFt){
    int result = 0;
    int timeWait = -1;  
    DPFPDD_CAPTURE_PARAM cparam = {0};
    cparam.size = sizeof(cparam);
    cparam.image_fmt = DPFPDD_IMG_FMT_ISOIEC19794;
    cparam.image_proc = DPFPDD_IMG_PROC_NONE;
    cparam.image_res = dpi;
    DPFPDD_CAPTURE_RESULT cresult = {0};
    cresult.size = sizeof(cresult);
    cresult.info.size = sizeof(cresult.info);

    unsigned int nOrigImageSize = 0;
    result = dpfpdd_capture(hReader, &cparam, 0, &cresult, &nOrigImageSize, NULL);
   
    if(DPFPDD_E_MORE_DATA != result){
        return "time dpfpdd_capture"; 
    }

    unsigned char* pImage = (unsigned char*)malloc(nOrigImageSize);
    if(NULL == pImage){ 
        return "error malloc"; 
    }

    g_hReader = hReader;
    struct sigaction new_action, old_action;
    new_action.sa_handler = &signal_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
 

    sigaction(SIGINT, &new_action, &old_action);

    sigset_t new_sigmask, old_sigmask;
    sigemptyset(&new_sigmask);

    sigaddset(&new_sigmask, SIGINT);

    pthread_sigmask(SIG_UNBLOCK, &new_sigmask, &old_sigmask);

    unsigned int nImageSize = nOrigImageSize;
    signal(SIGINT, signal_handler);

    result = dpfpdd_capture(hReader, &cparam, timeWait, &cresult, &nImageSize, pImage);

    if (DPFPDD_SUCCESS != result) {
        return "Error capturing fingerprint";
    } else {
        if (cresult.success) {

            unsigned int nFeaturesSize = MAX_FMD_SIZE;
            unsigned char* pFeatures = (unsigned char*)malloc(nFeaturesSize);
            if (NULL == pFeatures) {
                return "Error allocating memory";
            } else {
                long mseconds;
                struct timeval tv1, tv2;
                gettimeofday(&tv1, NULL);

                result = dpfj_create_fmd_from_fid(DPFJ_FID_ISO_19794_4_2005, pImage, nImageSize, nFtType, pFeatures, &nFeaturesSize);

                gettimeofday(&tv2, NULL);
                mseconds = (tv2.tv_sec - tv1.tv_sec) * 1000 + (tv2.tv_usec - tv1.tv_usec) / 1000; // time of operation in milliseconds

                if (DPFJ_SUCCESS == result) {                    
                    saveBMP("fingerprint.bmp", pImage, cresult.info.width, cresult.info.height);

                    return "Fingerprint captured and features extracted";
                } else {
                    free(pFeatures);
                    return "Error creating template";
                }
            }
        } else if (DPFPDD_QUALITY_CANCELED == cresult.quality) {
            return "Capture canceled";
        } else {
            return "Timeout";
        }
    }
}

// Función para guardar una imagen BMP
void saveBMP(const char* filename, unsigned char* imageData, int width, int height) {
    int leftOffset = -270;
    int newWidth = width - leftOffset;
    // Calcular el ancho para recortar por la mitad verticalmente
    int newHalfWidth = newWidth / 1.90;

    // Inicializar la estructura de la cabecera BMP
    BMPHeader header;
    memset(&header, 0, sizeof(BMPHeader));
    header.type = 0x4D42; // "BM" en little-endian
    header.size = sizeof(BMPHeader) + (newHalfWidth * 3 + ((4 - (newHalfWidth * 3) % 4) % 4)) * height;
    header.offset = sizeof(BMPHeader);
    header.header_size = 40;
    header.width = newHalfWidth; // Nuevo ancho
    header.height = height;     // Height es positivo para imágenes bottom-up
    header.planes = 1;
    header.bits_per_pixel = 24;
    header.compression = 0;
    header.image_size = (newHalfWidth * 3 + ((4 - (newHalfWidth * 3) % 4) % 4)) * height;

    FILE* outfile = fopen(filename, "wb");
    if (outfile == NULL) {
        printf("No se pudo abrir el archivo para escritura.\n");
        return;
    }

    fwrite(&header, sizeof(BMPHeader), 1, outfile);

    int padding = (4 - (newHalfWidth * 3) % 4) % 4;

    for (int y = height - 1; y >= 0; --y) {
        for (int x = leftOffset; x < leftOffset + newHalfWidth; ++x) {
            unsigned char pixelValue = imageData[y * width + x];

            fwrite(&pixelValue, 1, 1, outfile);  // Blue
            fwrite(&pixelValue, 1, 1, outfile);  // Green
            fwrite(&pixelValue, 1, 1, outfile);  // Red
        }

        for (int p = 0; p < padding; ++p) {
            fputc(0, outfile);
        }
    }

    fclose(outfile);
}
