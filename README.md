# Emulador MASIC, hecho en Qt6
Este potente programa (compatible con Linux y Windows) nos permite emular el sistema MASIC completo cargando un archivo binario ya ensamblado.
Muestra la salida de vídeo y nos permite ver el contenido de los registros y memoria en tiempo real, además
de poder avanzar paso a paso en saltos de ciclo de reloj o de instrucción. Se trata de un emulador de microarquitectura
bastante modular

## Cómo compilarlo
### Dependencias
Antes de compilar, necesitamos tener instalado como mínimo:
- Qt6.10.1 con MinGW 13.1.0
- Qt Creator 18.0.0

### Primer paso
Para empezar, abrimos el proyecto con Qt Creator (se selecciona el archivo CMakeLists.txt) y si tenemos configurado correctamente todo,
nos detectará Qt6.x o la versión que tengas instalada. Posiblemente nos pedirá configurar el proyecto. Le damos a configure, y para
compilarlo y ejecutar pulsamos Ctrl+R.

### En el caso de Windows
Una vez comprobamos que todo funciona correctamente, cerramos el programa, y veremos que se habrá generado una carpeta build en el proyecto.
De ahí extramos el .exe que se ha generado, pero no funciona por sí solo, ya que faltan las librerías. Lo movemos a alguna carpeta vacía y
abrimos Qt 6.x for MinGW (64-bit) desde el menú inicio, que nos prepara un CMD con todas las variables de entorno necesarias. Desde aquí
navegamos hasta el directorio donde esté la carpeta vacía que hemos creado y ejecutamos:
`windeployqt qtmasic.exe`
¡Listo! Ya tenemos el programa preparado para ser ejecutado.

### En el caso de Linux
El binario que se genera en la carpeta build por sí solo funciona perfectamente. Aquí es más sencillo :)

## Mejoras proyectadas a futuro
- Carga de código fuente en ensamblador, con salida de errores
- Posibilidad de provocar interrupciones durante la ejecución del programa
- Añadir posibilidad de cargar el firmware de la CPU
- Agregar una vista gráfica del diagrama de la arquitectura para ver las señales de control y el camino que siguen los datos en tiempo real
