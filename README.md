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

El sistema permite buscar registros utilizando un criterio:

1. **Por nombre:**
   - Busca el registro cuyo segundo campo coincide exactamente con la clave ingresada.
   - Utiliza una tabla hash para acelerar la búsqueda.


## Funcionamiento del código

- search.c
- Abre el CSV y construye un índice en memoria (tabla hash) que mapea claves a offsets en el archivo.
- Se queda en ejecución esperando consultas desde la interfaz mediante memoria compartida.
- Al recibir una consulta, busca en la tabla hash, verifica leyendo el registro en el archivo y devuelve el resultado a la interfaz.

- hash.c / hash.h
- Implementan la tabla hash y las operaciones de inserción y búsqueda.
- El índice almacena información necesaria para localizar el registro en disco (por ejemplo, hash y offset) y resuelve colisiones mediante listas enlazadas.
- Al buscar, tras localizar candidatos por hash se lee la línea en el archivo y se compara la clave real para asegurar coincidencia.

- ui.c
- Interfaz de usuario que envía consultas al proceso de búsqueda mediante memoria compartida y semáforos POSIX.
- Muestra los resultados recibidos desde el proceso de búsqueda.

- Comunicación entre procesos
- Memoria compartida: `shm_open` + `mmap` para intercambio de consultas y respuestas.
- Semáforos POSIX para sincronizar envío y recepción de consultas.

- Hashing
- Se utiliza la librería xxHash incluida en `lib/xxhash` para calcular los hashes de las claves.

## Compilación

Desde el directorio del proyecto:
```sh
./search /ruta/al/dataset.csv
"En otra terminal" ./ui