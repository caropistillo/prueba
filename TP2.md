TP2: Procesos de usuario
========================

env_alloc
---------------
**1. ¿Qué identificadores se asignan a los primeros 5 procesos creados? (Usar base hexadecimal.)**

De los valores impresos en Qemu, puede observarse que el valor del id del environment es el hexadecimal 0x1000. El valor 0x1000 surge de las cuentas realizadas en la siguiente porcion de código: 

    generation = (e->env_id + (1 << ENVGENSHIFT)) & ~(NENV - 1);

    if (generation <= 0)  // Don't create a negative env_id. 
          generation = 1 << ENVGENSHIFT; 

Entonces, como los primeros 5 procesos creados comienzan con sus id en 0, el "generation" de los mismos resultara en 0x1000.
Ademas se realiza un OR con *(e-envs)* correspondiente al indice de *e* en el arreglo envs:

    e->env_id = generation | (e - envs);

Considerando que los indices van de 0 a 4, se tendran entonces los siguientes valores de id para los primeros 5 procesos:

1. 0x1000 | 0x0 = 0x1000

2. 0x1000 | 0x1 = 0x1001

3. 0x1000 | 0x2 = 0x1002

4. 0x1000 | 0x3 = 0x1003

5. 0x1000 | 0x4 = 0x1004


**2. Supongamos que al arrancar el kernel se lanzan NENV procesos a ejecución. A continuación se destruye el proceso asociado a envs[630] y se lanza un proceso que cada segundo muere y se vuelve a lanzar. ¿Qué identificadores tendrá este proceso en sus sus primeras cinco ejecuciones?**

Como el proceso correspodiente a env[630] es siempre el que se "lanza y muere", el término *(e-envs)* se mantendrá constante en este caso e igual a 630 (0x276 en hexadecimal). Sin embargo la variable *generation* ya no es constante ya que cada vez que se lanza un nuevo proceso el id anterior implica un cambio en el valor de *generation* en cada paso. Siguiendo la lógica del inciso anterior, los id de los primeros 5 procesos seran ahora:

1. 0x1000 | 0x276 = 0x1276

2. 0x2264 | 0x276 = 0x2276

3. 0x3264 | 0x276 = 0x3276

4. 0x4264 | 0x276 = 0x4276

5. 0x5264 | 0x276 = 0x5276
 

env_init_percpu
---------------
**¿Cuántos bytes escribe la función lgdt, y dónde?**

La función LGDT carga 6 bytes en el registro GDTR o en el registro de la tabla del descriptor de interrupciones(IDTR).

**¿Qué representan esos bytes?**

Esos 6 bytes representan una ubicación de memoria que contiene la dirección base y el límite de memoria de la tabla de descriptores globales (GDT) o la tabla de descriptores de interrupciones (IDT). En esta implementación puede observarse que se asignan 16 bits al limite de memoria (los 2 bytes inferiores) y 32 bits de dirección base (los 4 bytes superiores). Esto puede verse en la estructura "Pseudodesc" definida en "mmu.h". Este es el tipo de dato que recibe la función "lgdt" cuando se la llama en "env_init_percpu".

**env_pop_tf**
----------
**1. Dada la secuencia de instrucciones assembly en la función, describir qué contiene durante su ejecución:**

* el tope de la pila justo antes popal

    Justo antes del *popal* la unica instruccion ejecutada es *movl %0,%%esp*. Tras esta directiva, %esp contendra la direccion del Trapframe, es decir, *reg_edi* (primer campo del struct PushRegs).

* el tope de la pila justo antes iret

    En esta situación, %esp contendrá tf_eip

* el tercer elemento de la pila justo antes de iret

    El tercer elemento de la pila, 8(%eip), contiene a tf_eflags

**2. En la documentación de iret en [IA32-2A] se dice:**

*If the return is to another privilege level, the IRET instruction also pops the stack pointer and SS from the stack, before resuming program execution.*

**¿Cómo determina la CPU (en x86) si hay un cambio de ring (nivel de privilegio)? Ayuda: Responder antes en qué lugar exacto guarda x86 el nivel de privilegio actual. 
¿Cuántos bits almacenan ese privilegio?**

Identifica si hay un cambio de ring a partir de los últimos dos bits del registro %cs. Así, puede saber si se esta en user mode (bits: 11) o en kernel mode (bits: 00).


