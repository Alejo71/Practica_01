# LEEME

## Estructura del Dataset

El archivo CSV utilizado como dataset debe tener la siguiente estructura:

- La **primera línea** corresponde a la cabecera (nombres de los campos).
- Cada línea representa un registro con los campos separados por comas.
- El **primer campo** es la clave o ID única del registro.
- El **segundo campo** es el nombre asociado al registro.
- El **tercer campo** es la descripcion asociada al registro.
- El **cuarto campo** es la instruccion asosciado al registro.
- El **quinto campo** es el link asociado al registro.
- El **sexto campo** es la fuente asociada al registro.
- El **septimo campo** es el NER (Reconocimiento de entidades nombradas) asociado al registro.
- Ejemplo de estructura:

```
id,nombre,otros_campos...
1, Jewell Ball'S Chicken, "Descripcion","Instruccion", www.link.com, Fuente, ["NER"]
```

## Criterios de Búsqueda Implementados

El sistema permite buscar registros utilizando dos criterios:

1. **Por clave (ID):**
   - Busca el registro cuyo primer campo coincide exactamente con la clave ingresada.
   - Utiliza una tabla hash para acelerar la búsqueda.


## Adaptaciones Realizadas

- Se implementó una tabla hash en memoria para indexar los registros por clave y permitir búsquedas eficientes.
- Se utiliza memoria compartida (`shm_open`, `mmap`) para la comunicación entre la interfaz de usuario y el proceso de búsqueda.
- Se emplean semáforos POSIX para la sincronización entre procesos, asegurando la correcta coordinación de las consultas y respuestas.
- La interfaz de usuario permite ingresar la clave a buscar y muestra el resultado obtenido desde el proceso de búsqueda.
- El código está modularizado en archivos separados para la interfaz, la lógica de búsqueda y la gestión de la tabla hash.
- Se documenta el uso de recursos del sistema operativo (memoria compartida y semáforos) y la estructura de datos utilizada para el índice.