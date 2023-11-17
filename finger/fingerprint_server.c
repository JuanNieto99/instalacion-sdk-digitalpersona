#include <stdio.h>
#include <string.h>
#include <microhttpd.h>
#include "fingerprint_capture.h"
#include "fingerprint_selection.h"
#include <dpfpdd.h>
#include <stdlib.h>
#include <signal.h>
#include <locale.h>
#include <pthread.h>

#define PORT 5050
#define CONNECTION_LIMIT 10  // Ajusta según tus necesidades

struct ThreadData {
    struct MHD_Connection *connection;
    const char *url;
    const char *method;
};

void sigint_handler(int signum) {
    // No hacer nada, simplemente ignorar la señal
}
// Función para codificar en Base64
char* base64_encode(const unsigned char* input, size_t length, size_t* output_length);

// Función para leer el contenido de un archivo en un búfer
unsigned char* read_file(const char* filename, size_t* length);

int request_handler2(void *cls, struct MHD_Connection *connection,
                     const char *url, const char *method,
                     const char *version, const char *upload_data,
                     size_t *upload_data_size, void **con_cls) {

    if (strcmp(url, "/close") == 0) {
        FILE* fp = popen("tmux send-keys -t finger_sesion C-c", "r");

        if (fp == NULL) {
            perror("popen");
            exit(EXIT_FAILURE);
        }

        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("%s", buffer);
        }

        pclose(fp);
        
        const char *json_response = "{\"message\": \"huellero cancelado\", \"type\": \"true\"}";

        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(json_response),
                                                                        (void *) json_response,
                                                                        MHD_RESPMEM_MUST_COPY);
         
        int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);

        return ret;
    }

    return MHD_NO;  // Página no encontrada
}
// Función para manejar las solicitudes HTTP
int request_handler(void *cls, struct MHD_Connection *connection,
                    const char *url, const char *method,
                    const char *version, const char *upload_data,
                    size_t *upload_data_size, void **con_cls) {

    struct MHD_Response *response;

    if (strcmp(url, "/isConnected") == 0) { 
        // Inicialización del lector de huellas y variables
      //  int result = dpfpdd_init();
        DPFPDD_DEV hReader = NULL;
        int dpi = 0;
        int bStop = 0;
        int result = dpfpdd_init();
        char szReader[MAX_DEVICE_NAME_LENGTH];
        sigset_t sigmask;

        // Configuración de máscara de señales
        sigfillset(&sigmask);
        pthread_sigmask(SIG_BLOCK, &sigmask, NULL);
        
        // Configuración de localización
        setlocale(LC_ALL, "");
        strncpy(szReader, "", sizeof(szReader));

        // Intento de obtener información sobre el lector de huellas
        unsigned int nReaderCnt = 1;
        while(!bStop) {
            // Consulta de información de los dispositivos
    
            DPFPDD_DEV_INFO* pReaderInfo = (DPFPDD_DEV_INFO*)malloc(sizeof(DPFPDD_DEV_INFO) * nReaderCnt);
            while(NULL != pReaderInfo) {
                unsigned int i = 0;
                for(i = 0; i < nReaderCnt; i++) {
                    pReaderInfo[i].size = sizeof(DPFPDD_DEV_INFO);
                }

                unsigned int nNewReaderCnt = nReaderCnt;
                int result2 = dpfpdd_query_devices(&nNewReaderCnt, pReaderInfo);

                // Manejo de errores en la consulta de dispositivos
                if(DPFPDD_SUCCESS != result2 && DPFPDD_E_MORE_DATA != result2) {

                    response = MHD_create_response_from_buffer(strlen("{\"message\": \"Huellero no conectado\", \"type\": \"false\"} "),
                                                            (void *) "{\"message\": \"Huellero no conectado\", \"type\": \"false\"} ",
                                                            MHD_RESPMEM_MUST_COPY);
                                MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
                                MHD_add_response_header(response, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
                                MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");

                                    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
                                    MHD_destroy_response(response);
                                    return ret;
                    printf("Error en dpfpdd_query_devices(): %d", result2);
                    free(pReaderInfo);
                    pReaderInfo = NULL;
                    nReaderCnt = 0;
                    break;
                }

                if(DPFPDD_E_MORE_DATA == result2) {
                    DPFPDD_DEV_INFO* pri = (DPFPDD_DEV_INFO*)realloc(pReaderInfo, sizeof(DPFPDD_DEV_INFO) * nNewReaderCnt);
                    if(NULL == pri) {
                        printf("Error en realloc(): ENOMEM");
                        
                        response = MHD_create_response_from_buffer(strlen("{\"message\": \"Huellero no conectado\", \"type\": \"false\"} "),
                                                            (void *) "{\"message\": \"Huellero no conectado\", \"type\": \"false\"} ",
                                                            MHD_RESPMEM_MUST_COPY);
                                    MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
                                    MHD_add_response_header(response, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
                                    MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");

                                    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
                                    MHD_destroy_response(response);
                                    return ret;
                    }
                    pReaderInfo = pri;
                    nReaderCnt = nNewReaderCnt;
                    continue;
                }

                nReaderCnt = nNewReaderCnt;
                break;
            }

            // Selección del lector y obtención de sus capacidades
            int result2 = 0;
            int nChoice = 0;
    

            // Si no se encuentra ningún lector de huellas
            if(0 == nReaderCnt) {
                response = MHD_create_response_from_buffer(strlen("{\"message\": \"Huellero no conectado\", \"type\": \"false\"} "),
                                                            (void *) "{\"message\": \"Huellero no conectado\", \"type\": \"false\"} ",
                                                            MHD_RESPMEM_MUST_COPY);
                                MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
                                MHD_add_response_header(response, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
                                MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");
                
                if(NULL != hReader){
                    result = dpfpdd_close(hReader);
                    hReader = NULL;
                } 
                
                int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
                MHD_destroy_response(response);
                return ret;
                dpfpdd_exit();  

            } else {
                // Selección y apertura del lector de huellas
                hReader = SelectAndOpenReader(szReader, sizeof(szReader),&dpi);
                                response = MHD_create_response_from_buffer(strlen("{\"message\": \"Huellero  conectado\", \"type\": \"true\"} "),
                                                            (void *) "{\"message\": \"Huellero  conectado\", \"type\": \"true\"} ",
                                                            MHD_RESPMEM_MUST_COPY);
                                MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
                                MHD_add_response_header(response, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
                                MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");
                
                if(NULL != hReader){
                    result = dpfpdd_close(hReader);
                    hReader = NULL;
                } 
                
                int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
                MHD_destroy_response(response);
                return ret;
                dpfpdd_exit();  

            } 
            
            if(NULL != pReaderInfo) free(pReaderInfo);
            pReaderInfo = NULL;
            nReaderCnt = 0;
        }
        
    }

    if (strcmp(url, "/Connect") == 0) {
    
        // Inicialización del lector de huellas y variables
        //  int result = dpfpdd_init();
        DPFPDD_DEV hReader = NULL;
        int dpi = 0;
        int bStop = 0;
        int result = dpfpdd_init();
        char szReader[MAX_DEVICE_NAME_LENGTH];
        sigset_t sigmask;

        // Configuración de máscara de señales
        sigfillset(&sigmask);
        pthread_sigmask(SIG_BLOCK, &sigmask, NULL);
        
        // Configuración de localización
        setlocale(LC_ALL, "");
        strncpy(szReader, "", sizeof(szReader));

        // Intento de obtener información sobre el lector de huellas
        unsigned int nReaderCnt = 1;
        while(!bStop) {
            // Consulta de información de los dispositivos
    
            DPFPDD_DEV_INFO* pReaderInfo = (DPFPDD_DEV_INFO*)malloc(sizeof(DPFPDD_DEV_INFO) * nReaderCnt);
            while(NULL != pReaderInfo) {
                unsigned int i = 0;
                for(i = 0; i < nReaderCnt; i++) {
                    pReaderInfo[i].size = sizeof(DPFPDD_DEV_INFO);
                }

                unsigned int nNewReaderCnt = nReaderCnt;
                int result2 = dpfpdd_query_devices(&nNewReaderCnt, pReaderInfo);

                // Manejo de errores en la consulta de dispositivos
                if(DPFPDD_SUCCESS != result2 && DPFPDD_E_MORE_DATA != result2) {

                    response = MHD_create_response_from_buffer(strlen("{\"message\": \"Huellero no conectado\", \"type\": \"false\"} "),
                                                            (void *) "{\"message\": \"Huellero no conectado\", \"type\": \"false\"} ",
                                                            MHD_RESPMEM_MUST_COPY);
                                MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
                                MHD_add_response_header(response, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
                                MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");

                                    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
                                    MHD_destroy_response(response);
                                    return ret;
                    printf("Error en dpfpdd_query_devices(): %d", result2);
                    free(pReaderInfo);
                    pReaderInfo = NULL;
                    nReaderCnt = 0;
                    break;
                }

                if(DPFPDD_E_MORE_DATA == result2) {
                    DPFPDD_DEV_INFO* pri = (DPFPDD_DEV_INFO*)realloc(pReaderInfo, sizeof(DPFPDD_DEV_INFO) * nNewReaderCnt);
                    if(NULL == pri) {
                        printf("Error en realloc(): ENOMEM");
                        break;
                        response = MHD_create_response_from_buffer(strlen("{\"message\": \"Huellero no conectado\", \"type\": \"false\"} "),
                                                            (void *) "{\"message\": \"Huellero no conectado\", \"type\": \"false\"} ",
                                                            MHD_RESPMEM_MUST_COPY);
                                    MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
                                    MHD_add_response_header(response, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
                                    MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");

                                    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
                                    MHD_destroy_response(response);
                                    return ret;
                    }
                    pReaderInfo = pri;
                    nReaderCnt = nNewReaderCnt;
                    continue;
                }

                nReaderCnt = nNewReaderCnt;
                break;
            }

            // Selección del lector y obtención de sus capacidades
            int result2 = 0;
            int nChoice = 0;
    

            // Si no se encuentra ningún lector de huellas
            if(0 == nReaderCnt) {
                response = MHD_create_response_from_buffer(strlen("{\"message\": \"Huellero no conectado\", \"type\": \"false\"} "),
                                                            (void *) "{\"message\": \"Huellero no conectado\", \"type\": \"false\"} ",
                                                            MHD_RESPMEM_MUST_COPY);
                                MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
                                MHD_add_response_header(response, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
                                MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");
                
                if(NULL != hReader){
                    result = dpfpdd_close(hReader);
                    hReader = NULL;
                } 
                
                int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
                MHD_destroy_response(response);
                return ret;
                dpfpdd_exit();  

            } else {
                // Selección y apertura del lector de huellas
                hReader = SelectAndOpenReader(szReader, sizeof(szReader),&dpi);

                unsigned char* pFeatures1 = NULL;
                unsigned int nFeatures1Size = 0;
                unsigned char* pFeatures2 = NULL;
                unsigned int nFeatures2Size = 0;

                int bStop = 0;
                char* base64Data = CaptureFinger("any finger", hReader, dpi,DPFJ_FMD_ISO_19794_2_2005, &pFeatures1);
                char* estado = "false";

                char* readerName = (char*)malloc( sizeof(szReader));
                if (readerName != NULL) {
                    strncpy(readerName, szReader,  sizeof(szReader));
                }

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

                int result = 0;
                int nChoice = 0;

                char* divice_name =  pReaderInfo[nChoice].descr.serial_num; 

                if(strlen(base64Data) > 30  ){
                    estado = "true";  

                    const char* input_file_name = "fingerprint.bmp";
                            
                    size_t input_length;
                    unsigned char* input_content = read_file(input_file_name, &input_length);

                    size_t encoded_length;
                    char* encoded_content = base64_encode(input_content, input_length, &encoded_length);

                    size_t legth = strlen(encoded_content);
                    char buffer[legth+50];

                    sprintf(buffer, "{\"message\": \"%s\", \"type\": \"%s\", \"serial_num_divice\": \"%s\"}", encoded_content , estado, divice_name);
                    response = MHD_create_response_from_buffer(strlen(buffer),
                                                    (void *)buffer,
                                                    MHD_RESPMEM_MUST_COPY);
                } else {
                    char buffer[512]; 
                    sprintf(buffer, "{\"message\": \"%s\", \"type\": \"%s\", \"serial_num_divice\": \"%s\"}", base64Data , estado, divice_name);
                    response = MHD_create_response_from_buffer(strlen(buffer),
                                                    (void *)buffer,
                                                    MHD_RESPMEM_MUST_COPY);
                }
                
                if(NULL != hReader){
                    result = dpfpdd_close(hReader);
                    hReader = NULL;
                } 

                dpfpdd_exit();  
                                MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
                                MHD_add_response_header(response, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
                                MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");

                int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
                MHD_destroy_response(response);
                
                return ret;
            } 

            if(NULL != pReaderInfo) free(pReaderInfo);
            pReaderInfo = NULL;
            nReaderCnt = 0;
        }
        
    }

    return MHD_NO;  // Página no encontrada
}

// Función principal
int main() {

    signal(SIGINT, sigint_handler);


   // while (keep_running) {
        struct MHD_Daemon *daemon;
        struct MHD_Daemon *daemon2;

       daemon2 = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG, 5051, NULL, NULL,
                                &request_handler2, NULL,
                                MHD_OPTION_CONNECTION_LIMIT, 10,
                                MHD_OPTION_END);
        // Inicia el demonio de MHD
        daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG, PORT, NULL, NULL,
                                &request_handler, NULL,
                                MHD_OPTION_CONNECTION_LIMIT, 10,
                                MHD_OPTION_END);

 
   // }
    if (daemon == NULL) {
        printf("Error al iniciar el servidor\n");
        return 1;
    }

    printf("Servidor escuchando en http://127.0.0.1:%d/\n", PORT);
    printf("Presiona Enter para detener el servidor...\n");
    getchar();

    // Detiene el demonio
    MHD_stop_daemon(daemon);
    MHD_stop_daemon(daemon2);

    return 0;
   
}

// Función para codificar datos binarios a Base64
char* base64_encode(const unsigned char* input, size_t length, size_t* output_length) {
    const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    *output_length = 4 * ((length + 2) / 3);

    char* encoded_data = (char*)malloc(*output_length + 1);
    if (encoded_data == NULL) {
        fprintf(stderr, "Error de asignación de memoria\n");
        exit(EXIT_FAILURE);
    }

    size_t i, j;
    for (i = 0, j = 0; i < length; i += 3, j += 4) {
        uint32_t octet_a = i < length ? input[i] : 0;
        uint32_t octet_b = i + 1 < length ? input[i + 1] : 0;
        uint32_t octet_c = i + 2 < length ? input[i + 2] : 0;

        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;

        encoded_data[j] = base64_chars[(triple >> 18) & 0x3F];
        encoded_data[j + 1] = base64_chars[(triple >> 12) & 0x3F];
        encoded_data[j + 2] = i + 1 < length ? base64_chars[(triple >> 6) & 0x3F] : '=';
        encoded_data[j + 3] = i + 2 < length ? base64_chars[triple & 0x3F] : '=';
    }

    encoded_data[*output_length] = '\0';
    return encoded_data;
}

// Función para leer el contenido de un archivo en un búfer
unsigned char* read_file(const char* filename, size_t* length) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    *length = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char* content = (unsigned char*)malloc(*length);
    fread(content, 1, *length, file);
    fclose(file);

    return content;
}