gdb_hello
---------
**1. Poner un breakpoint en env_pop_tf() y continuar la ejecución hasta allí.**

![Breakpoint en env_pop_tf](https://raw.githubusercontent.com/caropistillo/prueba/master/gdbHello01.png)

**2. En QEMU, entrar en modo monitor (Ctrl-a c), y mostrar las cinco primeras líneas del comando info registers.**

![Primeras cinco lineas del comando "info registers" en QEMU](images_TP2/infoRegisters1.png)

**3. De vuelta a GDB, imprimir el valor del argumento tf.**

![Valor del argumento tf en gdb](images_TP2/gdbHello02.png)

**4. Imprimir, con x/Nx tf tantos enteros como haya en el struct Trapframe donde N = sizeof(Trapframe) / sizeof(int).**

![Argumentos tf del struct Trapframe](images_TP2/gdbHello03.png)

**5. Avanzar hasta justo después del movl ...,%esp, usando si M para ejecutar tantas instrucciones como sea necesario en un solo paso.**

![Avance en assembly](images_TP2/gdbHello04.png)

**6. Comprobar, con x/Nx $sp que los contenidos son los mismos que tf (donde N es el tamaño de tf)**

![Valores de los registros sp](images_TP2/gdbHello05.png)

**7. Describir cada uno de los valores. Para los valores no nulos, se debe indicar dónde se configuró inicialmente el valor, y qué representa.**

Los 8 valores nulos del comienzo se corresponden a los 8 registros que guarda *tf_regs* (de tipo struct PushRegs).

Los 2 siguientes valores son 0x00000023. Estos se corresponden a los campos *tf_es* y *tf_ds* y son configurados en la función *env_alloc*, donde se setean en GD_UD | 3. Si se realiza esta cuenta puede comprobarse que el resultado coincide con lo observado en gdb.

Los 2 valores siguientes figuran nulos y se corresponden a *tf_trapno* y *tf_err* del struct Trapframe. 

El siguiente valor, 0x00800020, se corresponde al campo *tf_eip*. Este es el valor que necesita %eip para correr el proceso. Es interesante observar que si se ejecuta el siguiente comando:

![tf_eip](images_TP2/gdbHello08.png)

la salida nos indica que este campo contiene efectivamente la dirección *_start* de comienzo del proceso.

Luego le sigue el valor 0x0000001b, correspondiente a *tf_cs*. Este campo se configura en env_alloc al igual que *tf_es* y *tf_es* pero esta vez al valor al valor GD_UT | 3. El registro %cs corresponde al *Code Segment (CS)*, el puntero al código.

El próximo valor nulo se corresponde a *tf_eflags*. 

Los dos últimos valores se corresponden a *tf_esp* y *tf_ss* y también son cinfigurados en env_alloc. El valor asignado a *tf_esp* es el USTACKTOP y el asignado a *tf_ss* es GD_UD | 3 al igual que en los campos *tf_es* y *tf_ds* mencionados antes.

**8.** Continuar hasta la instrucción iret, sin llegar a ejecutarla. Mostrar en este punto, de nuevo, las cinco primeras líneas de info registers en el monitor de QEMU. Explicar los cambios producidos.

![Gdb hasta la instrucción iret exclusive](images_TP2/gdbHello06.png)

![Primeras cinco lineas del comando "info registers" en QEMU](images_TP2/infoRegisters2.png)

A partir de lo obtenido en *info_registers* puede verse que los valores de los registros GPR fueron seteados en 0, y esto coincide con los valores del *tf_regs* obtenidos en el inciso anterior. De la segunda linea puede notarse que el registro %esp contiene la dirección de *tf_esp*, que coincide con la dirección de la tercer linea obtenida en el inciso anterior.También se observa que el registro %eip tiene el valor de la dirección del IP (instruction pointer) actual, es decir el de la dirección de la instrucción *iret*. Por otra parte el registro %es contiene el valor 0023, que coincide con lo obtenido para el Trapframe en el inciso previo. El valor de CS se mantuvo constante al compararlo con lo obtenido previamente bajo el comando *info_registers*.


**9. Ejecutar la instrucción iret. En ese momento se ha realizado el cambio de contexto y los símbolos del kernel ya no son válidos.**

* imprimir el valor del contador de programa con p $pc o p $eip

* cargar los símbolos de hello con symbol-file obj/user/hello

* volver a imprimir el valor del contador de programa

![Valor del contador del programa antes y después de la carga de los simbolos de hello](images_TP2/gdbHello07.png)

Mostrar una última vez la salida de info registers en QEMU, y explicar los cambios producidos.

![Primeras cinco lineas del comando "info registers" en QEMU](images_TP2/infoRegisters3.png)

En esta instancia, los registros %eip, %esp y %cs ya tienen los valores que se indican en el struct Trapframe. Como se comprobó antes %eip apunta a la dirección del comienzo del proceso: _start. En particular es interesante observar que ahora el valor del registro %cs termina en 11 y no en 00, es decir, se paso de modo kernel a modo usuario.

**10. Poner un breakpoint temporal (tbreak, se aplica una sola vez) en la función syscall() y explicar qué ocurre justo tras ejecutar la instrucción int $0x30. Usar, de ser necesario, el monitor de QEMU.**

![Breakpoint temporal en syscall](images_TP2/gdbHello09.png)

![Ejecucion de la instruccion int $0x30](images_TP2/gdbHello10.png)

Luego de ejecutar la instruccion se volvio a utilizar el comando *info_registers* en el monitor del QEMU para ver el contexto actual. Puede observarse que el valor del registro %cs paso nuevamente a terminar en 00 (se cambio a modo kernel).

![Primeras cinco lineas del comando "info registers" en QEMU luego de la instruccion int $0x30](images_TP2/infoRegisters4.png)


kern_idt
----------

**¿Cómo decidir si usar TRAPHANDLER o TRAPHANDLER_NOEC? ¿Qué pasaría si se usara solamente la primera?**

En la tabla de la sección 6.3.1 del manual[IA32-3A], se especifica qué interrupciones poseen un "Error code". A partir de este dato se decide si usar TRAPHANDLER o TRAPHANDLER_NOEC.
Si la interrupción tiene "Error code" se usará TRAPHANDLER y si no lo tiene se usará TRAPHANDLER_NOEC. Si se usara solamente la primera, las interrupciones que no necesiten un "Error Code" estarán agregando contenido innecesario al stack alterandolo y haciendo que no coincida la cantidad de parametros necesarios para ese trap.


**¿Qué cambia, en la invocación de handlers, el segundo parámetro (istrap) de la macro SETGATE? ¿Por qué se elegiría un comportamiento u otro durante un syscall?**
En el archivo "mmh.h" se menciona que: 
*The difference between an interrupt gate and a trap gate is in the effect on IF (the interrupt-enable flag). An interrupt that vectors through an interrupt gate resets IF, thereby preventing other interrupts from interfering with the current interrupt handler*

Esto significa que cuando se invoque el handler de una interrupción, el parámetro isTrap va a dar o denegar permiso para que otras interupciones interfieran en el proceso de este handler.


**Leer user/softint.c y ejecutarlo con make run-softint-nox. ¿Qué excepción se genera? Si es diferente a la que invoca el programa… ¿cuál es el mecanismo por el que ocurrió esto, y por qué motivos?**
Se genera la interrupción 14, correspondiente a un "Page Fault". El mecanismo que se usa es la llamada mediante la instrucción int $14.


user_evilhello
----------------

**¿En qué se diferencia el código de la versión en evilhello.c mostrada arriba? ¿En qué cambia el comportamiento durante la ejecución? ¿Por qué? ¿Cuál es el mecanismo?**
El codigo de la version original de evilhello.c "despierta" al kernel a traves de la invocación de la syscall SYS_cputs con una dirección perteneciente el kernel como parámetro. El kernel manejará dicha invocación con el handler que corresponda, sin validar la dirección y dejando que el usuario acceda a la misma.

Por otra parte, la version modificada de evilhello.c que se muestra posee una variable *entry* que apunta a una dirección inválida. A diferencia del codigo original, se intenta desreferenciar a *entry* al intentar asignarle a first el primer byte de la dirección invalida. En consecuencia, ahora si se detectará que el usuario no tiene permiso a esa dirección cuando se intente acceder y se lanzará un "Page Fault".































