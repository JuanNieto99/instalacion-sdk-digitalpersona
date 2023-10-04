# instalacion-sdk-digitalpersona
USANDO UBUNTU 18.4
 
entrar a la carpeta sdk/sdk 
abrir la consola e instalar con "sudo ./install" el sdk de DIGITALPERSONA

volver a la carpeta "instalacion-sdk" copiar la carpeta  "Crossmatch" en "usr/local/lib"
	nota!
	si no te deja pegar debes darles permiso a la carpeta lib, vamos dentro de la carpeta "local" y abrimos una consola y ejecutamos "sudo chmod -R 777 lib"

luego vamos a "/opt/Crossmatch/urusdk-linux/redist" copiar los archivos de texto 99-*cc
	nota!
	si no  nos deja copiar los archivos abrimos la consola en la carpeta "urusdk-linux" y abrimos una consola y ejecutamos "sudo chmod -R 777 redist"
	
y pegamos los archivos copiados en "etc/udev/rules.d"  y remplazamos

	nota!
	si no te deja pegar debes darles permiso a la carpeta rules.d, vamos dentro de la carpeta "udev" y abrimos una consola y ejecutamos "sudo chmod -R 777 rules.d"

para las pruebas de java debe instalar java e ir a si desea usarlo con ubuntu 22 usar la version 17 de java

ir a los ejemplos en "opt/crossmatch/urusdk-linux/linux/samples/bin/java" y ejecutar "sudo ./run_x64.sh"
