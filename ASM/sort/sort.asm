/ Algoritmo de ordenamiento en MARIE
/ Algoritmo utilizado: INSERCION
/ Ocupación de memoria de programa: 168kB
/ Ocupación de memoria de datos (sin incluir el vector): 160kB
/ Performance
/ Vector ordenado -3,-2,-1,0,1,2,3 : 127 instrucciones
/ Vector invertido 3,2,1,0,-1,-2,-3 : 589 instrucciones
/ Vector desordenado 0,-2,1,3,-1,2,-3 : 375 instrucciones
/****************************************************************************************************/
/********************************************** INICIO **********************************************/
ORG 000
Jump ordenar        / Vector de reset

/****************************************** VECTOR DE DATOS *****************************************/
VECTOR, DEC 7       / Largo del vector
        DEC 3      / Primer elemento del vector
        DEC 2
        DEC 1
        DEC 0
        DEC -1
        DEC -2
        DEC -3

/********************************************* VARIABLES ********************************************/
LENGTH, HEX 001     / Posición de memoria donde se encuentra la longitud del vector
START,  HEX 002     / Posición de memoria donde se encuentra el inicio del vector

P1,     DEC 0       / Puntero      
P2,     DEC 0       / Puntero  
P3,     DEC 0       / Puntero
P4,     DEC 0       / Puntero

ONE,    DEC 1       / Variable utilizada para sumar o restar 1

AUX1,   DEC 0       / Variable auxiliar para realizar el "swap" entre elementos del vector
AUX2,   DEC 0       / Variable auxiliar para realizar el "swap" entre elementos del vector 

CONT,   DEC 0       / Contador    

/****************************************** INICIALIZACIÓN ******************************************/

ordenar,        Load    START   / Almaceno en P1 la dirección de memoria donde comienzan los datos
                Store   P1 
            
                LoadI   LENGTH  / Almaceno en la variable CONT = LENGTH - 1
                Subt    ONE
                Store   CONT

/****************************************** LOOP PRINCIPAL ******************************************/
loop,   Load    P1      / Cargo el puntero P1 y lo almaceno en P4 para utilizar a P1 sin perder la posición original
        Store   P4

        Load    P1      / Cargo P1 en el AC, le sumo 1 y lo almaceno en P2
        Add     ONE
        Store   P2 

        LoadI   P2      / Cargo el contenido del puntero P2 y lo almaceno en AUX2
        Store   AUX2

        LoadI   P1      / Cargo el contenido de lo que apunta P1, le resto el contenido de lo que apunta P2 y me fijo si es mayor a 0
        Subt    AUX2    / Si el AC > 0, salto a swap, sino, salto a nswap. 
        Skipcond 800    
        Jump    nswap
        Jump    swap


/******************************************** SUBRUTINAS ********************************************/
nswap,  Load    CONT    / Cargo el contador y le resto uno
        Subt    ONE
        Store   CONT

        Load    P1      / Cargo el puntero P1 y le sumo 1
        Add     ONE
        Store   P1

        Load CONT       / Chequeo si el contador es 0
        Skipcond 400    / Si el AC==0, salto a halt, sino, salto a loop
        Jump    loop
        Jump    halt

/****************************************************************************************************/
swap,   LoadI   P2      / Cargo el contenido en memoria de lo que apunta P2 y lo almaceno en AUX2 para hacer el swap
        Store   AUX2
        
        LoadI   P1      / Cargo el contenido en memoria de lo que apunta P1 y lo almaceno en AUX1 para hacer el swap
        Store   AUX1

        /***** Ejecuto el swap *****/
        Load    AUX1    / Cargo el valor de AUX1 y lo almaceno en la dirección que apunta P2
        StoreI  P2

        Load    AUX2    / Cargo el valor de AUX1 y lo almaceno en la dirección que apunta P2
        StoreI  P1

        
        /***** Defino un puntero al elemento anterior SI SOLO SI no estoy en el primer elemento *****/
IF,     Load    P1      / Cargo el puntero P1 y le resto la dirección START, de esta forma, sé si estoy en el primer elemento
        Subt    START     

        Skipcond 400    / Si el AC==0, simplemente salto a go, sino es cero, salto a ELSE
        Jump    ELSE
        Jump    go

        /***** Me fijo si es necesario realizar otro swap *****/
ELSE,   Load    P1      / Cargo la dirección P1, le resto un lugar y lo almaceno en P3
        Subt    ONE
        Store   P3

        LoadI   P1      / Cargo el contenido de lo que apunta P1 y lo almaceno en AUX1
        Store   AUX1

        LoadI   P3      / Cargo el contenido de lo que apunta P3 y le resto el contenido de lo que apunta P1
        Subt    AUX1
    
        Skipcond 800    / Si el AC > 0, salto a pila, sino, salto a go. 
        Jump    go
        Jump    pila


/****************************************************************************************************/
pila,  Load    P1      / Cargo el puntero P1 y lo alcameno en P2
        Store   P2

        Load    P3      / Cargo el puntero P3 y lo almaceno en P1
        Store   P1

        Jump    swap    / Salto a la subrutina que hace el swap. Pero esta vez, los punteros redefinidos

/****************************************************************************************************/
go,     Load    CONT    / Cargo el contador y lo decremento
        Subt    ONE
        Store   CONT

        Load    P4      / Cargo P4 (donde originalmente se encontraba P1), y lo almaceno en P1
        Add     ONE
        Store   P1

        Load CONT       / Cargo el contador y corroboro que no sea 0.
        Skipcond 400    / Si el AC==0, simplemente salto a halt, sino es cero, salto a loop
        Jump    loop
        Jump    halt

/****************************************************************************************************/
halt, Halt              / Fin del programa
/****************************************************************************************************/
/****************************************************************************************************/
